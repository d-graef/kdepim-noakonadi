/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2005 David Faure <faure@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "reminderclient.h"
#include "korganizer_korgac_interface.h"

#include <KDebug>
#include <KStandardDirs>
#include <KToolInvocation>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>

using namespace KPIM;

ReminderClient::ReminderClient()
{
  kDebug(5850) << "ReminderClient::ReminderClient()";
}

void ReminderClient::startDaemon()
{
  QDBusInterface iface( QLatin1String( "org.kde.korgac" ), QLatin1String( "/" ) );
  if ( iface.isValid() ){
    // Reminder daemon already runs
    return;
  }

  KGlobal::dirs()->addResourceType( "autostart", 0, "share/autostart" );
  QString desktopFile = KStandardDirs::locate( "autostart", QLatin1String( "korgac.desktop" ) );
  if ( desktopFile.isEmpty() ) {
    kWarning() << "Couldn't find autostart/korgac.desktop!";
  } else {
    QString error;
    if ( KToolInvocation::startServiceByDesktopPath( desktopFile, QStringList(), &error ) != 0 ) {
      kWarning() << "Failure starting korgac:" << error;
    }
  }
}

void ReminderClient::stopDaemon()
{
  OrgKdeKorganizerKOrgacInterface interface(
    QLatin1String( "org.kde.korgac" ), QLatin1String( "/ac" ), QDBusConnection::sessionBus() );
  interface.quit();
}

void ReminderClient::hideDaemon()
{
  OrgKdeKorganizerKOrgacInterface interface(
    QLatin1String( "org.kde.korgac" ), QLatin1String( "/ac" ), QDBusConnection::sessionBus() );
  interface.hide();
}

void ReminderClient::showDaemon()
{
  OrgKdeKorganizerKOrgacInterface interface(
    QLatin1String( "org.kde.korgac" ), QLatin1String( "/ac" ), QDBusConnection::sessionBus() );
  interface.show();
}

