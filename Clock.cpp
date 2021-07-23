/*
 Clock.cpp
 
 Created on: Mar 24, 2015
     Author: Zoltan Hudak
     
 This is a software implemented Real Time Clock driven by a Ticker.
 No external parts (like DS1307 or DS3231 or etc.) are needed.
 
 See demo <http://developer.mbed.org/users/hudakz/code/Clock_Hello/>
 
 Copyright (c) 2015 Zoltan Hudak <hudakz@outlook.com>
 All rights reserved.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "mbed.h"
#include "Clock.h"

// Static member initialization
time_t              Clock::_time = 0;
tm                  Clock::_tm = { 0, 0, 0, 0, 0, 0, 0, 0, -1 };
Callback<void ()>   Clock::_onTick = NULL;

/**
 * @brief   Constructs a Clock.
 * @note    The clock is driven by a Ticker.
 *          Since the Clock is attached as an external RTC
 *          standard C time functions can be called as well.
 * @param   year:   long format (for instance 2015)
 * @param   mon:    month (1 stands for January etc.)
 * @param   mday:   day of month
 * @param   hour:   24hour format
 * @param   min:    minutes
 * @param   ser:    seconds
 * @retval
 */
Clock::Clock(int year, int mon, int mday, int hour, int min, int sec) :
    _ticker(new Ticker)
{
    _ticker->attach_us(&tick, 1000000); // a Ticker ticking at 1s rate
    attach_rtc(time, NULL, NULL, NULL); // attach for C time functions
}

/**
 * @brief   Constructs a Clock.
 * @note    The clock is driven by a Ticker.
 *          Since the Clock is attached as an external RTC
 *          standard C time functions can be called as well.
 *          Time is set to the begin of Epoch: 00:00:00 January 1, 1970
 * @param
 * @retval
 */
Clock::Clock() :
    _ticker(new Ticker)
{
    _ticker->attach_us(&tick, 1000000); // a Ticker ticking at 1s rate
    attach_rtc(time, NULL, NULL, NULL); // attach for C time functions
}

/**
 * @brief   Destroys a Clock.
 * @note    Destructor
 * @param
 * @retval
 */
Clock::~Clock()
{
    _ticker->detach();  // suspend ticks
    delete _ticker;     // distroys _ticker
}

/**
 * @brief   Sets Clock time using human readable inputs
 * @note
 * @param   year:   long format (for instance 2015)
 * @param   mon:    month (1 stands for January etc.)
 * @param   mday:   day of month
 * @param   hour:   24hour format
 * @param   min:    minutes
 * @param   ser:    seconds
 * @retval
 */
void Clock::set(int year, int mon, int mday, int hour, int min, int sec)
{
    _ticker->detach();                  // suspend ticks
    _tm.tm_year = year - 1900;
    _tm.tm_mon = mon - 1;               // convert to 0 based month
    _tm.tm_mday = mday;
    _tm.tm_hour = hour;
    _tm.tm_min = min;
    _tm.tm_sec = sec;
    _tm.tm_isdst = -1;

    // Is DST on? 1 = yes, 0 = no, -1 = unknown
    _time = mktime(&_tm);               // convert to seconds elapsed since January 1, 1970
    set_time(_time);                    // set time
    _ticker->attach_us(&tick, 1000000); // resume ticks
}

/**
 * @brief   Sets Clock to tm value
 * @note
 * @param   val:    tm structure
 * @retval
 */
void Clock::set(tm& val)
{
    _ticker->detach();                  // suspend ticks
    _tm = val;
    _ticker->attach_us(&tick, 1000000); // resume ticks
}

/**
 * @brief   Sets Clock to time_t value
 * @note
 * @param   val:    time_t structure (Number of seconds elapsed since January 1, 1970)
 * @retval
 */
void Clock::set(time_t val)
{
    _ticker->detach();                  // suspend ticks
    _time = val;
    _tm = *::localtime(&_time);
    _ticker->attach_us(&tick, 1000000); // resume ticks
}

/**
 * @brief   Current time
 * @note
 * @param
 * @retval  Number of seconds elapsed since January 1, 1970
 */
time_t Clock::time(void)
{
    return _time;
}

/**
 * @brief   Year
 * @note
 * @param
 * @retval  Year
 */
int Clock::year(void)
{
    return(_tm.tm_year + 1900);
}

/**
 * @brief   Month
 * @note
 * @param
 * @retval  Month
 */
int Clock::mon(void)
{
    return(_tm.tm_mon + 1);
}

/**
 * @brief   Day of month
 * @note
 * @param
 * @retval  Day of the month
 */
int Clock::mday(void)
{
    return _tm.tm_mday;
}

/**
 * @brief   Day of week
 * @note    
 * @param
 * @retval  Day of week (Sunday = 0, Monday = 1, etc.)
 */
int Clock::wday(void)
{
    return _tm.tm_wday; // Sunday = 0, Monday = 1, etc.
}

/**
 * @brief   Hour
 * @note
 * @param
 * @retval  Hour
 */
int Clock::hour(void)
{
    return _tm.tm_hour;
}

/**
 * @brief   Minutes
 * @note
 * @param
 * @retval  Minutes
 */
int Clock::min(void)
{
    return _tm.tm_min;
}

/**
 * @brief   Seconds
 * @note
 * @param
 * @retval  Seconds
 */
int Clock::sec(void)
{
    return _tm.tm_sec;
}

/**
 * @brief   Clock tick handler
 * @note    Called by the _ticker once per second
 *          Calls event handler if attached one by the user
 * @param
 * @retval
 */
void Clock::tick(void)
{
    _tm = *::localtime(&(++_time));
    _onTick.call();
}

/**
 * @brief   Attaches a handler to the onTick event
 * @note    onTick event occurs each second
 * @param   fnc  User defined handler function of type 'void fnc(void)'
 * @retval
 */
void Clock::attach(void (*fptr) (void))
{
    if (fptr)
        _onTick = fptr;
}

/**
 * @brief   Attaches a class method as handler to the onTick event
 * @note    onTick event occurs each second
 * @param   tptr  Pointer a class
 * @param   mptr  Poiter to a 'void fnc(void)' method
 * @retval
 */
template<typename T>
void Clock::attach(T* tptr, void (T::*mptr) (void))
{
    if ((tptr != NULL) && (mptr != NULL))
        _onTick.attach(tptr, mptr);
}

/**
 * @brief   Detaches handler function from the onTick event
 * @note
 * @param
 * @retval
 */
void Clock::detach()
{
    _onTick = NULL;
}

/**
 * @brief   Converts human readable time to seconds elapsed since January 1, 1970
 * @note    A static helper function.
 * @param   year:   long format (for instance 2015)
 * @param   mon:    month (1 stands for January etc.)
 * @param   mday:   day of month
 * @param   hour:   24hour format
 * @param   min:    minutes
 * @param   ser:    seconds
 * @retval  Number of seconds elapsed since January 1, 1970
 */
time_t Clock::asTime(int year, int mon, int mday, int hour, int min, int sec)
{
    struct tm   t;
    t.tm_year = year - 1900;
    t.tm_mon = mon - 1; // convert to 0 based month
    t.tm_mday = mday;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    t.tm_isdst = -1;    // Is DST on? 1 = yes, 0 = no, -1 = unknown
    return mktime(&t);  // returns seconds elapsed since January 1, 1970
}

