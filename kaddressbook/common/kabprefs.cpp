/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kabprefs.h"

#include <kconfig.h>
#include <klocale.h>

class KABPrefsHelper {
  public:
    KABPrefsHelper() : q( 0 ) {}
    ~KABPrefsHelper() { delete q; }
    KABPrefs *q;
};

K_GLOBAL_STATIC( KABPrefsHelper, s_globalKABPrefs )

KABPrefs::KABPrefs()
  : KABPrefsBase()
{
  Q_ASSERT(!s_globalKABPrefs->q);
  s_globalKABPrefs->q = this;

  KConfigSkeleton::setCurrentGroup( "General" );

  QStringList defaultMap;
  defaultMap << "http://maps.google.com/maps?f=q&hl=%1&q=%n,%l,%s";
  addItemString( "LocationMapURL", mLocationMapURL, defaultMap[ 0 ] );
  addItemStringList( "LocationMapURLs", mLocationMapURLs, defaultMap );
}

KABPrefs::~KABPrefs()
{
}

KABPrefs *KABPrefs::instance()
{
  if (!s_globalKABPrefs->q) {
    new KABPrefs;
    s_globalKABPrefs->q->readConfig();
  }

  return s_globalKABPrefs->q;
}

void KABPrefs::setCategoryDefaults()
{
  mCustomCategories.clear();
  mCustomCategories << i18nc( "@item Contact category", "Business" )
                    << i18nc( "@item Contact category", "Family" )
                    << i18nc( "@item Contact category", "School" )
                    << i18nc( "@item Contact category", "Customer" )
                    << i18nc( "@item Contact category", "Friend" );
}

void KABPrefs::usrReadConfig()
{
  KConfigGroup group( config(), "General" );
  mCustomCategories = group.readEntry( "Custom Categories" , QStringList() );
  if ( mCustomCategories.isEmpty() )
    setCategoryDefaults();

  KPimPrefs::usrReadConfig();
}


void KABPrefs::usrWriteConfig()
{
  KConfigGroup group( config(), "General" );
  group.writeEntry( "Custom Categories", mCustomCategories );

  KPimPrefs::usrWriteConfig();
}
