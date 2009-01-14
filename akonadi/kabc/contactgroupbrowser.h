/*
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef CONTACTGROUPBROWSER_H
#define CONTACTGROUPBROWSER_H

#include "akonadi-kabccommon_export.h"
#include "itembrowser.h"

namespace Akonadi {
class Item;

class AKONADI_KABCCOMMON_EXPORT ContactGroupBrowser : public ItemBrowser
{
  public:
    ContactGroupBrowser( QWidget *parent = 0 );
    virtual ~ContactGroupBrowser();

  protected:
    virtual QString itemToRichText( const Item &item );

  private:
    class Private;
    Private* const d;

    Q_DISABLE_COPY( ContactGroupBrowser )
};

}

#endif