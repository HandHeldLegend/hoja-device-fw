#include "trackpad_haptics.h"

#include <stdint.h>

#include "main.h"

#include "hal/gpio_hal.h"
#include "nrf52_hal.h"

// The two actuators hang off an amplifier that takes PWM: the duty is the signal, half is rest. As
// the stock firmware drives it: a 62.5 kHz carrier (16 MHz counted to 256), each sample held for 16
// periods (3.9 kHz), the left channel upside down, the amplifier's enable up only while something
// plays. The clicks themselves are ours: one swing each way, held a couple of milliseconds each.
// A short 880 Hz tone could barely be felt; what could was the thump described below, a step held
// for a millisecond, so the click is made of steps.
//
// The carrier never stops. Stopped, its pins fall to no duty at all, which to the amplifier is a
// full swing one way: with the carrier stopped at the end of each click while the amplifier was
// still on, both pads thumped whichever one was pressed.

#define HAPTICS_PWM             NRF_PWM0
#define HAPTICS_TOP             256u
#define HAPTICS_REST            128
#define HAPTICS_HOLD            15u // Periods a sample is repeated for, after its first
#define HAPTICS_CHANNELS        4u  // The unit loads four values a sample, used or not
#define HAPTICS_POLARITY        0x8000u // High for the count given, then low
// The amplifier is given rest for a moment before the pulse, and after it
#define HAPTICS_LEAD            4u
#define HAPTICS_TAIL            4u

// A click is this many samples one way, then as many the other (a sample is a quarter millisecond).
// By feel: the push at full swing (127) was a bit strong, and a release of 3 ms at 76 was not felt.
#define HAPTICS_DOWN_SAMPLES    16u // 4 ms
#define HAPTICS_DOWN_LEVEL      105
#define HAPTICS_UP_SAMPLES      16u // 4 ms
#define HAPTICS_UP_LEVEL        88

#define HAPTICS_SEQUENCE_MAX    (HAPTICS_LEAD + HAPTICS_DOWN_SAMPLES + HAPTICS_TAIL)

static int8_t _haptics_down[HAPTICS_DOWN_SAMPLES];
static int8_t _haptics_up[HAPTICS_UP_SAMPLES];
static uint16_t _haptics_sequence[HAPTICS_SEQUENCE_MAX][HAPTICS_CHANNELS];
static bool _haptics_playing = false;

static void _haptics_pulse(int8_t *out, uint16_t samples, int8_t level)
{
    for (uint16_t i = 0; i < samples; i++)
        out[i] = (i < (samples / 2u)) ? level : (int8_t)-level;
}

static uint16_t _haptics_duty(int value, bool upside_down)
{
    int duty = HAPTICS_REST + (upside_down ? -value : value);
    if (duty < 1)
        duty = 1;
    if (duty > (int)(HAPTICS_TOP - 1u))
        duty = (int)(HAPTICS_TOP - 1u);
    return (uint16_t)(HAPTICS_POLARITY | (uint16_t)duty);
}

// Plays what is in the sequence. The unit holds the last sample once it is through, and every
// sequence ends at rest.
static void _haptics_play(uint16_t samples)
{
    HAPTICS_PWM->SEQ[0].CNT = (uint32_t)samples * HAPTICS_CHANNELS;
    HAPTICS_PWM->EVENTS_SEQEND[0] = 0;
    HAPTICS_PWM->TASKS_SEQSTART[0] = 1;
}

static void _haptics_fill(uint16_t at, int left, int right)
{
    _haptics_sequence[at][0] = _haptics_duty(left, true);
    _haptics_sequence[at][1] = _haptics_duty(right, false);
    _haptics_sequence[at][2] = HAPTICS_POLARITY;
    _haptics_sequence[at][3] = HAPTICS_POLARITY;
}

void trackpad_haptics_start(void)
{
    _haptics_pulse(_haptics_down, HAPTICS_DOWN_SAMPLES, HAPTICS_DOWN_LEVEL);
    _haptics_pulse(_haptics_up, HAPTICS_UP_SAMPLES, HAPTICS_UP_LEVEL);

    HAPTICS_PWM->ENABLE = PWM_ENABLE_ENABLE_Disabled;
    HAPTICS_PWM->PSEL.OUT[0] = nrf52_psel(PGPIO_PAD_HAPTIC_LEFT);
    HAPTICS_PWM->PSEL.OUT[1] = nrf52_psel(PGPIO_PAD_HAPTIC_RIGHT);
    HAPTICS_PWM->PSEL.OUT[2] = NRF52_PSEL_DISCONNECTED;
    HAPTICS_PWM->PSEL.OUT[3] = NRF52_PSEL_DISCONNECTED;
    HAPTICS_PWM->MODE = PWM_MODE_UPDOWN_Up;
    HAPTICS_PWM->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_1;
    HAPTICS_PWM->COUNTERTOP = HAPTICS_TOP;
    HAPTICS_PWM->LOOP = 0;
    HAPTICS_PWM->DECODER = (PWM_DECODER_LOAD_Individual << PWM_DECODER_LOAD_Pos) |
                           (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    HAPTICS_PWM->SEQ[0].PTR = (uint32_t)_haptics_sequence;
    HAPTICS_PWM->SEQ[0].REFRESH = HAPTICS_HOLD;
    HAPTICS_PWM->SEQ[0].ENDDELAY = 0;
    HAPTICS_PWM->SHORTS = 0;
    HAPTICS_PWM->INTENCLR = 0xFFFFFFFFu;
    HAPTICS_PWM->ENABLE = PWM_ENABLE_ENABLE_Enabled;

    // The carrier, at rest from here on (the amplifier is off until a click)
    for (uint16_t i = 0; i < HAPTICS_LEAD; i++)
        _haptics_fill(i, 0, 0);
    _haptics_play(HAPTICS_LEAD);
    _haptics_playing = false;
}

void trackpad_haptics_click(bool left, bool right, bool down)
{
    if (!left && !right)
        return;

    const int8_t *pulse = down ? _haptics_down : _haptics_up;
    const uint16_t samples = down ? HAPTICS_DOWN_SAMPLES : HAPTICS_UP_SAMPLES;
    const uint16_t total = (uint16_t)(HAPTICS_LEAD + samples + HAPTICS_TAIL);

    // A click still playing is cut short by this one
    for (uint16_t i = 0; i < total; i++)
    {
        const bool in_pulse = (i >= HAPTICS_LEAD) && (i < (HAPTICS_LEAD + samples));
        const int value = in_pulse ? pulse[i - HAPTICS_LEAD] : 0;
        _haptics_fill(i, left ? value : 0, right ? value : 0);
    }

    // The carrier is at rest as the amplifier comes on, and the pad not clicked stays there
    gpio_hal_write(PGPIO_PAD_HAPTIC_ENABLE, true);
    _haptics_play(total);
    _haptics_playing = true;
}

void trackpad_haptics_task(void)
{
    if (!_haptics_playing || !HAPTICS_PWM->EVENTS_SEQEND[0])
        return;

    // Through, and back at rest: the amplifier goes off
    HAPTICS_PWM->EVENTS_SEQEND[0] = 0;
    gpio_hal_write(PGPIO_PAD_HAPTIC_ENABLE, false);
    _haptics_playing = false;
}
