/*
    This file is part of KAddressbook.
    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klocale.h>
#include <kparts/genericfactory.h>

#include "kabcore.h"
#include "kaddressbookiface.h"
#include "libkdepim/aboutdataextension.h"
#include "libkdepim/infoextension.h"

#include "kaddressbook_part.h"

typedef KParts::GenericFactory< KAddressbookPart > KAddressbookFactory;
K_EXPORT_COMPONENT_FACTORY( libkaddressbookpart, KAddressbookFactory )

KAddressbookPart::KAddressbookPart( QWidget *parentWidget, const char *widgetName,
                                    QObject *parent, const char *name,
                                    const QStringList & )
  : DCOPObject( "KAddressBookIface" ), KParts::ReadOnlyPart( parent, name )
{
  kdDebug(5720) << "KAddressbookPart()" << endl;
  kdDebug(5720) << "  InstanceName: " << kapp->instanceName() << endl;

  setInstance( KAddressbookFactory::instance() );

  kdDebug(5720) << "KAddressbookPart()..." << endl;
  kdDebug(5720) << "  InstanceName: " << kapp->instanceName() << endl;

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget( parentWidget, widgetName );
  canvas->setFocusPolicy( QWidget::ClickFocus );
  setWidget( canvas );

  mExtension = new KAddressbookBrowserExtension( this );

  QVBoxLayout *topLayout = new QVBoxLayout( canvas );

  KGlobal::iconLoader()->addAppDir( "kaddressbook" );

  mCore = new KABCore( this, true, canvas );
  mCore->restoreSettings();
  topLayout->addWidget( mCore->widget() );

  KParts::InfoExtension *info = new KParts::InfoExtension( this, "KABPart" );
  connect( mCore, SIGNAL( contactSelected( const QString& ) ),
           info, SIGNAL( textChanged( const QString& ) ) );
  connect( mCore, SIGNAL( contactSelected( const QPixmap& ) ),
           info, SIGNAL( iconChanged( const QPixmap& ) ) );

  new KParts::AboutDataExtension( createAboutData(), this, "AboutData" );

  setXMLFile( "kaddressbook_part.rc" );
}

KAddressbookPart::~KAddressbookPart()
{
  mCore->save();
  closeURL();
}

KAboutData *KAddressbookPart::createAboutData()
{
  return KABCore::createAboutData();
}

void KAddressbookPart::addEmail( QString addr )
{
  mCore->addEmail( addr );
}

ASYNC KAddressbookPart::showContactEditor( QString uid )
{
  mCore->editContact( uid );
}

void KAddressbookPart::newContact()
{
  mCore->newContact();
}

QString KAddressbookPart::getNameByPhone( QString phone )
{
  return mCore->getNameByPhone( phone );
}

void KAddressbookPart::save()
{
  mCore->save();
}

void KAddressbookPart::exit()
{
  delete this;
}

bool KAddressbookPart::openFile()
{
  kdDebug(5720) << "KAddressbookPart:openFile()" << endl;

  mCore->widget()->show();
  return true;
}

void KAddressbookPart::guiActivateEvent( KParts::GUIActivateEvent *e )
{
  kdDebug(5720) << "KAddressbookPart::guiActivateEvent" << endl;
  KParts::ReadOnlyPart::guiActivateEvent( e );
}

KAddressbookBrowserExtension::KAddressbookBrowserExtension( KAddressbookPart *parent )
  : KParts::BrowserExtension( parent, "KAddressbookBrowserExtension" )
{
}

KAddressbookBrowserExtension::~KAddressbookBrowserExtension()
{
}

using namespace KParts;

#include "kaddressbook_part.moc"
