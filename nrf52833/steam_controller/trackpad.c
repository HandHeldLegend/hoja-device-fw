#include "trackpad.h"

#include <string.h>

#include "main.h"

#include "hal/gpio_hal.h"
#include "hal/i2c_hal.h"
#include "hal/sys_hal.h"
#include "nrf52_hal.h"

// The controller is spoken to as the stock firmware speaks to it, by the community's reading of that
// firmware: an HID-over-I2C device whose settings sit behind a register map of its own, read and
// written by command. The set-up below is the stock one, value for value; several fields in it have
// no known meaning.

#define TRACKPAD_VENDOR             0x0488u // Cirque
#define TRACKPAD_PRODUCT_A          0xD0C1u
#define TRACKPAD_PRODUCT_B          0x1015u
#define TRACKPAD_DESCRIPTOR_LEN     30u
#define TRACKPAD_AWAKE_MS           12u // From reset let go to the first command
// The controller wants this long before each register command
#define TRACKPAD_COMMAND_GAP_US     2000u
#define TRACKPAD_I2C_TIMEOUT_US     20000
// A controller that did not take its set-up is reset and tried again after this, this many times
#define TRACKPAD_RETRY_US           1000000u
#define TRACKPAD_TRIES              3u

#define TRACKPAD_ELECTRODES         8
#define TRACKPAD_EDGE               32767
// An electrode counts towards a finger's position above this: at rest it is within 20 of zero,
// and the ones beside a finger read a few hundred
#define TRACKPAD_FLOOR              100
// A pad is touched when its strongest electrode reads this (a finger on it reads 2000 to 3000,
// one leaving by an edge a few hundred), and stays touched down to the lower one
#define TRACKPAD_TOUCH_START        800
#define TRACKPAD_TOUCH_STAY         500
// A click, set by feel: at 1500 the press it took was far too hard, at 500 and 350 still too hard,
// at 240 right. A press meant as a click peaked at 290 to 470 (24 of them, both pads); a finger
// sliding read 30 to 100, and a hard press 2500 to 4000. Not yet each pad's own range, which the
// stock firmware keeps from the factory.
#define TRACKPAD_PRESS_START        240
#define TRACKPAD_PRESS_STAY         170
#define TRACKPAD_PRESSURE_SCALE     8
// A finger slid from one edge to the other took the centre from about 0.35 of an electrode to 6.5
// (in 256ths): the last electrode at each end has no neighbour to pull the centre past it
#define TRACKPAD_CENTRE_LOW         90
#define TRACKPAD_CENTRE_HIGH        1664
// With no frame for this long, nothing is touched
#define TRACKPAD_STALE_US           100000u

// A register command: its opcode pair, the address, the length, a zero, then (writing) the data and
// the sum of every byte before it. A read answers with its own length, the data and their sum.
#define TRACKPAD_COMMAND_LEN        8u
#define TRACKPAD_DATA_MAX_LEN       200u
#define TRACKPAD_OP_WRITE           0x00u
#define TRACKPAD_OP_READ            0x01u
#define TRACKPAD_OP_CLASS           0x09u
#define TRACKPAD_READ_OVERHEAD      3u

// Each block of settings is a run of records, record n at the block's address + n * this
#define TRACKPAD_RECORD_STEP        0x1000u
#define TRACKPAD_REG_GLOBAL         0x51000000u
#define TRACKPAD_REG_MEASUREMENT    0x51100000u
#define TRACKPAD_REG_GROUP          0x51200000u
#define TRACKPAD_REG_NOISE          0x51500000u
#define TRACKPAD_REG_BLOCK_6        0x51600000u // Meaning not known
#define TRACKPAD_REG_RESULT_MAP     0x51700000u
#define TRACKPAD_REG_COMPENSATION   0x51800000u
#define TRACKPAD_REG_BLOCK_9        0x51900000u // Meaning not known
#define TRACKPAD_REG_ROUTE          0x51A00000u
#define TRACKPAD_REG_ROUTE_LIMIT    0x51B00000u

#define TRACKPAD_GLOBAL_LEN         7u
#define TRACKPAD_GROUP_LEN          26u
#define TRACKPAD_GROUPS             5u
#define TRACKPAD_GROUP_CALIBRATE    0x40u // In a group's second byte: measure its resting levels again
#define TRACKPAD_MEASUREMENT_LEN    41u
#define TRACKPAD_MEASUREMENTS       9u
#define TRACKPAD_NOISE_LEN          9u
#define TRACKPAD_BLOCK_6_LEN        7u
#define TRACKPAD_RESULT_MAP_LEN     136u
#define TRACKPAD_RESULT_MAPS        2u
#define TRACKPAD_COMPENSATION_LEN   6u
#define TRACKPAD_COMPENSATIONS      16u
#define TRACKPAD_ROUTE_LEN          6u
#define TRACKPAD_ROUTES             20u

// A frame: a 5-byte head (the report's ID in its third byte, the payload's length after it), then
// the payload
#define TRACKPAD_FRAME_HEAD_LEN     5u
#define TRACKPAD_FRAME_REPORT_ID    0x0Eu
#define TRACKPAD_FRAME_PAYLOAD_LEN  76u

#define LE16(v) (uint8_t)((v) & 0xFF), (uint8_t)(((v) >> 8) & 0xFF)

// Frames on, one every 4 ms, nothing kept in the controller's own memory
static const uint8_t _trackpad_global[TRACKPAD_GLOBAL_LEN] = {1, LE16(4), 0, 0, 1, 1};

// Mode, calibration flags, frames between compensations, negative threshold, speed threshold,
// activity threshold, activity timeout, then 14 bytes of no known meaning
#define TRACKPAD_GROUP_TAIL_PAD     1, 0, 8, 0, 0x80, 0, 0x20, 0, 0, 2, 0, 0, 0, 0
static const uint8_t _trackpad_groups[TRACKPAD_GROUPS][TRACKPAD_GROUP_LEN] = {
    {0, 0x81, LE16(500), LE16(-75), LE16(300), LE16(500), LE16(150), 1, 0, 8, 0, 0x80, 0, 0x20, 0, 0, 8, 0, 0, 0, 0},
    {0, 0x81, LE16(500), LE16(-75), LE16(300), LE16(600), LE16(150), TRACKPAD_GROUP_TAIL_PAD},
    {1, 0x81, LE16(48), LE16(100), LE16(200), LE16(300), LE16(0), 4, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0xE8, 0x03, 0x0A, 0},
    {1, 0x81, LE16(500), LE16(-75), LE16(300), LE16(600), LE16(150), TRACKPAD_GROUP_TAIL_PAD},
    {0, 0x81, LE16(750), LE16(-32767), LE16(300), LE16(600), LE16(150), TRACKPAD_GROUP_TAIL_PAD},
};

// A measurement: which electrodes it drives and senses, and how. In the record the states are
// followed by the gain, the offset, a multiplier of 1, eight channel offsets of 0, the toggle
// frequency, the aperture, a waveform of 0 and the channel mask.
typedef struct
{
    uint8_t control;
    uint8_t electrodes[24];
    uint8_t gain;
    uint8_t offset;
    uint8_t toggle;
    uint8_t aperture;
    uint8_t mask[2];
} trackpad_measurement_s;

static const trackpad_measurement_s _trackpad_measurements[TRACKPAD_MEASUREMENTS] = {
    // The pads' electrodes, a half of one pad's at a time
    {0x80, {[0 ... 3] = 0x33, [4 ... 7] = 0x77}, 0x09, 0x2D, 0x12, 0x04, {0x00, 0xFF}},
    {0x80, {[0 ... 3] = 0x77, [4 ... 7] = 0x33}, 0x09, 0x2D, 0x12, 0x04, {0xFF, 0x00}},
    {0x80, {[8 ... 11] = 0x33, [12 ... 15] = 0x77}, 0x09, 0x2D, 0x12, 0x04, {0x00, 0xFF}},
    {0x80, {[8 ... 11] = 0x77, [12 ... 15] = 0x33}, 0x09, 0x2D, 0x12, 0x04, {0xFF, 0x00}},
    // The single sensors: pressure, grips, stick caps
    {0x81, {[16] = 0x34, [17] = 0x49, [20] = 0x34, [21] = 0x49}, 0x0A, 0xA3, 0x12, 0x09, {0x04, 0x04}},
    {0x82, {[18] = 0x74, [19] = 0x47}, 0x08, 0x00, 0x11, 0x0A, {0x60, 0x00}},
    {0x83, {[22] = 0x74, [23] = 0x47}, 0x04, 0x00, 0x11, 0x27, {0x00, 0x60}},
    {0x84, {[0 ... 15] = 0x77, [18] = 0x70, [19] = 0x07, [22] = 0x70, [23] = 0x07}, 0x04, 0x00, 0x12, 0x27, {0x00, 0x00}},
    {0x84, {[0 ... 15] = 0x77, [18] = 0x70, [19] = 0x07, [22] = 0x70, [23] = 0x07}, 0x04, 0x00, 0x11, 0x27, {0x00, 0x00}},
};

// Alternate frequency, noise change threshold, most adjustment a frame, filter weight, clean count,
// then two bytes of no known meaning
static const uint8_t _trackpad_noise[TRACKPAD_MEASUREMENTS][TRACKPAD_NOISE_LEN] = {
    {0x11, LE16(500), LE16(20), 0x19, 0x0A, 0x00, 0x00},
    {0x11, LE16(500), LE16(20), 0x19, 0x0A, 0x00, 0x00},
    {0x11, LE16(500), LE16(20), 0x19, 0x0A, 0x00, 0x00},
    {0x11, LE16(500), LE16(20), 0x19, 0x0A, 0x00, 0x00},
    {0x11, LE16(500), LE16(20), 0x19, 0x0A, 0x00, 0x00},
    {0x12, LE16(500), LE16(20), 0x19, 0x0A, 0xEE, 0x16},
    {0x12, LE16(500), LE16(200), 0x02, 0x0A, 0x99, 0x19},
    {0xFF, LE16(500), LE16(20), 0x19, 0x0A, 0x00, 0x00},
    {0xFF, LE16(500), LE16(20), 0x19, 0x0A, 0x00, 0x00},
};

static const uint8_t _trackpad_block_6[TRACKPAD_BLOCK_6_LEN] = {0xFF, 0x0A, 0x32, 0x00, 0x2C, 0x01, 0x1F};
static const uint8_t _trackpad_block_9[] = {LE16(500), LE16(20)};
static const uint8_t _trackpad_route_limit[] = {LE16(20)};

// The first pad's electrodes are scaled one by one (8.8 fixed point); the second's are left at 1
static const uint16_t _trackpad_map_0_scales[16] = {317, 296, 298, 297, 295, 305, 321, 338,
                                                    322, 314, 297, 295, 297, 303, 305, 305};

// Type, measurement, first and last result, threshold. The rest of the 20 are empty.
static const uint8_t _trackpad_routes[][TRACKPAD_ROUTE_LEN] = {
    {3, 0, 8, 15, LE16(200)}, {3, 1, 0, 7, LE16(200)}, {3, 2, 8, 15, LE16(300)}, {3, 3, 0, 7, LE16(300)},
    {1, 4, 2, 2, LE16(300)},  {1, 4, 10, 10, LE16(300)}, {1, 5, 5, 6, LE16(300)}, {1, 6, 13, 14, LE16(300)},
    {5, 0, 8, 15, LE16(200)}, {5, 1, 0, 7, LE16(200)},
};

static uint8_t _trackpad_sum(const uint8_t *data, uint16_t len)
{
    uint8_t sum = 0;
    for (uint16_t i = 0; i < len; i++)
        sum = (uint8_t)(sum + data[i]);
    return sum;
}

static void _trackpad_command(uint8_t *out, uint8_t op, uint32_t reg, uint8_t len)
{
    out[0] = op;
    out[1] = TRACKPAD_OP_CLASS;
    out[2] = (uint8_t)reg;
    out[3] = (uint8_t)(reg >> 8);
    out[4] = (uint8_t)(reg >> 16);
    out[5] = (uint8_t)(reg >> 24);
    out[6] = len;
    out[7] = 0;
}

static bool _trackpad_write(uint32_t reg, const uint8_t *data, uint8_t len)
{
    // In RAM: the bus sends a long write from where it is given
    static uint8_t message[TRACKPAD_COMMAND_LEN + TRACKPAD_DATA_MAX_LEN + 1u];

    if (len > TRACKPAD_DATA_MAX_LEN)
        return false;

    _trackpad_command(message, TRACKPAD_OP_WRITE, reg, len);
    memcpy(&message[TRACKPAD_COMMAND_LEN], data, len);
    const uint16_t total = (uint16_t)(TRACKPAD_COMMAND_LEN + len);
    message[total] = _trackpad_sum(message, total);

    return i2c_hal_write_timeout_us(TRACKPAD_I2C_INSTANCE, TRACKPAD_I2C_ADDRESS, message, total + 1u, false,
                                    TRACKPAD_I2C_TIMEOUT_US) == (int)(total + 1u);
}

static bool _trackpad_read_reg(uint32_t reg, uint8_t *out, uint8_t len)
{
    uint8_t command[TRACKPAD_COMMAND_LEN];
    uint8_t answer[TRACKPAD_DATA_MAX_LEN + TRACKPAD_READ_OVERHEAD];

    if (len > TRACKPAD_DATA_MAX_LEN)
        return false;

    _trackpad_command(command, TRACKPAD_OP_READ, reg, len);
    const uint16_t total = (uint16_t)(len + TRACKPAD_READ_OVERHEAD);

    if (i2c_hal_write_read_timeout_us(TRACKPAD_I2C_INSTANCE, TRACKPAD_I2C_ADDRESS, command, sizeof(command), answer,
                                      total, TRACKPAD_I2C_TIMEOUT_US) != (int)total)
        return false;

    // Its own length first, the sum of everything before it last
    if (((uint16_t)(answer[0] | (answer[1] << 8)) != total) || (answer[total - 1u] != _trackpad_sum(answer, total - 1u)))
        return false;

    memcpy(out, &answer[2], len);
    return true;
}

static bool _trackpad_write_measurement(uint8_t i)
{
    const trackpad_measurement_s *m = &_trackpad_measurements[i];
    uint8_t record[TRACKPAD_MEASUREMENT_LEN] = {0};

    record[0] = m->control;
    memcpy(&record[1], m->electrodes, sizeof(m->electrodes));
    record[25] = m->gain;
    record[26] = m->offset;
    record[27] = 1; // Channel offset multiplier; the eight channel offsets after it stay 0
    record[36] = m->toggle;
    record[37] = m->aperture;
    record[39] = m->mask[0];
    record[40] = m->mask[1];

    return _trackpad_write(TRACKPAD_REG_MEASUREMENT + (i * TRACKPAD_RECORD_STEP), record, sizeof(record));
}

// Which of the measurements' readings make up a pad's sixteen electrodes, and their scales
static bool _trackpad_write_result_map(uint8_t map)
{
    uint8_t record[TRACKPAD_RESULT_MAP_LEN] = {0};

    record[1] = TRACKPAD_ELECTRODES;
    record[2] = TRACKPAD_ELECTRODES;
    record[4] = (uint8_t)(1500 & 0xFF);
    record[5] = (uint8_t)(1500 >> 8);
    record[6] = (uint8_t)(1500 & 0xFF);
    record[7] = (uint8_t)(1500 >> 8);

    for (uint8_t i = 0; i < 16u; i++)
    {
        // A measurement and one of its readings. The first pad's go up in order; the second's
        // start half way and finish coming back down.
        uint8_t measurement = (i < 8u) ? 0u : 1u;
        uint8_t reading = i;
        if (map == 1u)
            reading = (i < 8u) ? (uint8_t)(8u + i) : (uint8_t)(15u - i);
        record[8u + (2u * i)] = measurement;
        record[9u + (2u * i)] = reading;

        // 32 bytes of no known meaning (0) sit between the pairs and the scales
        const uint16_t scale = (map == 0u) ? _trackpad_map_0_scales[i] : 256u;
        record[72u + (2u * i)] = (uint8_t)scale;
        record[73u + (2u * i)] = (uint8_t)(scale >> 8);
        record[104u + (2u * i)] = (uint8_t)(256u & 0xFFu);
        record[105u + (2u * i)] = (uint8_t)(256u >> 8);
    }

    return _trackpad_write(TRACKPAD_REG_RESULT_MAP + (map * TRACKPAD_RECORD_STEP), record, sizeof(record));
}

// The controller answers as the part this set-up is for
static bool _trackpad_present(void)
{
    static const uint8_t ask[2] = {0x20, 0x00};
    uint8_t descriptor[TRACKPAD_DESCRIPTOR_LEN];

    if (i2c_hal_write_read_timeout_us(TRACKPAD_I2C_INSTANCE, TRACKPAD_I2C_ADDRESS, ask, sizeof(ask), descriptor,
                                      sizeof(descriptor), TRACKPAD_I2C_TIMEOUT_US) != (int)sizeof(descriptor))
        return false;

    const uint16_t length = (uint16_t)(descriptor[0] | (descriptor[1] << 8));
    const uint16_t vendor = (uint16_t)(descriptor[20] | (descriptor[21] << 8));
    const uint16_t product = (uint16_t)(descriptor[22] | (descriptor[23] << 8));
    return (length == TRACKPAD_DESCRIPTOR_LEN) && (vendor == TRACKPAD_VENDOR) &&
           ((product == TRACKPAD_PRODUCT_A) || (product == TRACKPAD_PRODUCT_B));
}

// ---- Set-up, a command at a time ----

// The commands in the stock firmware's order: frames off, each block of settings, each group told
// to measure its resting levels afresh (its record read, then written back), frames on.
#define TRACKPAD_STEP_PRESENT       0u
#define TRACKPAD_STEP_GLOBAL_READ   1u
#define TRACKPAD_STEP_GLOBAL_OFF    2u
#define TRACKPAD_STEP_GROUPS        3u
#define TRACKPAD_STEP_MEASUREMENTS  (TRACKPAD_STEP_GROUPS + TRACKPAD_GROUPS)
#define TRACKPAD_STEP_NOISE         (TRACKPAD_STEP_MEASUREMENTS + TRACKPAD_MEASUREMENTS)
#define TRACKPAD_STEP_BLOCK_6       (TRACKPAD_STEP_NOISE + TRACKPAD_MEASUREMENTS)
#define TRACKPAD_STEP_RESULT_MAPS   (TRACKPAD_STEP_BLOCK_6 + TRACKPAD_MEASUREMENTS)
#define TRACKPAD_STEP_COMPENSATION  (TRACKPAD_STEP_RESULT_MAPS + TRACKPAD_RESULT_MAPS)
#define TRACKPAD_STEP_BLOCK_9       (TRACKPAD_STEP_COMPENSATION + TRACKPAD_COMPENSATIONS)
#define TRACKPAD_STEP_ROUTES        (TRACKPAD_STEP_BLOCK_9 + 1u)
#define TRACKPAD_STEP_ROUTE_LIMIT   (TRACKPAD_STEP_ROUTES + TRACKPAD_ROUTES)
#define TRACKPAD_STEP_CALIBRATE     (TRACKPAD_STEP_ROUTE_LIMIT + 1u)
#define TRACKPAD_STEP_GLOBAL_ON     (TRACKPAD_STEP_CALIBRATE + (2u * TRACKPAD_GROUPS))
#define TRACKPAD_STEPS              (TRACKPAD_STEP_GLOBAL_ON + 1u)

static bool _trackpad_setup_step(uint8_t step)
{
    static const uint8_t empty[TRACKPAD_COMPENSATION_LEN] = {0};
    // A record read by one step for the next to write back
    static uint8_t held[TRACKPAD_GROUP_LEN];

    if (step == TRACKPAD_STEP_PRESENT)
        return _trackpad_present();

    if (step == TRACKPAD_STEP_GLOBAL_READ)
        return _trackpad_read_reg(TRACKPAD_REG_GLOBAL, held, TRACKPAD_GLOBAL_LEN);

    if (step == TRACKPAD_STEP_GLOBAL_OFF)
    {
        // Frames off while the set-up goes in, the rest of that record as the controller had it
        held[0] = 0;
        return _trackpad_write(TRACKPAD_REG_GLOBAL, held, TRACKPAD_GLOBAL_LEN);
    }

    if (step < TRACKPAD_STEP_MEASUREMENTS)
    {
        const uint8_t i = (uint8_t)(step - TRACKPAD_STEP_GROUPS);
        return _trackpad_write(TRACKPAD_REG_GROUP + (i * TRACKPAD_RECORD_STEP), _trackpad_groups[i], TRACKPAD_GROUP_LEN);
    }

    if (step < TRACKPAD_STEP_NOISE)
        return _trackpad_write_measurement((uint8_t)(step - TRACKPAD_STEP_MEASUREMENTS));

    if (step < TRACKPAD_STEP_BLOCK_6)
    {
        const uint8_t i = (uint8_t)(step - TRACKPAD_STEP_NOISE);
        return _trackpad_write(TRACKPAD_REG_NOISE + (i * TRACKPAD_RECORD_STEP), _trackpad_noise[i], TRACKPAD_NOISE_LEN);
    }

    if (step < TRACKPAD_STEP_RESULT_MAPS)
        return _trackpad_write(TRACKPAD_REG_BLOCK_6 + ((step - TRACKPAD_STEP_BLOCK_6) * TRACKPAD_RECORD_STEP),
                               _trackpad_block_6, sizeof(_trackpad_block_6));

    if (step < TRACKPAD_STEP_COMPENSATION)
        return _trackpad_write_result_map((uint8_t)(step - TRACKPAD_STEP_RESULT_MAPS));

    if (step < TRACKPAD_STEP_BLOCK_9)
        return _trackpad_write(TRACKPAD_REG_COMPENSATION + ((step - TRACKPAD_STEP_COMPENSATION) * TRACKPAD_RECORD_STEP),
                               empty, TRACKPAD_COMPENSATION_LEN);

    if (step == TRACKPAD_STEP_BLOCK_9)
        return _trackpad_write(TRACKPAD_REG_BLOCK_9, _trackpad_block_9, sizeof(_trackpad_block_9));

    if (step < TRACKPAD_STEP_ROUTE_LIMIT)
    {
        // The routes past those in use are written empty
        const uint8_t i = (uint8_t)(step - TRACKPAD_STEP_ROUTES);
        const uint8_t used = sizeof(_trackpad_routes) / sizeof(_trackpad_routes[0]);
        return _trackpad_write(TRACKPAD_REG_ROUTE + (i * TRACKPAD_RECORD_STEP), (i < used) ? _trackpad_routes[i] : empty,
                               TRACKPAD_ROUTE_LEN);
    }

    if (step == TRACKPAD_STEP_ROUTE_LIMIT)
        return _trackpad_write(TRACKPAD_REG_ROUTE_LIMIT, _trackpad_route_limit, sizeof(_trackpad_route_limit));

    if (step < TRACKPAD_STEP_GLOBAL_ON)
    {
        const uint8_t at = (uint8_t)(step - TRACKPAD_STEP_CALIBRATE);
        const uint32_t reg = TRACKPAD_REG_GROUP + ((at / 2u) * TRACKPAD_RECORD_STEP);
        if (!(at & 1u))
            return _trackpad_read_reg(reg, held, TRACKPAD_GROUP_LEN);
        held[1] |= TRACKPAD_GROUP_CALIBRATE;
        return _trackpad_write(reg, held, TRACKPAD_GROUP_LEN);
    }

    return _trackpad_write(TRACKPAD_REG_GLOBAL, _trackpad_global, sizeof(_trackpad_global));
}

// ---- Frames ----

// One frame's readings, as the controller sends them (38 values, least significant byte first)
typedef struct
{
    int16_t left_x[TRACKPAD_ELECTRODES];
    int16_t left_y[TRACKPAD_ELECTRODES];
    int16_t right_x[TRACKPAD_ELECTRODES];
    int16_t right_y[TRACKPAD_ELECTRODES];
    int16_t right_pressure;
    int16_t left_pressure;
    int16_t right_grip;
    int16_t left_grip;
    int16_t right_stick;
    int16_t left_stick;
} trackpad_frame_s;

_Static_assert(sizeof(trackpad_frame_s) == TRACKPAD_FRAME_PAYLOAD_LEN, "a frame is 38 readings");

// Where a finger is along eight electrodes: the middle of what they read between them, in 256ths of
// an electrode. Below the floor an electrode is not counted: it is at rest, or far from the finger.
// False when none counts.
static bool _trackpad_centre(const int16_t *electrodes, int32_t *centre)
{
    int32_t total = 0;
    int32_t weighted = 0;

    for (uint8_t i = 0; i < TRACKPAD_ELECTRODES; i++)
    {
        const int32_t reading = (int32_t)electrodes[i] - TRACKPAD_FLOOR;
        if (reading <= 0)
            continue;
        total += reading;
        weighted += reading * i;
    }
    if (!total)
        return false;

    *centre = (weighted * 256) / total;
    return true;
}

// A centre as a position from edge to edge. Both ways the last electrode is at the edge positions
// count down towards (the left, the top), so the scale is turned round.
static int16_t _trackpad_position(int32_t centre)
{
    int32_t along = ((centre - TRACKPAD_CENTRE_LOW) * (2 * TRACKPAD_EDGE)) / (TRACKPAD_CENTRE_HIGH - TRACKPAD_CENTRE_LOW);
    along = TRACKPAD_EDGE - along;
    if (along > TRACKPAD_EDGE)
        along = TRACKPAD_EDGE;
    if (along < -TRACKPAD_EDGE)
        along = -TRACKPAD_EDGE;
    return (int16_t)along;
}

static void _trackpad_pad_update(trackpad_pad_s *pad, const int16_t *x, const int16_t *y, int16_t pressure)
{
    int16_t strongest = 0;
    for (uint8_t i = 0; i < TRACKPAD_ELECTRODES; i++)
    {
        if (x[i] > strongest)
            strongest = x[i];
        if (y[i] > strongest)
            strongest = y[i];
    }

    // A finger arriving has to read more than one staying does
    pad->touched = strongest >= (pad->touched ? TRACKPAD_TOUCH_STAY : TRACKPAD_TOUCH_START);
    if (!pad->touched)
    {
        pad->pressed = false;
        pad->pressure = 0;
        return;
    }

    // An axis with nothing to go on keeps where it was
    int32_t centre;
    if (_trackpad_centre(x, &centre))
        pad->x = _trackpad_position(centre);
    if (_trackpad_centre(y, &centre))
        pad->y = _trackpad_position(centre);

    pad->pressed = pressure >= (pad->pressed ? TRACKPAD_PRESS_STAY : TRACKPAD_PRESS_START);

    int32_t scaled = (pressure > 0) ? ((int32_t)pressure * TRACKPAD_PRESSURE_SCALE) : 0;
    if (scaled < 1)
        scaled = 1;
    if (scaled > TRACKPAD_EDGE)
        scaled = TRACKPAD_EDGE;
    pad->pressure = (uint16_t)scaled;
}

// ---- The task ----

typedef enum
{
    TRACKPAD_OFF,
    TRACKPAD_SETUP,
    TRACKPAD_RUNNING,
    TRACKPAD_FAILED,
} trackpad_phase_t;

static trackpad_phase_t _trackpad_phase = TRACKPAD_OFF;
static uint8_t _trackpad_step = 0;
static uint8_t _trackpad_tries = 0;
static uint64_t _trackpad_next_us = 0;   // Not before this: the next command, or the next try
static bool _trackpad_reading = false;   // A frame is being read in the background
static uint64_t _trackpad_frame_us = 0;  // When the last frame came
static uint8_t _trackpad_frame[TRACKPAD_FRAME_HEAD_LEN + TRACKPAD_FRAME_PAYLOAD_LEN];
static trackpad_pad_s _trackpad_left;
static trackpad_pad_s _trackpad_right;

static void _trackpad_reset(uint64_t now_us)
{
    gpio_hal_write(PGPIO_TRACKPAD_RESET, false);
    sys_hal_sleep_ms(TRACKPAD_RESET_MS);
    gpio_hal_write(PGPIO_TRACKPAD_RESET, true);

    _trackpad_phase = TRACKPAD_SETUP;
    _trackpad_step = 0;
    _trackpad_next_us = now_us + (TRACKPAD_AWAKE_MS * 1000u);
}

void trackpad_start(void)
{
    // Low while a frame waits
    nrf52_gpio_init_input(PGPIO_TRACKPAD_READY, true, false);

    _trackpad_phase = TRACKPAD_SETUP;
    _trackpad_step = 0;
    _trackpad_tries = 0;
    _trackpad_reading = false;
    _trackpad_next_us = sys_hal_now_us() + (TRACKPAD_AWAKE_MS * 1000u);
    memset(&_trackpad_left, 0, sizeof(_trackpad_left));
    memset(&_trackpad_right, 0, sizeof(_trackpad_right));
}

static void _trackpad_frame_done(uint64_t now_us)
{
    _trackpad_reading = false;
    if (nrf52_i2c_read_result(TRACKPAD_I2C_INSTANCE) != (int)sizeof(_trackpad_frame))
        return;

    const uint16_t payload = (uint16_t)(_trackpad_frame[3] | (_trackpad_frame[4] << 8));
    if (_trackpad_frame[2] != TRACKPAD_FRAME_REPORT_ID)
        return;
    if (payload != TRACKPAD_FRAME_PAYLOAD_LEN)
    {
        // Something other than readings, which this read was the wrong length for: read whole, to
        // take it out of the controller's way
        static uint8_t other[TRACKPAD_FRAME_HEAD_LEN + TRACKPAD_DATA_MAX_LEN];
        if (payload <= TRACKPAD_DATA_MAX_LEN)
            i2c_hal_read_timeout_us(TRACKPAD_I2C_INSTANCE, TRACKPAD_I2C_ADDRESS, other,
                                    TRACKPAD_FRAME_HEAD_LEN + payload, false, TRACKPAD_I2C_TIMEOUT_US);
        return;
    }

    trackpad_frame_s frame;
    memcpy(&frame, &_trackpad_frame[TRACKPAD_FRAME_HEAD_LEN], sizeof(frame));
    _trackpad_pad_update(&_trackpad_left, frame.left_x, frame.left_y, frame.left_pressure);
    _trackpad_pad_update(&_trackpad_right, frame.right_x, frame.right_y, frame.right_pressure);
    _trackpad_frame_us = now_us;
}

void trackpad_task(uint64_t now_us)
{
    switch (_trackpad_phase)
    {
        case TRACKPAD_SETUP:
            if (now_us < _trackpad_next_us)
                break;
            if (!_trackpad_setup_step(_trackpad_step))
            {
                _trackpad_phase = TRACKPAD_FAILED;
                _trackpad_next_us = now_us + TRACKPAD_RETRY_US;
                break;
            }
            // The controller wants a pause before each command
            _trackpad_next_us = sys_hal_now_us() + TRACKPAD_COMMAND_GAP_US;
            if (++_trackpad_step >= TRACKPAD_STEPS)
                _trackpad_phase = TRACKPAD_RUNNING;
            break;

        case TRACKPAD_RUNNING:
            if (_trackpad_reading)
            {
                if (nrf52_i2c_read_busy(TRACKPAD_I2C_INSTANCE))
                    break;
                _trackpad_frame_done(now_us);
            }

            // The head and the readings in one read: every frame the set-up asks for is this long
            if (!gpio_hal_read(PGPIO_TRACKPAD_READY))
                _trackpad_reading = nrf52_i2c_read_start(TRACKPAD_I2C_INSTANCE, TRACKPAD_I2C_ADDRESS, _trackpad_frame,
                                                         sizeof(_trackpad_frame));

            // Frames stop when nothing is touched or held, and the last one may have had a finger in it
            if ((now_us - _trackpad_frame_us) > TRACKPAD_STALE_US)
            {
                memset(&_trackpad_left, 0, sizeof(_trackpad_left));
                memset(&_trackpad_right, 0, sizeof(_trackpad_right));
            }
            break;

        case TRACKPAD_FAILED:
            // A controller that did not take its set-up is reset and given it again, a few times
            if ((_trackpad_tries < TRACKPAD_TRIES) && (now_us >= _trackpad_next_us))
            {
                _trackpad_tries++;
                _trackpad_reset(now_us);
            }
            break;

        default:
            break;
    }
}

bool trackpad_busy(void)
{
    return _trackpad_reading && nrf52_i2c_read_busy(TRACKPAD_I2C_INSTANCE);
}

void trackpad_get(trackpad_pad_s *left, trackpad_pad_s *right)
{
    *left = _trackpad_left;
    *right = _trackpad_right;
}
