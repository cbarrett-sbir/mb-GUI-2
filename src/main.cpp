/*******************************************************
PROGRAM NAME - MB-GUI

AUTHOR - C. BARRETT, 2023/04/24

USAGE - Control temperature and settings of SBIR's
low-cost blackbody product via a touchscreen GUI.

DATE - Started 2023/02/12

BUGS - Very infrequently a single touchController to incr/decr will
adjust the set-point > 1 unit.

DESCRIPTION - This program operates on the simple grand 
loop paradigm. Touch coordinates are read from the TSC2007 
touchController controller, then translated into button presses
through simple x/y comparison. Based on the current state
of the screen, logic changes the appearance of the GUI in
the switch statement.
*******************************************************/

#include "blackbody.h"
#include "blackbody_GUI.h"
#include "serial_methods.h"

//AG
#define BACKLIGHT_PWM 2
//AG

Blackbody_GUI GUI;

// ButtonState type from "blackbody_GUI.h"
ButtonState buttonState = nonePressed;
ButtonState prevButtonState = nonePressed;

unsigned long prevTime = 0;
unsigned prevStatus = 0;
// buffer for incoming serial data
std::string buffer = "";

void setup()
{
	GUI.tft.begin();
	GUI.tft.setRotation(3);		// landscape
	GUI.tft.setSwapBytes(true); // Swap the color byte order when rendering
	digitalWrite(BACKLIGHT_PWM, HIGH);

	//GUI.drawBootScreen();

	//GUI.initDisplayGraphics();
	//GUI.initTouchControl();
	GUI.drawKeypad();

	Serial.begin(19200);
	Serial5.begin(115200);
}

void loop()
{
	prevStatus = GUI.blackbody.status;

	// check for response from blackbody SCC
	if (Serial.available())		// changed from Serial5 to Serial 
								// to test with other screen.
								// 6 locations in main.cpp,
								// 1 in serial_methods -c.b
	{
		readSerial(buffer, GUI.blackbody);
	}

	// every 0.5s request the source plate
	// temperature and BB status over UART
	if(millis() - prevTime > 500)
	{
		prevTime = millis();
		Serial.println("M2");
		Serial.println("MS");
	}

	// read touch data from controller
	uint16_t x, y, z1, z2;  // z1 ~= pressure
	GUI.touchController.read_touch(&y, &x, &z1, &z2);
	//AG Serial.print("Touch point: (");
	 //Serial.print(x); Serial.print(", ");
	 //Serial.print(y); Serial.print(", ");
	 //Serial.print(z1); Serial.print(" / ");
	 //Serial.print(z2); Serial.println(")");
	//AG

	prevButtonState = buttonState;

	// convert from touch cooridnates to button presses
	if (z1 > 3)		// ignore weak presses 
					// (strong press reads about 200-500)
	{
		buttonState = GUI.parseTouch(x, y);
	}
	else { buttonState = nonePressed; }

	GUI.updateState(buttonState, prevButtonState);

	// unlock setpoint field if you press
	// within it
	if(GUI.locked && prevButtonState == unlockRegionPressed)
	{
		GUI.drawUnlocked();
		GUI.locked = false;
	}

	if(GUI.blackbody.status != prevStatus)
	{
		GUI.drawStatus(GUI.blackbody.status);
	}

	// finally, update sourcePlateTemp and setPoint on screen
	GUI.drawSetPoint(GUI.targetPoint, GUI.targetPointColor, TFT_WHITE, 310, 125);
	GUI.drawSetPoint(GUI.blackbody.sourcePlateTemp, TFT_BLACK, TFT_WHITE, 10 + GUI.tft.textWidth("111.1", 7), 50);
}
