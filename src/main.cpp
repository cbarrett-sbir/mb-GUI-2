/*******************************************************
PROGRAM NAME - MB-GUI
 
AUTHOR - C.BARRETT, 2023/04/24

USAGE - Control temperature and settings of SBIR's
low-cost blackbody product via a touchscreen GUI.

DATE - Started 2023/02/12

BUGS - Infrequently a single touch to incr/decr will
adjust the set-point > 1 unit.

DESCRIPTION - This program operates on the simple grand
loop paradigm. Touch coordinates are read from the TSC2007
touch controller, then translated into button presses
through simple x/y comparison. Based on the current state
of the screen, logic changes the appearance of the GUI in
the switch statement.
*******************************************************/

#include "blackbody.h"
#include "blackbody_GUI.h"
#include "serial_methods.h"

// AG
#define BACKLIGHT_PWM 2
// AG

Blackbody_GUI GUI;

// ButtonState type from "blackbody_GUI.h"
ButtonState buttonState = nonePressed;
ButtonState prevButtonState = nonePressed;

unsigned long prevTime = 0;
// buffer for incoming serial data
std::string buffer = "";

void setup()
{
	GUI.tft.begin();
	GUI.tft.setRotation(3);		// landscape
	GUI.tft.setSwapBytes(true); // Swap the color byte order when rendering
	digitalWrite(BACKLIGHT_PWM, HIGH);

	//GUI.drawBootScreen();

	GUI.initDisplayGraphics();
	GUI.initTouchControl();

	Serial.begin(19200);
	Serial5.begin(115200);
}

void loop()
{
	prevButtonState = buttonState;
	GUI.prevStatus = GUI.blackbody.status;

	// check for response from blackbody SDC
	if (Serial5.available()) // changed from Serial5 to Serial
							// to test with other screen.
							// 6 locations in main.cpp,
							// 1 in serial_methods -c.b
	{
		readSerial(buffer, GUI.blackbody);
	}

	// every 1s request the source plate
	// temperature and BB status over UART
	if (millis() - prevTime > 250)
	{
		if (GUI.currPage == configPage || GUI.currPage == addressAdjustPage || GUI.currPage == windowAdjustPage)
		{
			Serial5.println("ML");
			Serial5.println("MCURRIP");
		}
		if (GUI.currPage == homePage)
		{
			Serial5.println("M2");
			Serial5.println("MDA");
		}
		Serial5.println("MS");
		prevTime = millis();
	}

	// read touch data from controller
	uint16_t x, y, z1, z2; // z1 ~= pressure
	GUI.touchController.read_touch(&y, &x, &z1, &z2);
	// Serial.print("Touch point: ("); // AG
	// Serial.print(x);
	// Serial.print(", ");
	// Serial.print(y);
	// Serial.print(", ");
	// Serial.print(z1);
	// Serial.print(" / ");
	// Serial.print(z2);
	// Serial.println(")");
	// AG

	unsigned pressureThreshold = 150;

	switch (GUI.currPage)
	{
	/***************************************************************************************
											Home Page
	***************************************************************************************/
	case homePage:
		if (z1 > pressureThreshold) // ignore weak presses
					// (strong press reads about 200-500)
		{
			buttonState = GUI.parseHomePageTouch(x, y);
		}
		else
		{
			buttonState = nonePressed;
		}

		GUI.updateHomePageState(buttonState, prevButtonState);

		if (GUI.currPage != homePage)
		{
			break;
		}

		// unlock setpoint field if you press
		// within it
		if (GUI.locked && prevButtonState == unlockRegionPressed)
		{
			GUI.drawUnlocked();
			GUI.locked = false;
		}

		if (GUI.blackbody.status != GUI.prevStatus)
		{
			GUI.drawStatus(GUI.blackbody.status);
		}

		// finally, update sourcePlateTemp and setPoint on screen
		if (GUI.locked)
		{
			GUI.drawSetPoint(GUI.blackbody.setPoint, 1, GUI.targetPointColor, TFT_WHITE, 310, 125);
		}
		else
		{
			GUI.drawSetPoint(GUI.targetPoint, 1, GUI.targetPointColor, TFT_WHITE, 310, 125);
		}
		GUI.drawSetPoint(GUI.blackbody.sourcePlateTemp, 2, TFT_BLACK, TFT_WHITE, 10 + GUI.tft.textWidth("111.1", 7), 50);
		break;

	/***************************************************************************************
											Config Page
	***************************************************************************************/
	case configPage:
		if (z1 > pressureThreshold) // ignore weak presses
					// (strong press reads about 200-500)
		{
			buttonState = GUI.parseConfigPageTouch(x, y);
		}
		else
		{
			buttonState = nonePressed;
		}

		GUI.updateConfigPageState(buttonState, prevButtonState);

		break;
	/***************************************************************************************
										Address Adjust Page
	***************************************************************************************/
	case addressAdjustPage:
		if (z1 > pressureThreshold) // ignore weak presses
					// (strong press reads about 200-500)
		{
			buttonState = GUI.parseAddressPageTouch(x, y);
		}
		else
		{
			buttonState = nonePressed;
		}

		GUI.updateAddressAdjustPageState(buttonState, prevButtonState);

		break;
	/***************************************************************************************
										Window Adjust Page
	***************************************************************************************/
	case windowAdjustPage:
		if (z1 > pressureThreshold) // ignore weak presses
					// (strong press reads about 200-500)
		{
			buttonState = GUI.parseWindowPageTouch(x, y);
		}
		else
		{
			buttonState = nonePressed;
		}

		GUI.updateWindowAdjustPageState(buttonState, prevButtonState);

		break;

	/***************************************************************************************
											Error Page
	***************************************************************************************/
	case errorPage:
		if (z1 > pressureThreshold) // ignore weak presses
					// (strong press reads about 200-500)
		{
			buttonState = GUI.parseErrorPageTouch(x, y);
		}
		else
		{
			buttonState = nonePressed;
		}

		GUI.updateErrorPageState(buttonState, prevButtonState);
		break;
	}
}
