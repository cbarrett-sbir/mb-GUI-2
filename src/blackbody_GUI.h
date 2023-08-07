#ifndef BLACKBODY_GUI_H_
#define BLACKBODY_GUI_H_

#include <limits.h>

// organizes the all the different signals that can be recieved from the screen
// e.g. currState = incrPressed when the increment button is pressed
enum ButtonState {nonePressed, incrPressed, decrPressed, unlockNumberPressed, unlockRegionPressed};
enum Page {homePage, configPage, errorPage};

class Blackbody_GUI
{
  public:
  // tracks the target set-point 
  // (number displayed when set-point region is unlocked and being adjusted)
  float targetPoint = 0;
  unsigned targetPointColor = TFT_LIGHTGREY;

  // controls if set-point is greyed out
  bool locked = true;

  // guard conditions used to implement the button hold auto-increment/decrement feature
  bool incrHeld = false;
  bool decrHeld = false;
  bool incrEnable = false;
  bool decrEnable = false;

  // time tracking variables. roll over after 47.9 days
  // code should be roll over safe - not yet tested
  unsigned long timeIncrFirstPressed = ULONG_MAX;
  unsigned long timeIncrLastPressed = ULONG_MAX;
  unsigned long timeDecrFirstPressed = 0;
  unsigned long timeDecrLastPressed = 0;
  unsigned long timeLastActivity = 0;
  unsigned int autoRate = 1;

  bool isIncrButton(const unsigned x, const unsigned y);
  bool isDecrButton(const unsigned x, const unsigned y);
  bool isUnluckNumber(const unsigned x, const unsigned y);
  bool isUnlockRegion(const unsigned x, const unsigned y);
  bool isError(const unsigned x, const unsigned y);
  bool isConfig(const unsigned x, const unsigned y);

  void drawStatus(TFT_eSPI& tft, const unsigned status);
  void drawLocked(TFT_eSPI& tft, TFT_eSprite& upArrow, TFT_eSprite& downArrow);
  void drawUnlocked(TFT_eSPI& tft, TFT_eSprite& upArrow, TFT_eSprite& downArrow);
  void drawSetPointRegion(TFT_eSPI& tft);
  void drawConfigButton(TFT_eSPI& tft);
};

#endif  // BLACKBODY_GUI_H_
