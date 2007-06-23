/*  This file is part of kdepim
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef NETWORKSTATUS_COMMON_H
#define NETWORKSTATUS_COMMON_H

#include <qstringlist.h>

namespace NetworkStatus
{
    enum Status { NoNetworks = 1, Unreachable, OfflineDisconnected,  OfflineFailed, ShuttingDown, Offline, Establishing, Online };
    enum RequestResult { RequestAccepted = 1, Connected, UserRefused, Unavailable };
//    QString toString( Status st );
}

#endif
