/*
    KDE Alarm Daemon GUI.

    This file is part of the GUI interface for the KDE alarm daemon.
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>
    Based on the original, (c) 1998, 1999 Preston Brown

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef CALENDARITERATION_H
#define CALENDARITERATION_H

#include "adcalendar.h"

// The CalendarIteration class gives secure public access to AlarmGui::mCalendars
class CalendarIteration
{
  public:
    CalendarIteration(CalendarList& c)  : calendars(c) { calendar = calendars.first(); }
    bool           ok() const           { return !!calendar; }
    bool           next()               { return !!(calendar = calendars.next()); }
    bool           available() const    { return calendar->available(); }
    bool           enabled() const      { return calendar->enabled(); }
    void           enabled(bool tf)     { calendar->enabled_ = tf; }
    const QString& urlString() const    { return calendar->urlString(); }
  private:
    CalendarList&  calendars;
    ADCalendar*    calendar;
};

#endif
