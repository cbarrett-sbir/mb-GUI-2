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

#include <TFT_eSPI.h>			// graphics library
#include <Adafruit_TSC2007.h>	// touchController controller
#include <WireIMXRT.h>

#include "blackbody.h"
#include "blackbody_GUI.h"
#include "serial_methods.h"
#include "boot_background.h"
#include "up_arrow_active.h"
#include "up_arrow_inactive.h"

//AG
#define BACKLIGHT_PWM 2
//AG
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite upArrowActive = TFT_eSprite(&tft);
TFT_eSprite upArrowInactive = TFT_eSprite(&tft);
TFT_eSprite downArrowActive = TFT_eSprite(&tft);
TFT_eSprite downArrowInactive = TFT_eSprite(&tft);
Adafruit_TSC2007 touchController;

Blackbody blackbody; // class containing live data about MB-08's blackbody
Blackbody_GUI GUI;

// activates print statements about current state
bool debug = false;

// ButtonState type from "blackbody_GUI.h"
ButtonState buttonState = nonePressed;
ButtonState prevState = nonePressed;

unsigned long prevTime = 0;
unsigned prevStatus = 0;
// buffer for incoming serial data
std::string buffer = "";

// calculate max number length so set-point/temp is fully written over
int padding = tft.textWidth("111.1", 7);

// helper function to clean up float printing code
void drawSetPoint(float number, uint16_t fgColor, uint16_t bgColor, uint16_t xPos, uint16_t yPos)
{
	tft.setTextPadding(padding);	// padding is global
	tft.setTextDatum(MR_DATUM);		// always right aligned
	tft.setTextColor(fgColor, bgColor, true);
	tft.drawFloat(number, 1, xPos, yPos, 7); // last argument (7) indicates font # 7 which is "7-seg digits"
}

void initDisplayGraphics(TFT_eSPI& tft)
{
	tft.fillScreen(TFT_WHITE);

	upArrowActive.createSprite(32, 32);
	upArrowActive.setSwapBytes(true); // change this if colors are weird
	upArrowActive.pushImage(0, 0, 32, 32, up_arrow_active);

	upArrowInactive.createSprite(32, 32);
	upArrowInactive.setSwapBytes(false); // change this if colors are weird
	upArrowInactive.pushImage(0, 0, 32, 32, up_arrow_inactive);

	downArrowActive.createSprite(32, 32);
	downArrowActive.setSwapBytes(true); // change this if colors are weird
	downArrowActive.pushImage(0, 0, 32, 32, up_arrow_active);

	downArrowInactive.createSprite(32, 32);
	downArrowInactive.setSwapBytes(true); // change this if colors are weird
	downArrowInactive.pushImage(0, 0, 32, 32, up_arrow_inactive);


	GUI.drawSetPointRegion(tft);
	GUI.drawConfigButton(tft);

	GUI.drawLocked(tft, upArrowInactive, downArrowInactive);

	GUI.drawStatus(tft, blackbody.status);
}

void initTouchControl(Adafruit_TSC2007& touchController)
{
	if (!touchController.begin(72, &Wire1)) // SCL = 16, SDA = 17
	{
		if (debug)
			Serial.println("Couldn't find touchController");
		while (1) delay(10);
	}
	if (debug)
		Serial.println("Found touchController controller");
}

void setup()
{
	tft.begin();
	tft.setRotation(3);		// landscape
	tft.setSwapBytes(true); // Swap the color byte order when rendering
	digitalWrite(BACKLIGHT_PWM, HIGH);

	// tft.pushImage(0, 0, BG_WIDTH, BG_HEIGHT, boot_background);
	// tft.setTextDatum(MC_DATUM);
	// tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
	// tft.drawString("INITIALIZING", 160, 110, 4);
	// tft.setTextDatum(MR_DATUM);
	// tft.setTextPadding(padding);
	// for (int seconds(20); seconds > 0; seconds--)
	// {
	// 	tft.drawNumber(seconds, 185, 170, 7);
	// 	delay(1000);
	// }

	initDisplayGraphics(tft);
	initTouchControl(touchController);

	Serial.begin(19200);
	Serial5.begin(115200);
}

void loop()
{
	prevStatus = blackbody.status;

	// read incoming serial transmission in a non-blocking fashion
	if (Serial.available())		// changed from Serial5 to Serial to test with other screen.
								// 6 locations in main.cpp, 1 in serial_methods -c.b
	{
		readSerial(buffer, blackbody);
	}

	// every 0.5s send the command to request
	// the source plate temperature over UART
	if(millis() - prevTime > 500)
	{
		prevTime = millis();
		Serial.println("M2");
		Serial.println("MS");
	}

	// Read Touch Data
	uint16_t x, y, z1, z2;  // z1 ~= pressure
	touchController.read_touch(&y, &x, &z1, &z2);
	//AG Serial.print("Touch point: (");
	 //Serial.print(x); Serial.print(", ");
	 //Serial.print(y); Serial.print(", ");
	 //Serial.print(z1); Serial.print(" / ");
	 //Serial.print(z2); Serial.println(")");
	//AG
	prevState = buttonState; // record the previous state for use in identifying button transitions

	// translate touchController coordinates into distinct button presses
	if (z1 > 3)	// ignore weak presses (strong presses read about 200-500)
	{
		// the locked flag is set when the set-point is greyed out
		// it is used to ignore presses to the incr/decr buttons
		if(GUI.isIncrButton(x, y) && !GUI.locked)
		{
			buttonState = incrPressed;
		}
		else if(GUI.isDecrButton(x, y) && !GUI.locked)
		{
			buttonState = decrPressed;
		}
		else if(GUI.isUnluckNumber(x, y))
		{
			buttonState = unlockNumberPressed;
		}
		else if(GUI.isUnlockRegion(x, y) && GUI.locked)
		{
			buttonState = unlockRegionPressed;
		}
		else if(GUI.isError(x, y) && blackbody.status == 32)
		{
			tft.fillScreen(TFT_WHITE);  // splash screens
			tft.setTextDatum(MC_DATUM);
			tft.setTextColor(TFT_RED, TFT_WHITE, true);
			tft.drawString("ERROR PAGE", 160, 120, 4);
			delay(5000);
			initDisplayGraphics(tft);
		}
		else if(GUI.isConfig(x, y))
		{
			tft.fillScreen(TFT_WHITE);  // splash screens
			tft.setTextDatum(MC_DATUM);
			tft.setTextColor(TFT_RED, TFT_WHITE, true);
			tft.drawString("CONFIG PAGE", 160, 120, 4);
			delay(5000);
			initDisplayGraphics(tft);
		}
	}
	else { buttonState = nonePressed; } // indicates that no touchController is present

	if (prevState != nonePressed && buttonState == nonePressed)
	{
		if (debug)
			Serial.println("nonePressed");
		GUI.timeLastActivity = millis();
	}


	// Update state
	switch(buttonState)
	{
	case incrPressed:
	{
		if(GUI.incrHeld)	// flag that indicates if Incr button has been held > 1s
		{                 	// & controls auto-increment fucntionality
			if (debug)
				Serial.println("GUI.incrHeld");

			GUI.autoRate = 50; // 1/ ( rate[ms] ) = rate of incrementation. rate should be renamed period.
			if(prevState != incrPressed) { GUI.incrHeld = false; break; }
			if(millis() - GUI.timeIncrFirstPressed >= 5000) { GUI.autoRate = 7.5; }	// after 5s, speed up incrementation
			
			if(millis() - GUI.timeIncrLastPressed >= GUI.autoRate)
			{
				GUI.targetPoint += 0.1;
				GUI.timeIncrLastPressed = millis();
			}
		break;
		}

		if(prevState == nonePressed) // initial press
		{
			if (debug)
				Serial.println("incrInitPress");
			GUI.incrEnable = true;
			GUI.targetPoint += 0.1;
			GUI.timeIncrLastPressed = millis();
			GUI.timeIncrFirstPressed = GUI.timeIncrLastPressed;
		}
		// button held > 1s
		else if (GUI.incrEnable && (prevState == incrPressed) && (millis() - GUI.timeIncrLastPressed > 1000))
		{
			if (debug)
				Serial.println("incrHeld");
			GUI.targetPoint++;
			GUI.incrHeld = true;
			GUI.timeIncrLastPressed = millis();
			GUI.timeIncrFirstPressed = GUI.timeIncrLastPressed;
		}
		break;
	}

	case decrPressed:
	{
		if(GUI.decrHeld)	// code is symmetric to increment
		{
			if (debug)
				Serial.println("GUI.decrHeld");
			GUI.autoRate = 50;
			if(prevState != decrPressed) { GUI.decrHeld = false; break; }
			if(millis() - GUI.timeDecrFirstPressed >= 5000) { GUI.autoRate = 7.5; }
			if(millis() - GUI.timeDecrLastPressed >= GUI.autoRate)
			{
				GUI.targetPoint -= 0.1;
				GUI.timeDecrLastPressed = millis();
			}
			break;
		}

		if(prevState == nonePressed) // initial press
		{
			if (debug)
				Serial.println("decrInitPress");
			GUI.decrEnable = true;
			GUI.targetPoint -= 0.1;
			GUI.timeDecrLastPressed = millis();
			GUI.timeDecrFirstPressed = GUI.timeDecrLastPressed;
		}
		// button held
		else if (GUI.decrEnable && (prevState == decrPressed) && (millis() - GUI.timeDecrLastPressed > 1000))
		{
			if (debug)
				Serial.println("decrHeld");
			GUI.targetPoint--;
			GUI.decrHeld = true;
			GUI.timeDecrLastPressed = millis();
			GUI.timeDecrFirstPressed = GUI.timeDecrLastPressed;
		}
		break;
	}

	case unlockNumberPressed: // (unlocks the set-point region)
		{
		if(prevState != nonePressed) { break; } // ensure only initial press caught
		
		if(GUI.locked)
		{
			GUI.drawUnlocked(tft, upArrowActive, downArrowActive);
		}
		else
		{
			// change the set-point region to locked color (grey) 
			// and update set-point to the value the user entered (GUI.targetPoint)
			GUI.drawLocked(tft, upArrowInactive, downArrowInactive);
			blackbody.setPoint = GUI.targetPoint;

			// send the updated set-point over serial
			Serial.print("DA ");
			Serial.println(blackbody.setPoint);
		}

		GUI.locked = !GUI.locked; // toggle
		break;
		}

	default: // runs if currState = nonePressed (or some other uncaught state)
		GUI.decrHeld = false; // stops auto-incr/decr
		GUI.incrHeld = false;
		GUI.incrEnable = false;
		GUI.decrEnable = false;

		if(!GUI.locked)
		{
			if (millis() - GUI.timeLastActivity > 10000)
			{
				GUI.drawLocked(tft, upArrowInactive, downArrowInactive);
				GUI.targetPoint = blackbody.setPoint;
				GUI.locked = true;
			}
		}
		break;
	}

	// the following unlocks the set-point field if you press
	// anywhere within it
	if(GUI.locked && prevState == unlockRegionPressed)
	{
		GUI.drawUnlocked(tft, upArrowActive, downArrowActive);
		GUI.locked = false;
	}

	if(blackbody.status != prevStatus)
	{
		GUI.drawStatus(tft, blackbody.status);
	}

	// finally, update sourcePlateTemp and setPoint on screen
	drawSetPoint(GUI.targetPoint, GUI.targetPointColor, TFT_WHITE, 310, 125);
	drawSetPoint(blackbody.sourcePlateTemp, TFT_BLACK, TFT_WHITE, 10 + tft.textWidth("111.1", 7), 50);
}
