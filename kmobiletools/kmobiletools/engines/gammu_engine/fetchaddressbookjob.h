/***************************************************************************
   Copyright (C) 2005 by Marco Gulino <marco.gulino@gmail.com>
   Copyright (C) 2005 by Stefan Bogner <bochi@kmobiletools.org>
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

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

#ifndef FETCHADDRESSBOOKJOB_H
#define FETCHADDRESSBOOKJOB_H

#include "gammujob.h"

/**
 * This class triggers the fetch of the address book
 *
 * @author Matthias Lechner
 */
class FetchAddressBookJob : public GammuJob
{
    public:
        explicit FetchAddressBookJob( Device *device, kmobiletoolsGammu_engine* parent = 0,
                             const char* name = 0 );
        kmobiletoolsJob::JobType type() { return kmobiletoolsJob::fetchAddressBook; }

        ContactPtrList addresseeList();

    protected:
        void run();

    private:
        KABC::Addressee::List m_addresseeList;

};

#endif
