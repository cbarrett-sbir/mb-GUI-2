#ifndef BLACKBODY_GUI_H_
#define BLACKBODY_GUI_H_

#include <TFT_eSPI.h>			    // graphics
#include <Adafruit_TSC2007.h>	// touch controller

#include "blackbody.h"

#include <limits.h>

// represents tje different signals that can be recieved from the screen
// e.g. currState = incrPressed when the increment button is pressed
enum ButtonState {nonePressed, incrPressed, decrPressed, setpointNumberPressed, unlockRegionPressed};
enum Page {homePage, configPage, errorPage};

class Blackbody_GUI
{
  public:
  // display
  TFT_eSPI tft = TFT_eSPI();

  Adafruit_TSC2007 touchController;
  Blackbody blackbody;

  // tracks the target setpoint 
  // (number displayed when setpoint region is unlocked and being adjusted)
  float targetPoint = 0;
  unsigned targetPointColor = TFT_LIGHTGREY;

  // controls if set-point is greyed out
  bool locked = true;

  void updateState(ButtonState buttonState, ButtonState prevButtonState);
  ButtonState parseTouch(const unsigned x, const unsigned y);

  void drawStatus(const unsigned status);
  void drawLocked();
  void drawUnlocked();
  void drawSetPointRegion();
  void drawConfigButton();
  void drawBootScreen();
  void drawSetPoint(float number, uint16_t fgColor, uint16_t bgColor, uint16_t xPos, uint16_t yPos);
  void initDisplayGraphics();
  void initTouchControl();
  void drawKeypad();

  private:
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

  // sprites
  TFT_eSprite upArrowActive = TFT_eSprite(&tft);
  TFT_eSprite upArrowInactive = TFT_eSprite(&tft);
  TFT_eSprite downArrowActive = TFT_eSprite(&tft);
  TFT_eSprite downArrowInactive = TFT_eSprite(&tft);

  bool isIncrButton(const unsigned x, const unsigned y);
  bool isDecrButton(const unsigned x, const unsigned y);
  bool isSetpointNumber(const unsigned x, const unsigned y);
  bool isUnlockRegion(const unsigned x, const unsigned y);
  bool isError(const unsigned x, const unsigned y);
  bool isConfig(const unsigned x, const unsigned y);
};

#endif  // BLACKBODY_GUI_H_
