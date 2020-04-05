/**
    modules/tide.c: tide module for openchronos-ng

    Copyright (C) 2012 Thomas Post <openchronos-ng@post-net.ch>

    http://github.com/BenjaminSoelberg/openchronos-ng-elf

    This file is part of openchronos-ng.

    openchronos-ng is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    openchronos-ng is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/

#include "messagebus.h"
#include "menu.h"
#include "tide.h"

/* drivers */
#include "drivers/rtca.h"
#include "drivers/display.h"
#include <time.h>
#include <stdbool.h>
#include <stdint.h>


#define DAYINMINUTES 1440
#define TRUE 1
#define FALSE 0

struct Tide
{
    uint8_t hour;  /* tide start hour */
    uint8_t min; /* tide start minute */
    int8_t height; /* height of this tide */
    uint8_t day; /* to account for day changes */
};

static uint16_t idxTide;

/* time to the next low tide */
static struct Tide thisTide;
static struct Tide nextTide;

/* calculations */
static uint8_t bRising = 0;
static int16_t i16currentHeight = 0; // (scale = value * 10 )
static uint16_t i16timeToNextTide = 1;
////////

uint8_t moduleActivated = 0;
uint8_t activeDisplay = 0;

/* MARK: Drawing */

void blinkCol(uint8_t screen, uint8_t line)
{
    switch (line) {
    case 1:
        display_symbol(screen, LCD_SEG_L1_COL, SEG_ON);
        display_symbol(screen, LCD_SEG_L1_COL, BLINK_ON);
        break;
    case 2:
        display_symbol(screen, LCD_SEG_L2_COL0, SEG_ON);
        display_symbol(screen, LCD_SEG_L2_COL0, BLINK_ON);
        break;
    default:
        break;
    }
}

void drawScreen(void)
{
    /* do nothing if not visible */
    if (!moduleActivated)
        return;

    display_clear(0, 0);
    display_clear(1, 0);
    display_clear(2, 0);

    /* screen 0 */
    /* line 1 = current height */
    /* line 2 = time to end tide */

    uint8_t u8HoursTonNextTide = (uint8_t) i16timeToNextTide / 60;
    uint8_t u8MinutesTonNextTide = (uint8_t) i16timeToNextTide % 60;
    //uint8_t u8CurrentHeight = i16currentHeight * 10;
    /* screen0 line1 current height */
    if (i16currentHeight > 0)
    {
        display_chars(0, LCD_SEG_L1_3_2, "  ", SEG_SET);
        _printf(0, LCD_SEG_L1_1_0, "%02d", i16currentHeight);
    }
    else
    {
        display_chars(0, LCD_SEG_L1_3_2, " -", SEG_SET);
        _printf(0, LCD_SEG_L1_1_0, "%02d", -i16currentHeight);
    }
    display_symbol(0, LCD_UNIT_L1_M, SEG_ON);
    display_symbol(0, LCD_SEG_L1_DP0, SEG_ON);

    /* screen0 line2 time */
    _printf(0, LCD_SEG_L2_3_2, "%02u", u8HoursTonNextTide);
    _printf(0, LCD_SEG_L2_1_0, "%02u", u8MinutesTonNextTide);
    display_symbol(0, LCD_SEG_L2_COL0, SEG_ON);
    display_symbol(0, LCD_ICON_STOPWATCH, SEG_ON);

    /* screen 1 */
    /* line 1 = this tide height */
    /* line 2 = this tide time */

    /* screen01 line1 this tide height */
    if (thisTide.height > 0)
    {
        display_chars(1, LCD_SEG_L1_3_2, "  ", SEG_SET);
        _printf(1, LCD_SEG_L1_1_0, "%02d", thisTide.height);
    }
    else
    {
        display_chars(1, LCD_SEG_L1_3_2, " -", SEG_SET);
        _printf(1, LCD_SEG_L1_1_0, "%02d", -thisTide.height);
    }
    display_symbol(1, LCD_UNIT_L1_M, SEG_ON);
    display_symbol(1, LCD_SEG_L1_DP0, SEG_ON);

    /* screen1 line2 this tide time */
    _printf(1, LCD_SEG_L2_3_2, "%02u", thisTide.hour);
    _printf(1, LCD_SEG_L2_1_0, "%02u", thisTide.min);
    display_symbol(1, LCD_SEG_L2_COL0, SEG_ON);

    /* screen 2 */
    /* line 1 = next tide height */
    /* line 2 = next tide time */

    /* screen2 line1 next tide height */
    if (nextTide.height > 0)
    {
        display_chars(2, LCD_SEG_L1_3_2, "  ", SEG_SET);
        _printf(2, LCD_SEG_L1_1_0, "%02d", nextTide.height);
    }
    else
    {
        display_chars(2, LCD_SEG_L1_3_2, " -", SEG_SET);
        _printf(2, LCD_SEG_L1_1_0, "%02d", -nextTide.height);
    }
    display_symbol(2, LCD_UNIT_L1_M, SEG_ON);
    display_symbol(2, LCD_SEG_L1_DP0, SEG_ON);

    /* screen2 line2 this tide time */
    _printf(2, LCD_SEG_L2_3_2, "%02u", nextTide.hour);
    _printf(2, LCD_SEG_L2_1_0, "%02u", nextTide.min);
    display_symbol(2, LCD_SEG_L2_COL0, SEG_ON);
    
    /* show rising symbol in all screns */
    if (bRising)
    {
        display_symbol(0, LCD_SYMB_ARROW_UP, SEG_ON);
        display_symbol(1, LCD_SYMB_ARROW_UP, SEG_ON);
        display_symbol(2, LCD_SYMB_ARROW_UP, SEG_ON);
    }
    else
    {
        display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_ON);
        display_symbol(1, LCD_SYMB_ARROW_DOWN, SEG_ON);
        display_symbol(2, LCD_SYMB_ARROW_DOWN, SEG_ON);
    }
}

/* MARK: System Bus Events */
void minuteTick()
{
    uint16_t u16timeNowInMinutes = (rtca_time.hour * 60) + rtca_time.min;
    uint16_t u16timeThisTideInMinutes = (thisTide.hour * 60)  + thisTide.min;
    uint16_t u16timeNextTideInMinutes = (nextTide.hour * 60)  + nextTide.min;

    i16timeToNextTide = u16timeNextTideInMinutes - u16timeNowInMinutes;

    // if the next tide is the next day, add 24*60 minutes to the next tide time
    if (rtca_time.day != nextTide.day)
    {
        u16timeNextTideInMinutes += DAYINMINUTES;
    }

    // check if changed tide and update
    if (u16timeNextTideInMinutes <= u16timeNowInMinutes)
    {
        idxTide += 3;

        thisTide.hour = nextTide.hour;
        thisTide.min = nextTide.min;
        thisTide.height = nextTide.height;
        thisTide.day = nextTide.day;

        // if the next tide is in the next day, jumps the day mark
        if (tides[idxTide] > 80)
        {
            nextTide.day = tides[idxTide+1];

            idxTide += 3;
        }
        nextTide.hour = tides[idxTide];
        nextTide.min = tides[idxTide+1];
        nextTide.height = tides[idxTide+2];
    }

    // calculate current height
    float fCoefficient = (float) (nextTide.height - thisTide.height) / (u16timeNextTideInMinutes - u16timeThisTideInMinutes);
    i16currentHeight = (int16_t) (fCoefficient * (u16timeNowInMinutes - u16timeThisTideInMinutes) + (float) thisTide.height);
    // set flag of rising or not
    if ((nextTide.height - thisTide.height) >= 0)
    {
        bRising = TRUE;
    }
    else
    {
        bRising = FALSE;
    }

    /* draw screens */
    drawScreen();
}

void buttonUp(void)
{
    lcd_screen_activate(0xff);
    drawScreen();
}

void buttonDown(void)
{
    if (activeDisplay <= 0)
        activeDisplay = 2;
    else
        activeDisplay = (activeDisplay - 1) % 3;

    lcd_screen_activate(activeDisplay);
    drawScreen();
}

/* MARK: - Activate and Deactivate */
void activate(void)
{
    moduleActivated = 1;
    /* create three empty screens */
    lcd_screens_create(3);

    activeDisplay = 0;
    lcd_screen_activate(activeDisplay);
    drawScreen();
}

void deactivate(void)
{
    moduleActivated = 0;
    /* destroy virtual screens */
    lcd_screens_destroy();

    display_clear(0, 0);
}

void mod_tide_init(void)
{
    sys_messagebus_register(&minuteTick, SYS_MSG_RTC_MINUTE);
    menu_add_entry("TIDE",
                   &buttonUp,
                   &buttonDown,
                   NULL,
                   NULL,
                   NULL,
                   NULL,
                   &activate,
                   &deactivate);

    // find initial tide:
    uint16_t i = 0;
    uint16_t szTides = sizeof(tides) / sizeof(tides[0]);
    uint8_t bFoundTide = 0;
    uint8_t u8day = 0;
    while (bFoundTide == 0)
    {
        // found a day mark
        if (tides[i] > 80)
        {
            // if the day mark is equal to current day
            if ((tides[i] - 80) == rtca_time.mon && tides[i+1] == rtca_time.day)
            {
                u8day = rtca_time.day;
                // find thistide/nexttide to initialize
                i += 3;
                // initialize with first tide
                idxTide = i;
                thisTide.hour = tides[i];
                thisTide.min = tides[i+1];
                thisTide.height = tides[i+2];
                thisTide.day = u8day;

                // we can assume the next is in the same day, since theres always 2 tides in a day
                nextTide.hour = tides[i+3];
                nextTide.min = tides[i+4];
                nextTide.height = tides[i+5];
                nextTide.day = u8day;

                uint16_t timeNowInMinutes = (rtca_time.hour * 60) + rtca_time.min;
                uint16_t timeTideInMinutes = 0;
                while (i < szTides)
                {
                    if (tides[i] < 80)
                    {
                        timeTideInMinutes = (tides[i] * 60)  + tides[i+1] + (u8day - rtca_time.day) * DAYINMINUTES;
                        if (timeTideInMinutes <= timeNowInMinutes)
                        {
                            // the tide can be in this state
                            thisTide.hour = tides[i];
                            thisTide.min = tides[i+1];
                            thisTide.height = tides[i+2];
                            thisTide.day = u8day;
                            idxTide = i;
                        }
                        else
                        {
                            // we passed, so this is the next tide
                            // must account if it is the next day (the watch initilizes close to midnight)
                            nextTide.hour = tides[i];
                            nextTide.min = tides[i+1];
                            nextTide.height = tides[i+2];
                            nextTide.day = u8day;
                            bFoundTide = 1;
                            break;
                        }
                    }
                    else
                    {
                        u8day = tides[i+1];
                    }
                    i += 3;
                }
                break;
            }
        }
        i += 3;
    }
}
