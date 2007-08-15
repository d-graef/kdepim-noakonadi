/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>
   Copyright (C) 2007 by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KMOBILETOOLSADDRESSBOOK_H
#define KMOBILETOOLSADDRESSBOOK_H

#include <QtCore/QList>

#include <libkmobiletools/kmobiletools_export.h>
#include <libkmobiletools/addressbookentry.h>

namespace KMobileTools {
/**
 * This class holds a list of contacts (former ContactsList)
 *
 * @author Matthias Lechner <matthias@lmme.de>
 * @author Marco Gulino <marco@kmobiletools.org>
 */
class KMOBILETOOLS_EXPORT Addressbook : public QList<AddressbookEntry> {
public:
    /**
     * Constructs an empty address book
     */
    Addressbook();

    /**
     * Destructs the address book
     */
    ~Addressbook();

    Addressbook( const Addressbook& entry );
    Addressbook& operator=( const Addressbook& addressbook );
};

}

#endif
