#ifndef BLACKBODY_GUI_H_
#define BLACKBODY_GUI_H_

#include <TFT_eSPI.h>         // graphics
#include <Adafruit_TSC2007.h> // touch controller

#include "blackbody.h"
#include "ip_address.h"

#include <limits.h>

// represents the different signals that can be recieved from the screen
// e.g. currState = incrPressed when the increment button is pressed
enum ButtonState
{
	nonePressed,
	incrPressed,
	decrPressed,
	setpointNumberPressed,
	unlockRegionPressed,
	configPressed,
	ipModePressed,
	addressPressed,
	readyWindowPressed,
	backPressed,
	savePressed,
	byteZeroPressed,
	byteOnePressed,
	byteTwoPressed,
	byteThreePressed,
	nextPressed,
	errorPressed,
	fetchErrorPressed
};
enum Page
{
	homePage,
	configPage,
	errorPage,
	addressAdjustPage,
	windowAdjustPage
};

class Blackbody_GUI
{
public:
	TFT_eSPI tft = TFT_eSPI(); // display
	Adafruit_TSC2007 touchController;
	Blackbody blackbody;
	
	float targetPoint = 0; // (number displayed when setpoint region is unlocked and being adjusted)
	unsigned targetPointColor = TFT_LIGHTGREY;
	bool locked = true;       // controls if set-point is greyed out
	Page currPage = homePage; // organizes pages
	unsigned prevStatus = blackbody.status;

	void updateHomePageState(ButtonState buttonState, ButtonState prevButtonState);
	void updateConfigPageState(ButtonState buttonState, ButtonState prevButtonState);
	void updateAddressAdjustPageState(ButtonState buttonState, ButtonState prevButtonState);
	void updateWindowAdjustPageState(ButtonState buttonState, ButtonState prevButtonState);
	void updateErrorPageState(ButtonState buttonState, ButtonState prevButtonState);

	ButtonState parseHomePageTouch(const unsigned x, const unsigned y),
				parseConfigPageTouch(const unsigned x, const unsigned y),
				parseAddressPageTouch(const unsigned x, const unsigned y),
				parseWindowPageTouch(const unsigned x, const unsigned y),
				parseErrorPageTouch(const unsigned x, const unsigned y);

	void drawStatus(const unsigned status),
		 drawLocked(),
		 drawUnlocked(),
		 drawSetPointRegion(),
		 drawConfigButton(),
		 drawBootScreen(),
		 drawSetPoint(float number, uint16_t fgColor, uint16_t bgColor, uint16_t xPos, uint16_t yPos),
		 initDisplayGraphics(),
		 initTouchControl(),
		 drawConfigScreen();

private:
	bool prevIpMode = blackbody.ipMode;
	float prevReadyWindow = blackbody.readyWindow;
	bool prospectiveIpMode = blackbody.ipMode;
	float prospectiveReadyWindow = blackbody.readyWindow;
	ip_address prospectiveIpAddress = ip_address(192, 168, 200, 161);
	ip_address prevIpAddress = ip_address(192, 168, 200, 161);
	unsigned currByteSelected = 3;

	std::string currErrorDevice = "";
	bool ERROR_CLEARED = false;

	// guard conditions used to implement the button hold auto-increment/decrement feature
	bool incrHeld = false;
	bool decrHeld = false;
	bool incrEnable = false;
	bool decrEnable = false;

	// time tracking variables. roll over after 47.9 days
	// code should be roll over safe - not yet tested
	unsigned long	timeIncrFirstPressed = ULONG_MAX,
					timeIncrLastPressed = ULONG_MAX,
					timeDecrFirstPressed = 0,
					timeDecrLastPressed = 0,
					timeLastActivity = 0;
	unsigned autoRate = 1;

	// sprites
	TFT_eSprite upArrowActive = TFT_eSprite(&tft);
	TFT_eSprite upArrowInactive = TFT_eSprite(&tft);
	TFT_eSprite downArrowActive = TFT_eSprite(&tft);
	TFT_eSprite downArrowInactive = TFT_eSprite(&tft);
	TFT_eSprite backArrow = TFT_eSprite(&tft);
	TFT_eSprite smallBackArrow = TFT_eSprite(&tft);
	TFT_eSprite checkmark = TFT_eSprite(&tft);
	TFT_eSprite downArrow = TFT_eSprite(&tft);

	// config page
	void drawIPMode(bool mode, const unsigned x, const unsigned y);
	void drawAddress(const unsigned x, const unsigned y, const unsigned color);
	void drawReadyWindow(const unsigned x, const unsigned y);
	void drawErrorScreen();
	void drawStringWordWrap(const std::string& input, const size_t maxLineLength, const int x, const int y);

	bool is_equal(float f1, float f2, float epsilon);
};

#endif // BLACKBODY_GUI_H_
