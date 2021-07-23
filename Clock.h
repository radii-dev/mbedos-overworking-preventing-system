/*
 Clock.h
 
 Created on: Mar 24, 2015
     Author: Zoltan Hudak
 
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
#ifndef Clock_H_
#define Clock_H_

#include "mbed.h"

class   Clock
{
    Ticker*             _ticker;
    static time_t       _time;
    static struct tm    _tm;
protected:
    static Callback<void ()>    _onTick;
public:
    Clock(int year, int mon, int mday, int hour, int min, int sec);
    Clock();
    ~Clock();
    void                        set(int year, int mon, int mday, int hour, int min, int sec);
    void                        set(tm& val);
    void                        set(time_t time);
    static time_t               time();
    int                         year(void);
    int                         mon(void);
    int                         mday(void);
    int                         wday(void);
    int                         hour(void);
    int                         min(void);
    int                         sec(void);
    static void                 tick(void);
    static time_t               asTime(int year, int mon, int mday, int hour, int min, int sec);
    void                        attach(void (*fptr) (void));
    template<typename T> void   attach(T* tptr, void (T:: *mptr) (void));
    void                        detach();
};
#endif /* Clock_H_ */
