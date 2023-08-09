#include <TFT_eSPI.h>	// graphics
#include <WireIMXRT.h>

#include "blackbody_GUI.h"

#include "up_arrow_active.h"
#include "up_arrow_inactive.h"
#include "boot_background.h"

ButtonState Blackbody_GUI::parseTouch(const unsigned x, const unsigned y)
{
	if(isIncrButton(x, y) && !locked)
	{
		return incrPressed;
	}
	else if(isDecrButton(x, y) && !locked)
	{
		return decrPressed;
	}
	else if(isSetpointNumber(x, y))
	{
		return setpointNumberPressed;
	}
	else if(isUnlockRegion(x, y) && locked)
	{
		return unlockRegionPressed;
	}
	else if(isError(x, y) && blackbody.status == 32)
	{
		tft.fillScreen(TFT_WHITE);  // splash screen
		tft.setTextDatum(MC_DATUM);
		tft.setTextColor(TFT_RED, TFT_WHITE, true);
		tft.drawString("ERROR PAGE", 160, 120, 4);
		delay(5000);
		initDisplayGraphics();
	}
	else if(isConfig(x, y))
	{
		tft.fillScreen(TFT_WHITE);  // splash screen
		tft.setTextDatum(MC_DATUM);
		tft.setTextColor(TFT_RED, TFT_WHITE, true);
		tft.drawString("CONFIG PAGE", 160, 120, 4);
		delay(5000);
		initDisplayGraphics();
	}
	
	// if nothing caught
	return nonePressed;
}

void Blackbody_GUI::updateState(const ButtonState buttonState, const ButtonState prevButtonState)
{
	bool debug = false;
	if (buttonState == nonePressed && prevButtonState != nonePressed)
	{
		// used to lock setpoint region after 10s
		timeLastActivity = millis();
	}

	// Update state
	switch(buttonState)
	{
	case incrPressed:
	{
		if(incrHeld)	// flag that indicates if Incr button has been held > 1s
		{                 	// & controls auto-increment fucntionality
			if (debug)
				Serial.println("incrHeld");

			autoRate = 50; // 1/ ( rate[ms] ) = rate of incrementation. rate should be renamed period.
			if(prevButtonState != incrPressed) { incrHeld = false; break; }
			if(millis() - timeIncrFirstPressed >= 5000) { autoRate = 7.5; }	// after 5s, speed up incrementation
			
			if(millis() - timeIncrLastPressed >= autoRate)
			{
				targetPoint += 0.1;
				timeIncrLastPressed = millis();
			}
		break;
		}

		if(prevButtonState == nonePressed) // initial press
		{
			if (debug)
				Serial.println("incrInitPress");
			incrEnable = true;
			targetPoint += 0.1;
			timeIncrLastPressed = millis();
			timeIncrFirstPressed = timeIncrLastPressed;
		}
		// button held > 1s
		else if (incrEnable && (prevButtonState == incrPressed) && (millis() - timeIncrLastPressed > 1000))
		{
			if (debug)
				Serial.println("incrHeld");
			targetPoint++;
			incrHeld = true;
			timeIncrLastPressed = millis();
			timeIncrFirstPressed = timeIncrLastPressed;
		}
		break;
	}

	case decrPressed:
	{
		if(decrHeld)	// code is symmetric to increment
		{
			if (debug)
				Serial.println("decrHeld");
			autoRate = 50;
			if(prevButtonState != decrPressed) { decrHeld = false; break; }
			if(millis() - timeDecrFirstPressed >= 5000) { autoRate = 7.5; }
			if(millis() - timeDecrLastPressed >= autoRate)
			{
				targetPoint -= 0.1;
				timeDecrLastPressed = millis();
			}
			break;
		}

		if(prevButtonState == nonePressed) // initial press
		{
			if (debug)
				Serial.println("decrInitPress");
			decrEnable = true;
			targetPoint -= 0.1;
			timeDecrLastPressed = millis();
			timeDecrFirstPressed = timeDecrLastPressed;
		}
		// button held
		else if (decrEnable && (prevButtonState == decrPressed) && (millis() - timeDecrLastPressed > 1000))
		{
			if (debug)
				Serial.println("decrHeld");
			targetPoint--;
			decrHeld = true;
			timeDecrLastPressed = millis();
			timeDecrFirstPressed = timeDecrLastPressed;
		}
		break;
	}

	case setpointNumberPressed: // (unlocks the set-point region)
		{
		if(prevButtonState != nonePressed) { break; } // ensure only initial press caught
		
		if(locked)
		{
			drawUnlocked();
		}
		else
		{
			// change the set-point region to locked color (grey) 
			// and update set-point to the value the user entered (targetPoint)
			drawLocked();
			blackbody.setPoint = targetPoint;

			// send the updated set-point over serial
			Serial.print("DA ");
			Serial.println(blackbody.setPoint);
		}

		locked = !locked; // toggle
		break;
		}

	default: // runs if currState = nonePressed (or some other uncaught state)
		decrHeld = false; // stops auto-incr/decr
		incrHeld = false;
		incrEnable = false;
		decrEnable = false;

		if(!locked)
		{
			if (millis() - timeLastActivity > 10000)
			{
				drawLocked();
				targetPoint = blackbody.setPoint;
				locked = true;
			}
		}
		break;
	}
}

void Blackbody_GUI::drawStatus(const unsigned status)
{
  unsigned xPos = 80; unsigned yPos = 140;

  if (status == 0)
  {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_DARKGREEN, TFT_WHITE, true);
    tft.setTextPadding(tft.textWidth("READY", 4));
    tft.drawString("READY", xPos, yPos, 4);
    tft.drawSmoothRoundRect(34, yPos - 20, 6, 5, 93, 34, TFT_DARKGREEN);
  }
  else if (status == 16)
  {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_RED, TFT_WHITE, true);
    tft.setTextPadding(tft.textWidth("READY", 4));
    tft.drawString("BUSY", xPos, yPos, 4);
    tft.drawSmoothRoundRect(34, yPos - 20, 6, 5, 93, 34, TFT_RED);
  }
  else if (status == 32)
  {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_RED, TFT_WHITE, true);
    tft.setTextPadding(tft.textWidth("READY", 4));
    tft.drawString("ERROR", xPos, yPos, 4);
    tft.drawSmoothRoundRect(34, yPos - 20, 6, 5, 93, 34, TFT_RED);
  }
}

void Blackbody_GUI::drawLocked()
{
  // change to locked color (GREY)
  tft.drawSmoothRoundRect(180, 25, 6, 5, 120, 60, TFT_SILVER);   // top rect
  tft.drawSmoothRoundRect(180, 165, 6, 5, 120, 60, TFT_SILVER);  // bottom rect
  upArrowInactive.pushSprite(224, 39);  // top arrow
  downArrowInactive.setPivot(224+32, 179+32);  
  downArrowInactive.pushRotated(180); // bottom arrow
  targetPointColor = TFT_SILVER;
}

void Blackbody_GUI::drawUnlocked()
{
  // change to unlocked color (BLACK)
  tft.drawSmoothRoundRect(180, 25, 6, 5, 120, 60, TFT_BLACK);
  tft.drawSmoothRoundRect(180, 165, 6, 5, 120, 60, TFT_BLACK);
  upArrowActive.pushSprite(224, 39);  // top arrow
  downArrowActive.setPivot(224+32, 179+32);  
  downArrowActive.pushRotated(180); // bottom arrow
  targetPointColor = TFT_BLACK;
}

void Blackbody_GUI::drawSetPointRegion()
{
  	tft.drawSmoothRoundRect(165, 8, 6, 5, 150, 227, TFT_BLACK);

	tft.setTextDatum(TL_DATUM);
	tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
  	tft.setTextPadding(tft.textWidth("SET POINT", 4));
	tft.drawString("SET POINT", 180, 0, 4);
}

void Blackbody_GUI::drawConfigButton()
{
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
  tft.drawString("CONFIG", 80, 190, 4);
  tft.drawSmoothRoundRect(30, 170, 6, 5, 100, 34, TFT_BLACK);
}

void Blackbody_GUI::drawSetPoint(float number, uint16_t fgColor, uint16_t bgColor, uint16_t xPos, uint16_t yPos)
{
  tft.setTextPadding(tft.textWidth("111.1", 7));
  tft.setTextDatum(MR_DATUM);		// always right aligned
  tft.setTextColor(fgColor, bgColor, true);
  tft.drawFloat(number, 1, xPos, yPos, 7); // last argument (7) indicates font # 7 which is "7-seg digits"
}

void Blackbody_GUI::initDisplayGraphics()
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


	drawSetPointRegion();
	drawConfigButton();

	drawLocked();

	drawStatus(blackbody.status);
}

void Blackbody_GUI::initTouchControl()
{
  bool debug = false;
	if (!touchController.begin(72, &Wire1)) // SCL = 16, SDA = 17
	{
		if (debug)
			Serial.println("Couldn't find touchController");
		while (1) delay(10);
	}
	if (debug)
		Serial.println("Found touchController controller");
}

void Blackbody_GUI::drawBootScreen()
{
	tft.pushImage(0, 0, BG_WIDTH, BG_HEIGHT, boot_background);
	tft.setTextDatum(MC_DATUM);
	tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
	tft.drawString("INITIALIZING", 160, 110, 4);
	tft.setTextDatum(MR_DATUM);
	tft.setTextPadding(tft.textWidth("111.1", 7));
	for (int seconds(20); seconds > 0; seconds--)
	{
		tft.drawNumber(seconds, 185, 170, 7);
		delay(1000);
	}
}

void Blackbody_GUI::drawKeypad()
{
	tft.fillScreen(TFT_WHITE);
	tft.setTextDatum(ML_DATUM);		// always right aligned
	tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
	tft.drawString("192", 5, 120, 7);
	tft.drawString("168", 110, 120, 7);
	tft.drawString("000", 220, 120, 7);
	tft.drawString("001", 290, 120, 7);
	// for(int i(0); i < 3; i++)
	// {
	// 	for(int j(0); j < 4; j++)
	// 	{
	// 		tft.drawRoundRect(4 + 54*i, 4 + j*54, 50, 50, 5, TFT_BLACK);
	// 	}
	// }
}

// private
bool Blackbody_GUI::isIncrButton(const unsigned x, const unsigned y)
{
	return (x >= 2385) && (x <= 3600) && (y <= 3485) && (y >= 2640);
}

bool Blackbody_GUI::isDecrButton(const unsigned x, const unsigned y)
{
  return (x >= 2385) && (x <= 3600) && (y <= 1360) && (y >= 460);
}

bool Blackbody_GUI::isSetpointNumber(const unsigned x, const unsigned y)
{
  return (x >= 2160) && (x <= 3715) && (y <= 2400) && (y >= 1650);
}

bool Blackbody_GUI::isUnlockRegion(const unsigned x, const unsigned y)
{
  return (x >= 2200) && (x <= 3715) && (y <= 3700) && (y >= 300);
}

bool Blackbody_GUI::isError(const unsigned x, const unsigned y)
{
  return (x >= 700) && (x <= 1800) && (y <= 2000) && (y >= 1436);
}

bool Blackbody_GUI::isConfig(const unsigned x, const unsigned y)
{
  return (x >= 700) && (x <= 1800) && (y <= 1200) && (y >= 700);
}