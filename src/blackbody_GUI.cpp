#include <TFT_eSPI.h> // graphics library

#include "blackbody_GUI.h"
#include "up_arrow_active.h"

bool Blackbody_GUI::isIncrButton(const unsigned x, const unsigned y)
{
  return (x >= 2385) && (x <= 3600) && (y <= 3485) && (y >= 2640);
}

bool Blackbody_GUI::isDecrButton(const unsigned x, const unsigned y)
{
  return (x >= 2385) && (x <= 3600) && (y <= 1360) && (y >= 460);
}

bool Blackbody_GUI::isUnluckNumber(const unsigned x, const unsigned y)
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


void Blackbody_GUI::drawStatus(TFT_eSPI& tft, const unsigned status)
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

void Blackbody_GUI::drawLocked(TFT_eSPI& tft, TFT_eSprite& upArrow, TFT_eSprite& downArrow)
{
  // change to locked color (GREY)
  tft.drawSmoothRoundRect(180, 25, 6, 5, 120, 60, TFT_SILVER);   // top rect
  tft.drawSmoothRoundRect(180, 165, 6, 5, 120, 60, TFT_SILVER);  // bottom rect
  upArrow.pushSprite(224, 39);  // top arrow
  downArrow.setPivot(224+32, 179+32);  
  downArrow.pushRotated(180); // bottom arrow
  this->targetPointColor = TFT_SILVER;
}

void Blackbody_GUI::drawUnlocked(TFT_eSPI& tft, TFT_eSprite& upArrow, TFT_eSprite& downArrow)
{
  // change to unlocked color (BLACK)
  tft.drawSmoothRoundRect(180, 25, 6, 5, 120, 60, TFT_BLACK);
  tft.drawSmoothRoundRect(180, 165, 6, 5, 120, 60, TFT_BLACK);
  upArrow.pushSprite(224, 39);  // top arrow
  downArrow.setPivot(224+32, 179+32);  
  downArrow.pushRotated(180); // bottom arrow
  this->targetPointColor = TFT_BLACK;
}

void Blackbody_GUI::drawSetPointRegion(TFT_eSPI& tft)
{
  tft.drawSmoothRoundRect(165, 8, 6, 5, 150, 227, TFT_BLACK);

	tft.setTextDatum(TL_DATUM);
	tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
  tft.setTextPadding(tft.textWidth("SET POINT", 4));
	tft.drawString("SET POINT", 180, 0, 4);
}

void Blackbody_GUI::drawConfigButton(TFT_eSPI& tft)
{
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_BLACK, TFT_WHITE, true);
  tft.drawString("CONFIG", 80, 190, 4);
  tft.drawSmoothRoundRect(30, 170, 6, 5, 100, 34, TFT_BLACK);
}
