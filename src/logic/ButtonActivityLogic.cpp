#include "ButtonActivityLogic.h"

int buttonPressTypeToBitmask(ButtonPressType pressType) {
  int returnValue = 0;
  if (pressType == SHORT_PRESS) {
    returnValue |= (1 << 0);
  } else if (pressType == LONG_PRESS) {
    returnValue |= (1 << 1);
  }
  return returnValue;
}
