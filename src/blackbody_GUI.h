#ifndef BLACKBODY_GUI_H_
#define BLACKBODY_GUI_H_

#include <TFT_eSPI.h>			    // graphics
#include <Adafruit_TSC2007.h>	// touch controller

#include "blackbody.h"
#include "ip_address.h"

#include <limits.h>

// represents the different signals that can be recieved from the screen
// e.g. currState = incrPressed when the increment button is pressed
enum ButtonState {nonePressed, incrPressed, decrPressed, setpointNumberPressed, unlockRegionPressed, configPressed, ipModePressed, addressPressed, readyWindowPressed, backPressed};
enum Page {homePage, configPage, errorPage, addressAdjustPage};

class Blackbody_GUI
{
  public:
  TFT_eSPI tft = TFT_eSPI();  // display
  Adafruit_TSC2007 touchController;
  Blackbody blackbody;
  float targetPoint = 0;      // (number displayed when setpoint region is unlocked and being adjusted)
  unsigned targetPointColor = TFT_LIGHTGREY;
  bool locked = true;         // controls if set-point is greyed out
  Page currPage = homePage;   // organizes pages

  void updateHomePageState(ButtonState buttonState, ButtonState prevButtonState);
  void updateConfigPageState(ButtonState buttonState, ButtonState prevButtonState);
  ButtonState parseHomePageTouch(const unsigned x, const unsigned y);
  ButtonState parseConfigPageTouch(const unsigned x, const unsigned y);

  void drawStatus(const unsigned status);
  void drawLocked();
  void drawUnlocked();
  void drawSetPointRegion();
  void drawConfigButton();
  void drawBootScreen();
  void drawSetPoint(float number, uint16_t fgColor, uint16_t bgColor, uint16_t xPos, uint16_t yPos);
  void initDisplayGraphics();
  void initTouchControl();
  void drawConfigScreen();

  private:
  // config settings
  bool ipMode = 1;  // 1 = DHCP, 0 = STATIC
  ip_address address = ip_address(192, 168, 0, 1);

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

  // home page
  bool isIncrButton(const unsigned x, const unsigned y);
  bool isDecrButton(const unsigned x, const unsigned y);
  bool isSetpointNumber(const unsigned x, const unsigned y);
  bool isUnlockRegion(const unsigned x, const unsigned y);
  bool isError(const unsigned x, const unsigned y);
  bool isConfig(const unsigned x, const unsigned y);

  // config page
  bool isIPMode(const unsigned x, const unsigned y);
  bool isAddress(const unsigned x, const unsigned y);
  bool isReadyWindow(const unsigned x, const unsigned y);
  bool isBackButton(const unsigned x, const unsigned y);

  void drawIPMode(bool mode, const unsigned x, const unsigned y);
  void drawAddress(const unsigned x, const unsigned y, const unsigned color);
  void drawReadyWindow(const unsigned x, const unsigned y);
};

#endif  // BLACKBODY_GUI_H_
