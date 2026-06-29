#pragma once

#include "src/ButtonHandling/ButtonHandler.h"

/// Converts a button press type into the bitmask returned by checkButtonActivity().
/// Bit 0 is set for a short press, bit 1 for a long press.
int buttonPressTypeToBitmask(ButtonPressType pressType);
