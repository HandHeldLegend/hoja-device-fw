#ifndef STEAM_CONTROLLER_TRACKPAD_HAPTICS_H
#define STEAM_CONTROLLER_TRACKPAD_HAPTICS_H

// The actuator under each trackpad. A pad has no switch: the click a thumb feels when it pushes one
// down is this, played by the firmware when the push registers.

#include <stdbool.h>

void trackpad_haptics_start(void);

// A click under one pad or both: the push down, or the lighter one of letting go
void trackpad_haptics_click(bool left, bool right, bool down);

// Each pass of the main loop: the amplifier goes off again once a click has played
void trackpad_haptics_task(void);

#endif
