/*
  This file is part of the KDE Kontact Plugin Interface Library.

  Copyright (c) 2003 David Faure <faure@kde.org>

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

#include "uniqueapphandler.h"
#include "core.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kstartupinfo.h>
#include <kuniqueapplication.h>
#include <kwindowsystem.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>

/*
 Test plan for the various cases of interaction between standalone apps and kontact:

 1) start kontact, select "Mail".
 1a) type "korganizer" -> it switches to korganizer
 1b) type "kmail" -> it switches to kmail
 1c) type "kaddressbook" -> it switches to kaddressbook
 1d) type "kmail foo@kde.org" -> it opens a kmail composer, without switching
 1e) type "knode" -> it switches to knode
 1f) type "kaddressbook --new-contact" -> it opens a kaddressbook contact window
 1g) type "knode news://foobar/group" -> it pops up "can't resolve hostname"

 2) close kontact. Launch kmail. Launch kontact again.
 2a) click "Mail" icon -> kontact doesn't load a part, but activates the kmail window
 2b) type "kmail foo@kde.org" -> standalone kmail opens composer.
 2c) close kmail, click "Mail" icon -> kontact loads the kmail part.
 2d) type "kmail" -> kontact is brought to front

 3) close kontact. Launch korganizer, then kontact.
 3a) both Todo and Calendar activate the running korganizer.
 3b) type "korganizer" -> standalone korganizer is brought to front
 3c) close korganizer, click Calendar or Todo -> kontact loads part.
 3d) type "korganizer" -> kontact is brought to front

 4) close kontact. Launch kaddressbook, then kontact.
 4a) "Contacts" icon activate the running kaddressbook.
 4b) type "kaddressbook" -> standalone kaddressbook is brought to front
 4c) close kaddressbook, type "kaddressbook -a foo@kde.org" -> kontact loads part and opens editor
 4d) type "kaddressbook" -> kontact is brought to front

 5) close kontact. Launch knode, then kontact.
 5a) "News" icon activate the running knode.
 5b) type "knode" -> standalone knode is brought to front
 5c) close knode, type "knode news://foobar/group" -> kontact loads knode and pops up msgbox
 5d) type "knode" -> kontact is brought to front

 6) start "kontact --module summaryplugin"
 6a) type "dcop kmail kmail newInstance" -> kontact switches to kmail (#103775)
 6b) type "kmail" -> kontact is brought to front
 6c) type "kontact" -> kontact is brought to front
 6d) type "kontact --module summaryplugin" -> kontact switches to summary

*/

using namespace Kontact;

//@cond PRIVATE
class UniqueAppHandler::Private
{
  public:
    Plugin *mPlugin;
};
//@endcond

UniqueAppHandler::UniqueAppHandler( Plugin *plugin )
 : d( new Private )
{
  kDebug() << " plugin->objectName().toLatin1() :" << plugin->objectName().toLatin1();

  d->mPlugin = plugin;
  QDBusConnection::sessionBus().registerService(
    "org.kde." + plugin->objectName().toLatin1() );
}

UniqueAppHandler::~UniqueAppHandler()
{
  delete d;
}

int UniqueAppHandler::newInstance()
{
  QWidget *w = kapp->activeWindow();
  if ( w ) {
    w->show();
    KWindowSystem::forceActiveWindow( w->winId() );
    KStartupInfo::appStarted();
  }

  // Then ensure the part appears in kontact
  d->mPlugin->core()->selectPlugin( d->mPlugin );
  return 0;
}

Plugin *UniqueAppHandler::plugin() const
{
  return d->mPlugin;
}

#ifdef __GNUC__
#warning Port to DBus!
#endif
/*bool UniqueAppHandler::process( const DCOPCString &fun, const QByteArray &data,
                                DCOPCString& replyType, QByteArray &replyData )
{
  if ( fun == "newInstance()" ) {
    replyType = "int";

    KCmdLineArgs::reset(); // forget options defined by other "applications"
    loadCommandLineOptions(); // implemented by plugin

    // This bit is duplicated from KUniqueApplication::processDelayed()
    QByteArray tmp(data);
	QDataStream ds( &tmp,QIODevice::ReadOnly );
    ds.setVersion(QDataStream::Qt_3_1);
    KCmdLineArgs::loadAppArgs( ds );
    if ( !ds.atEnd() ) { // backwards compatibility
      QByteArray asn_id;
      ds >> asn_id;
      kapp->setStartupId( asn_id );
    }

    QDataStream _replyStream( &replyData,QIODevice::WriteOnly );
    _replyStream.setVersion(QDataStream::Qt_3_1);
    _replyStream << newInstance( );
  } else if ( fun == "load()" ) {
    replyType = "bool";
    (void)d->mPlugin->part(); // load the part without bringing it to front

    QDataStream _replyStream( &replyData,QIODevice::WriteOnly );
    _replyStream.setVersion(QDataStream::Qt_3_1);
    _replyStream << true;
  } else {
    return DCOPObject::process( fun, data, replyType, replyData );
  }
  return true;
}

DCOPCStringList UniqueAppHandler::interfaces()
{
  DCOPCStringList ifaces = DCOPObject::interfaces();
  ifaces += "Kontact::UniqueAppHandler";
  return ifaces;
}

DCOPCStringList UniqueAppHandler::functions()
{
  DCOPCStringList funcs = DCOPObject::functions();
  funcs << "int newInstance()";
  funcs << "bool load()";
  return funcs;
}
*/

//@cond PRIVATE
class UniqueAppWatcher::Private
{
  public:
    UniqueAppHandlerFactoryBase *mFactory;
    Plugin *mPlugin;
    bool mRunningStandalone;
};
//@endcond

UniqueAppWatcher::UniqueAppWatcher( UniqueAppHandlerFactoryBase *factory, Plugin *plugin )
  : QObject( plugin ), d( new Private )
{
  d->mFactory = factory;
  d->mPlugin = plugin;

  // The app is running standalone if 1) that name is known to D-Bus
  QString serviceName = "org.kde." + plugin->objectName().toLatin1();
  d->mRunningStandalone =
    QDBusConnection::sessionBus().interface()->isServiceRegistered( serviceName );
  kDebug() << " plugin->objectName() :" << plugin->objectName()
           << " isServiceRegistered ? :" << d->mRunningStandalone;

  QString owner = QDBusConnection::sessionBus().interface()->serviceOwner( serviceName );
  if ( d->mRunningStandalone && ( owner == QDBusConnection::sessionBus().baseService() ) ) {
    d->mRunningStandalone = false;
  }

  if ( d->mRunningStandalone ) {
    //TODO port it
    //kapp->dcopClient()->setNotifications( true );
    //connect( kapp->dcopClient(), SIGNAL(applicationRemoved(const QByteArray&)),
    //         this, SLOT(unregisteredFromDCOP(const QByteArray&)) );
  } else {
    d->mFactory->createHandler( d->mPlugin );
  }
}

UniqueAppWatcher::~UniqueAppWatcher()
{
  delete d;

#ifdef __GNUC__
#warning Port to DBus!
#endif
//  if ( d->mRunningStandalone )
//    kapp->dcopClient()->setNotifications( false );

  //TODO: deleting the mFactory here causes a crash. determine why, or if this
  //is needed at all.
  //delete d->mFactory;
}

bool UniqueAppWatcher::isRunningStandalone() const
{
  return d->mRunningStandalone;
}

void UniqueAppWatcher::unregisteredFromDCOP( const QByteArray &appId )
{
  if ( appId == d->mPlugin->objectName() && d->mRunningStandalone ) {
#ifdef __GNUC__
#warning Port to DBus!
#endif
//    disconnect( kapp->dcopClient(), SIGNAL(applicationRemoved(const QByteArray&)),
//                this, SLOT(unregisteredFromDCOP(const QByteArray&)) );
    kDebug() << appId;
    d->mFactory->createHandler( d->mPlugin );
//    kapp->dcopClient()->setNotifications( false );
    d->mRunningStandalone = false;
  }
}

void Kontact::UniqueAppHandler::loadKontactCommandLineOptions()
{

  KCmdLineOptions options;
  options.add( "module <module>", ki18n( "Start with a specific Kontact module" ) );
  options.add( "iconify", ki18n( "Start in iconified (minimized) mode" ) );
  options.add( "list", ki18n( "List all possible modules and exit" ) );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();
  KCmdLineArgs::addStdCmdLineOptions();
}

#include "uniqueapphandler.moc"