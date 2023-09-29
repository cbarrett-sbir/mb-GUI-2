#include <TFT_eSPI.h> // graphics
#include <WireIMXRT.h>
#include <cstring>

#include "blackbody_GUI.h"

#include "Free_Fonts.h"
#include "NotoSansBold36.h"
#include "boot_background.h"
#include "images.h"

#define AA_FONT_LARGE NotoSansBold36

// public
// home page 
ButtonState Blackbody_GUI::parseHomePageTouch(const unsigned x, const unsigned y)
{
	if (isIncrButton(x, y) && !locked)
	{
		return incrPressed;
	}
	else if (isDecrButton(x, y) && !locked)
	{
		return decrPressed;
	}
	else if (isSetpointNumber(x, y))
	{
		return setpointNumberPressed;
	}
	else if (isUnlockRegion(x, y) && locked)
	{
		return unlockRegionPressed;
	}
	else if (isError(x, y) && blackbody.status == 32)
	{
		tft.fillScreen(TFT_WHITE); // splash screen
		tft.setTextDatum(MC_DATUM);
		tft.setTextColor(TFT_RED, TFT_WHITE, true);
		tft.drawString("ERROR PAGE", 160, 120, 4);
		delay(5000);
		initDisplayGraphics();
	}
	else if (isConfig(x, y))
	{
		return configPressed;
	}

	// if nothing caught
	return nonePressed;
}

void Blackbody_GUI::updateHomePageState(const ButtonState buttonState, const ButtonState prevButtonState)
{
	bool debug = false;
	if (buttonState == nonePressed && prevButtonState != nonePressed)
	{
		// used to lock setpoint region after 10s
		timeLastActivity = millis();
	}

	// Update state
	switch (buttonState)
	{
	case incrPressed:
	{
		if (incrHeld) // flag that indicates if Incr button has been held > 1s
		{			  // & controls auto-increment fucntionality
			if (debug)
				Serial.println("incrHeld");

			autoRate = 50; // 1/ ( rate[ms] ) = rate of incrementation. rate should be renamed period.
			if (prevButtonState != incrPressed)
			{
				incrHeld = false;
				break;
			}
			if (millis() - timeIncrFirstPressed >= 5000)
			{
				autoRate = 7.5;
			} // after 5s, speed up incrementation

			if (millis() - timeIncrLastPressed >= autoRate)
			{
				targetPoint += 0.1;
				timeIncrLastPressed = millis();
			}
			break;
		}

		if (prevButtonState == nonePressed) // initial press
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
		if (decrHeld) // code is symmetric to increment
		{
			if (debug)
				Serial.println("decrHeld");
			autoRate = 50;
			if (prevButtonState != decrPressed)
			{
				decrHeld = false;
				break;
			}
			if (millis() - timeDecrFirstPressed >= 5000)
			{
				autoRate = 7.5;
			}
			if (millis() - timeDecrLastPressed >= autoRate)
			{
				targetPoint -= 0.1;
				timeDecrLastPressed = millis();
			}
			break;
		}

		if (prevButtonState == nonePressed) // initial press
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
		if (prevButtonState != nonePressed)
		{
			break;
		} // ensure only initial press caught

		if (locked)
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

	case configPressed:
	{
		if (prevButtonState == nonePressed)
		{
			drawConfigScreen();
			currPage = configPage;
			delay(150);
		}
	}

	default:			  // runs if currState = nonePressed (or some other uncaught state)
		decrHeld = false; // stops auto-incr/decr
		incrHeld = false;
		incrEnable = false;
		decrEnable = false;

		if (!locked)
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

void Blackbody_GUI::drawSetPointRegion()
{
	tft.drawSmoothRoundRect(165, 8, 6, 5, 150, 227, TFT_BLACK);

	tft.setTextDatum(TL_DATUM);
	tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
	tft.setTextPadding(tft.textWidth("SETPOINT", 4));
	tft.drawString("SETPOINT", 182, 0, 4);
}

// config page
ButtonState Blackbody_GUI::parseConfigPageTouch(const unsigned x, const unsigned y)
{
	if ((x >= 1570) && (x <= 2830) && (y <= 3312) && (y >= 2730))
	{
		return ipModePressed;
	}
	else if ((x >= 400) && (x <= 2930) && (y <= 2170) && (y >= 1522))
	{
		return addressPressed;
	}
	else if ((x >= 400) && (x <= 1770) && (y <= 1000) && (y >= 400))
	{
		return readyWindowPressed;
	}
	else if ((x >= 300) && (x <= 800) && (y <= 3700) && (y >= 2950))
	{
		return backPressed;
	}
	else if ((x >= 330) && (x <= 1660) && (y <= 350) && (y >= 960))
	{
		return readyWindowPressed;
	}

	return nonePressed;
}

void Blackbody_GUI::updateConfigPageState(ButtonState buttonState, ButtonState prevButtonState)
{
	switch (buttonState)
	{
	case ipModePressed:
	{ // catch only first press
		if (prevButtonState != nonePressed)
		{
			break;
		}
		blackbody.ipMode= !blackbody.ipMode;

		drawIPMode(blackbody.ipMode, 130, 60);
		if (blackbody.ipMode)
		{
			drawAddress(5, 90, TFT_LIGHTGREY);
		}
		else
		{
			drawAddress(5, 90, TFT_BLACK);
		}
		break;
	}

	case backPressed:
	{
		if (prevButtonState != nonePressed)
		{
			break;
		}
		tft.fillScreen(TFT_WHITE);

		drawSetPointRegion();
		drawConfigButton();

		if (locked)
		{
			drawLocked();
		}
		else
		{
			drawUnlocked();
		}

		drawStatus(blackbody.status);
		currPage = homePage;
		break;
	}

	case addressPressed:
	{
		if (blackbody.ipMode== 1)
		{
			break;
		} // 1 = DHCP, used to ignore presses in DHCP

		if (prevButtonState != nonePressed)
		{
			break;
		}

		tft.fillScreen(TFT_WHITE);
		tft.setTextDatum(ML_DATUM);
		tft.loadFont(AA_FONT_LARGE);
		const unsigned middle_left = 160 - (tft.textWidth("000.111.222.333") / 2);

		tft.drawString(blackbody.address.getAddress(), middle_left, 120);
		// draw byte selection indicator
		currByteSelected = 3;
		tft.drawWideLine(middle_left + tft.textWidth("000.111.222."), 140, middle_left + tft.textWidth("000.111.222.333"), 140, 4, TFT_BLACK, TFT_WHITE);
		tft.unloadFont();

		// incr/decr buttons
		upArrowActive.pushSprite(185+44, 15+14);
		downArrow.pushSprite(185+44, 240-(32+14+15));
		tft.drawSmoothRoundRect(185, 15, 6, 5, 120, 60, TFT_BLACK);
		tft.drawSmoothRoundRect(185, 165, 6, 5, 120, 60, TFT_BLACK);

		// back buttom
		backArrow.pushSprite(24, 24);
		tft.drawSmoothRoundRect(15, 15, 6, 5, 50, 50, TFT_BLACK);

		// save button
		checkmark.pushSprite(89, 24);
		tft.drawSmoothRoundRect(80, 15, 6, 5, 50, 50, TFT_BLACK);
		
		prospectiveIpAddress = blackbody.address;
		currPage = addressAdjustPage;
		break;
	}
	
	case readyWindowPressed:
	{
		tft.fillScreen(TFT_WHITE);
		tft.setTextDatum(ML_DATUM);

		tft.setTextColor(TFT_BLACK);
		tft.drawFloat(blackbody.readyWindow, 1, 60, 120, 7);

		// incr/decr buttons
		upArrowActive.pushSprite(185+44, 15+14);
		tft.setPivot(185+44+32, 165+14);
		downArrowActive.pushRotated(180);
		tft.drawSmoothRoundRect(185, 15, 6, 5, 120, 60, TFT_BLACK);
		tft.drawSmoothRoundRect(185, 165, 6, 5, 120, 60, TFT_BLACK);

		// back buttom
		backArrow.pushSprite(24, 24);
		tft.drawSmoothRoundRect(15, 15, 6, 5, 50, 50, TFT_BLACK);

		// save button
		checkmark.pushSprite(89, 24);
		tft.drawSmoothRoundRect(80, 15, 6, 5, 50, 50, TFT_BLACK);
		
		prospectiveReadyWindow = blackbody.readyWindow;
		currPage = windowAdjustPage;
	}

	default:
	{
		// DHCP or STATIC
		if (prevIpMode != blackbody.ipMode)
		{
			drawIPMode(blackbody.ipMode, 130, 60);
			prevIpMode = blackbody.ipMode;
		}

		// IP address
		if (prevIpAddress != blackbody.address)
		{
			if (blackbody.ipMode)
			{
				drawAddress(5, 90, TFT_LIGHTGREY);
			}
			else
			{
				drawAddress(5, 90, TFT_BLACK);
			}
			prevIpAddress = blackbody.address;
		}

		// ready window
		if ( !is_equal(prevReadyWindow, blackbody.readyWindow, 0.01) )
		{
			drawReadyWindow(5, 165);
			prevReadyWindow = blackbody.readyWindow;
		}
	}
	}
}

void Blackbody_GUI::drawConfigScreen()
{
	#define ip_mode_height 60
	#define address_height 90
	#define rdy_win_height 165
	tft.fillScreen(TFT_WHITE);

	tft.setTextDatum(TC_DATUM);
	tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
	tft.setFreeFont(FSSB12);
	tft.drawString("CONFIG", 160, 10, GFXFF);

	tft.setTextDatum(TL_DATUM);
	tft.drawString("IP Mode", 5, ip_mode_height, GFXFF);
	drawIPMode(blackbody.ipMode, 130, ip_mode_height);

	tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
	tft.drawString("Address", 5, address_height, GFXFF);
	if (blackbody.ipMode)
	{
		drawAddress(5, 90, TFT_LIGHTGREY);
	}
	else
	{
		drawAddress(5, 90, TFT_BLACK);
	}

	tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
	tft.drawString("Ready Window", 5, rdy_win_height, GFXFF);
	drawReadyWindow(5, rdy_win_height);

	// back buttom
	smallBackArrow.pushSprite(10+8, 10+8);
	tft.drawSmoothRoundRect(10, 10, 6, 5, 40, 40, TFT_BLACK);
}

// address page
ButtonState Blackbody_GUI::parseAddressPageTouch(const unsigned x, const unsigned y)
{
	if ((x >= 2370) && (x <= 3722) && (y <= 3600) && (y >= 2760))
	{
		return incrPressed;
	}
	else if ((x >= 2370) && (x <= 3722) && (y <= 1350) && (y >= 485))
	{
		return decrPressed;
	}
	else if ((x >= 450) && (x <= 1250) && (y <= 2400) && (y >= 1600))
	{
		return byteZeroPressed;
	}
	else if ((x >= 1290) && (x <= 2100) && (y <= 2400) && (y >= 1600))
	{
		return byteOnePressed;
	}
	else if ((x >= 2130) && (x <= 2900) && (y <= 2400) && (y >= 1600))
	{
		return byteTwoPressed;
	}
	else if ((x >= 2930) && (x <= 3700) && (y <= 2400) && (y >= 1600))
	{
		return byteThreePressed;
	}
	else if ((x >= 430) && (x <= 1000) && (y <= 3550) && (y >= 2750))
	{
		return backPressed;
	}
	else if ((x >= 1170) && (x <= 1760) && (y <= 3550) && (y >= 2570))
	{
		return savePressed;
	}

	return nonePressed;
}

void Blackbody_GUI::updateAddressAdjustPageState(ButtonState buttonState, ButtonState prevButtonState)
{
	tft.loadFont(AA_FONT_LARGE);
	const unsigned middle_left = 160 - (tft.textWidth("000.111.222.333") / 2);
	switch (buttonState)
	{
	case incrPressed:
	{
		if (incrHeld)	   // flag that indicates if Incr button has been held > 1s
		{				   // & controls auto-increment functionality
			autoRate = 50; // 1/ ( rate[ms] ) = rate of incrementation. rate should be renamed period.
			if (prevButtonState != incrPressed)
			{
				incrHeld = false;
				break;
			}
			if (millis() - timeIncrFirstPressed >= 1000)
			{
				autoRate = 7.5;
			} // after 1s, speed up incrementation

			if (millis() - timeIncrLastPressed >= autoRate)
			{
				prospectiveIpAddress.incrByte(currByteSelected);
				timeIncrLastPressed = millis();
			}
			break;
		}

		if (prevButtonState == nonePressed) // initial press
		{
			incrEnable = true;
			prospectiveIpAddress.incrByte(currByteSelected);
			timeIncrLastPressed = millis();
			timeIncrFirstPressed = timeIncrLastPressed;
		}

		// button held > 1s
		else if (incrEnable && (prevButtonState == incrPressed) && (millis() - timeIncrLastPressed > 1000))
		{
			prospectiveIpAddress.incrByte(currByteSelected);
			incrHeld = true;
			timeIncrLastPressed = millis();
			timeIncrFirstPressed = timeIncrLastPressed;
		}
		break;
	}

	case decrPressed:
	{
		if (decrHeld) // code is symmetric to increment
		{
			autoRate = 50;
			if (prevButtonState != decrPressed)
			{
				decrHeld = false;
				break;
			}
			if (millis() - timeDecrFirstPressed >= 1000)
			{
				autoRate = 7.5;
			}
			if (millis() - timeDecrLastPressed >= autoRate)
			{
				prospectiveIpAddress.decrByte(currByteSelected);
				timeDecrLastPressed = millis();
			}
			break;
		}

		if (prevButtonState == nonePressed) // initial press
		{
			decrEnable = true;
			prospectiveIpAddress.decrByte(currByteSelected);
			timeDecrLastPressed = millis();
			timeDecrFirstPressed = timeDecrLastPressed;
		}
		// button held
		else if (decrEnable && (prevButtonState == decrPressed) && (millis() - timeDecrLastPressed > 1000))
		{
			prospectiveIpAddress.decrByte(currByteSelected);
			decrHeld = true;
			timeDecrLastPressed = millis();
			timeDecrFirstPressed = timeDecrLastPressed;
		}
		break;
	}
	
	case byteZeroPressed:
	{
		if (prevButtonState != nonePressed)
		{
			break;
		}
		if (currByteSelected != 0)
		{
			currByteSelected = 0;
			tft.fillRect(10, 138, 300, 5, TFT_WHITE);
			tft.drawWideLine(middle_left, 140, middle_left + tft.textWidth("000"), 140, 4, TFT_BLACK, TFT_WHITE);
		}
		break;
	}

	case byteOnePressed:
	{
		if (prevButtonState != nonePressed)
		{
			break;
		}
		if (currByteSelected != 1)
		{
			currByteSelected = 1;
			tft.fillRect(10, 138, 300, 5, TFT_WHITE);
			tft.drawWideLine(middle_left + tft.textWidth("000."), 140, middle_left + tft.textWidth("000.000"), 140, 4, TFT_BLACK, TFT_WHITE);
		}
		break;
	}

	case byteTwoPressed:
	{
		if (prevButtonState != nonePressed)
		{
			break;
		}
		if (currByteSelected != 2)
		{
			currByteSelected = 2;
			tft.fillRect(10, 138, 300, 5, TFT_WHITE);
			tft.drawWideLine(middle_left + tft.textWidth("000.000."), 140, middle_left + tft.textWidth("000.000.000"), 140, 4, TFT_BLACK, TFT_WHITE);
		}
		break;
	}

	case byteThreePressed:
	{
		if (prevButtonState != nonePressed)
		{
			break;
		}
		if (currByteSelected != 3)
		{
			currByteSelected = 3;
			tft.fillRect(10, 138, 300, 5, TFT_WHITE);
			tft.drawWideLine(middle_left + tft.textWidth("000.000.000."), 140, middle_left + tft.textWidth("000.000.000.000"), 140, 4, TFT_BLACK, TFT_WHITE);
		}
		break;
	}

	case backPressed:
	{
		if (prevButtonState == nonePressed)
		{
			tft.unloadFont();
			drawConfigScreen();
			currPage = configPage;
		}
	}
	break;

	case savePressed:
	{
		if (prevButtonState != nonePressed)
		{
			break;
		}
		Serial.print("ADDR ");
		Serial.println(prospectiveIpAddress.getAddress());
		break;
	}

	default:			  // runs if currState = nonePressed (or some other uncaught state)
		decrHeld = false; // stops auto-incr/decr
		incrHeld = false;
		incrEnable = false;
		decrEnable = false;
		break;
	}
	if (currPage == addressAdjustPage)
	{
		if (prospectiveIpAddress == blackbody.address)
		{
			tft.setTextColor(TFT_BLACK, TFT_WHITE);
			tft.drawString(blackbody.address.getAddress(), middle_left, 120);
		}
		else
		{
			tft.setTextColor(TFT_LIGHTGREY, TFT_WHITE);
			tft.drawString(prospectiveIpAddress.getAddress(), middle_left, 120);
		}
	}
}

// ready window page
ButtonState Blackbody_GUI::parseWindowPageTouch(const unsigned x, const unsigned y)
{
	if ((x >= 2370) && (x <= 3722) && (y <= 3600) && (y >= 2760))
	{
		return incrPressed;
	}
	else if ((x >= 2370) && (x <= 3722) && (y <= 1350) && (y >= 485))
	{
		return decrPressed;
	}
	else if ((x >= 430) && (x <= 1000) && (y <= 3550) && (y >= 2750))
	{
		return backPressed;
	}
	else if ((x >= 1170) && (x <= 1760) && (y <= 3550) && (y >= 2570))
	{
		return savePressed;
	}

	return nonePressed;
}

void Blackbody_GUI::updateWindowAdjustPageState(ButtonState buttonState, ButtonState prevButtonState)
{
	switch (buttonState)
	{
	case incrPressed:
	{
		if (incrHeld)	   // flag that indicates if Incr button has been held > 1s
		{				   // & controls auto-increment functionality
			autoRate = 50; // 1/ ( rate[ms] ) = rate of incrementation. rate should be renamed period.
			if (prevButtonState != incrPressed)
			{
				incrHeld = false;
				break;
			}
			if (millis() - timeIncrFirstPressed >= 1000)
			{
				autoRate = 7.5;
			} // after 1s, speed up incrementation

			if (millis() - timeIncrLastPressed >= autoRate)
			{
				if (prospectiveReadyWindow < 9.9)
				{
					prospectiveReadyWindow += 0.1;
				}
				timeIncrLastPressed = millis();
			}
			break;
		}

		if (prevButtonState == nonePressed) // initial press
		{
			incrEnable = true;
			if (prospectiveReadyWindow < 9.9)
			{
				prospectiveReadyWindow += 0.1;
			}
			timeIncrLastPressed = millis();
			timeIncrFirstPressed = timeIncrLastPressed;
		}

		// button held > 1s
		else if (incrEnable && (prevButtonState == incrPressed) && (millis() - timeIncrLastPressed > 1000))
		{
			if (prospectiveReadyWindow < 9.9)
			{
				prospectiveReadyWindow += 0.1;
			}
			incrHeld = true;
			timeIncrLastPressed = millis();
			timeIncrFirstPressed = timeIncrLastPressed;
		}
		break;
	}

	case decrPressed:
	{
		if (decrHeld) // code is symmetric to increment
		{
			autoRate = 50;
			if (prevButtonState != decrPressed)
			{
				decrHeld = false;
				break;
			}
			if (millis() - timeDecrFirstPressed >= 1000)
			{
				autoRate = 7.5;
			}
			if (millis() - timeDecrLastPressed >= autoRate)
			{
				if (prospectiveReadyWindow > 0.1)
				{
					prospectiveReadyWindow -= 0.1;
				}
				timeDecrLastPressed = millis();
			}
			break;
		}

		if (prevButtonState == nonePressed) // initial press
		{
			decrEnable = true;
			if (prospectiveReadyWindow > 0.1)
			{
				prospectiveReadyWindow -= 0.1;
			}
			timeDecrLastPressed = millis();
			timeDecrFirstPressed = timeDecrLastPressed;
		}
		// button held
		else if (decrEnable && (prevButtonState == decrPressed) && (millis() - timeDecrLastPressed > 1000))
		{
			if (prospectiveReadyWindow > 0.1)
			{
				prospectiveReadyWindow -= 0.1;
			}
			decrHeld = true;
			timeDecrLastPressed = millis();
			timeDecrFirstPressed = timeDecrLastPressed;
		}
		break;
	}
	
	case backPressed:
	{
		if (prevButtonState == nonePressed)
		{
			Serial.print("L");
			Serial.println(prospectiveReadyWindow);
			tft.unloadFont();
			drawConfigScreen();
			currPage = configPage;
		}
	}

	case savePressed:
	{
		if (prevButtonState != nonePressed)
		{
			break;
		}
		Serial.print("L ");
		Serial.println(prospectiveReadyWindow);
	}

	default:
	{
		if (currPage == windowAdjustPage)
		{
			if (is_equal(prospectiveReadyWindow, blackbody.readyWindow, 0.01))
			{
				tft.setTextColor(TFT_BLACK, TFT_WHITE);
				tft.drawFloat(prospectiveReadyWindow, 1, 60, 120, 7);
			}
			else
			{
				tft.setTextColor(TFT_LIGHTGREY, TFT_WHITE);
				tft.drawFloat(prospectiveReadyWindow, 1, 60, 120, 7);
			}
			
		}
		break;
	}
	}
}

void Blackbody_GUI::drawStatus(const unsigned status)
{
	unsigned xPos = 80;
	unsigned yPos = 140;

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
	tft.drawSmoothRoundRect(180, 25, 6, 5, 120, 60, TFT_SILVER);  // top rect
	tft.drawSmoothRoundRect(180, 165, 6, 5, 120, 60, TFT_SILVER); // bottom rect
	upArrowInactive.pushSprite(224, 39);						  // top arrow
	tft.setPivot(0,0);
	downArrowInactive.setPivot(224 + 32, 179 + 32);
	downArrowInactive.pushRotated(180); // bottom arrow
	targetPointColor = TFT_SILVER;
}

void Blackbody_GUI::drawUnlocked()
{
	// change to unlocked color (BLACK)
	tft.drawSmoothRoundRect(180, 25, 6, 5, 120, 60, TFT_BLACK);
	tft.drawSmoothRoundRect(180, 165, 6, 5, 120, 60, TFT_BLACK);
	tft.setPivot(0,0);
	upArrowActive.pushSprite(224, 39); // top arrow
	downArrowActive.setPivot(224 + 32, 179 + 32);
	downArrowActive.pushRotated(180); // bottom arrow
	targetPointColor = TFT_BLACK;
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
	tft.setTextDatum(MR_DATUM); // always right aligned
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

	backArrow.createSprite(32, 32);
	backArrow.setSwapBytes(true); // change this if colors are weird
	backArrow.pushImage(0, 0, 32, 32, back_arrow);

	smallBackArrow.createSprite(24, 24);
	smallBackArrow.setSwapBytes(true); // change this if colors are weird
	smallBackArrow.pushImage(0, 0, 24, 24, _back_arrow_small);

	checkmark.createSprite(32, 32);
	checkmark.setSwapBytes(true); // change this if colors are weird
	checkmark.pushImage(0, 0, 32, 32, _checkmark);

	downArrow.createSprite(32, 32);
	downArrow.setSwapBytes(true); // change this if colors are weird
	downArrow.pushImage(0, 0, 32, 32, _down_arrow);

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
		while (1)
			delay(10);
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
	return (x >= 700) && (x <= 1800) && (y <= 1300) && (y >= 830);
}

void Blackbody_GUI::drawIPMode(const bool mode, const unsigned x, const unsigned y)
{
	if (mode) // 1 = DCHP, 0 =  STATIC
	{
		tft.setTextDatum(TL_DATUM);
		tft.setTextColor(TFT_GREEN, TFT_WHITE, true);
		tft.fillRect(x - 10, y, 10, 20, TFT_WHITE);							  // clear 'S' from "STATIC"
		tft.fillRect(x + tft.textWidth("DHCP", GFXFF), y, 10, 20, TFT_WHITE); // clear 'C' from "STATIC"
		tft.setTextPadding(tft.textWidth("DHCP", GFXFF));
		tft.drawString("DHCP", x, y, GFXFF);
		tft.drawRoundRect(x - 20, y - 5, 110, 30, 5, TFT_GREEN);
		return;
	}
	else
	{
		tft.setTextDatum(TL_DATUM);
		tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
		tft.setTextPadding(tft.textWidth("STATIC", GFXFF));
		tft.drawString("STATIC", x - 10, y, GFXFF);
		tft.drawRoundRect(x - 20, y - 5, 110, 30, 5, TFT_BLACK);
	}
}

void Blackbody_GUI::drawAddress(const unsigned x, const unsigned y, const unsigned color)
{
	tft.setTextDatum(TL_DATUM);
	tft.setTextColor(color, TFT_WHITE, true);
	tft.setTextPadding(tft.textWidth("192.168.100.001", GFXFF));
	tft.drawString(blackbody.address.getAddress(), x + 15, y + 35, GFXFF);
	tft.drawRoundRect(x, y + 25, 220, 40, 5, color);
}

void Blackbody_GUI::drawReadyWindow(const unsigned x, const unsigned y)
{
	tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
	tft.drawString("+-", x + 15, y + 35, GFXFF);
	tft.drawFloat(blackbody.readyWindow, 1, x+40, y + 35, GFXFF);
	tft.drawString("C", x + 85, y + 35, GFXFF);
	tft.drawCircle(x+80, y+39, 3, TFT_BLACK);
	tft.drawRoundRect(x, y + 25, 120, 40, 5, TFT_BLACK);
}

bool Blackbody_GUI::is_equal(float f1, float f2, float epsilon) 
{
        if(abs(f1-f2)<epsilon)
                return true;
        return false;
}
