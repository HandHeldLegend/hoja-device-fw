#ifndef STEAM_CONTROLLER_TRACKPAD_H
#define STEAM_CONTROLLER_TRACKPAD_H

// The two trackpads, the grip sensors and the sticks' touch sensing: one Cirque touch controller
// (the stock firmware's "Olympus") on the buttons' I2C bus. It measures; it does not work out where
// a finger is. A frame is what each electrode read, eight across and eight down for each pad, and one
// number each for a pad's pressure, a grip and a stick's cap. The position, the touch and the click
// are worked out here.
//
// Read off a controller (a finger slid edge to edge, a press, a thumb on each stick): across a pad,
// electrode 7 is at its left edge and 0 at its right; down it, 0 is at the bottom and 7 at the top.
// An electrode under a finger reads 2000 to 3000 and one at rest within 20 of zero. A hard press
// took a pad's pressure from under 150 (a finger resting) to 2500 to 4000, and a thumb on a stick
// took its number from near zero to about 1100. The grips rest anywhere from 0 to 2000 and move
// with the hands holding the gamepad: not worked out yet, and not used.
//
// The controller sends frames only while something is touched or held: a gamepad lying on a desk
// sends none.

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool touched;
    bool pressed;      // Pushed down: the pad's click (it has no switch of its own)
    int16_t x;         // The left edge is -32767, the right edge 32767
    int16_t y;         // The top edge is -32767, the bottom edge 32767
    uint16_t pressure; // Up to 32767, and never 0 while touched
} trackpad_pad_s;

// After the controller's reset has been let go
void trackpad_start(void);

// Each pass of the main loop. First the controller's measurement set-up, which it keeps none of
// over a reset: 86 commands, one a pass and 2 ms apart as it wants them, so about a quarter of a
// second in which the loop keeps running. Then its frames, each read in the background.
void trackpad_task(uint64_t now_us);

// A frame is being read: the buttons' bus is taken for up to 2 ms, and a read of them would wait
bool trackpad_busy(void);

// As of the last frame. Nothing touched before the first, or once frames have stopped coming.
void trackpad_get(trackpad_pad_s *left, trackpad_pad_s *right);

#endif
