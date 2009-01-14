/*
    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

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

#include "contactgroupeditor.h"

#include <QtGui/QGridLayout>
#include <QtGui/QMessageBox>

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/monitor.h>
#include <akonadi/session.h>

#include "ui_contactgroupeditor.h"
#include "waitingoverlay.h"

namespace Akonadi
{

class ContactGroupEditor::Private
{
  public:
    Private( ContactGroupEditor *parent )
      : mParent( parent ), mMonitor( 0 )
    {
    }

    ~Private()
    {
      delete mMonitor;
    }

    void fetchDone( KJob* );
    void storeDone( KJob* );
    void itemChanged( const Akonadi::Item &item, const QSet<QByteArray>& );
    void memberChanged();

    void loadContactGroup( const KABC::ContactGroup &group );
    bool storeContactGroup( KABC::ContactGroup &group );
    void setupMonitor();
    KLineEdit* addMemberEdit();

    ContactGroupEditor *mParent;
    ContactGroupEditor::Mode mMode;
    Item mItem;
    Monitor *mMonitor;
    Collection mDefaultCollection;
    Ui::ContactGroupEditor gui;
    QVBoxLayout *membersLayout;
    QList<KLineEdit*> members;
};

}

using namespace Akonadi;

void ContactGroupEditor::Private::fetchDone( KJob *job )
{
  if ( job->error() )
    return;

  ItemFetchJob *fetchJob = qobject_cast<ItemFetchJob*>( job );
  if ( !fetchJob )
    return;

  if ( fetchJob->items().isEmpty() )
    return;

  mItem = fetchJob->items().first();

  const KABC::ContactGroup group = mItem.payload<KABC::ContactGroup>();
  loadContactGroup( group );
}

void ContactGroupEditor::Private::storeDone( KJob *job )
{
  if ( job->error() ) {
    emit mParent->error( job->errorString() );
    return;
  }

  if ( mMode == EditMode )
    emit mParent->contactGroupStored( mItem );
  else if ( mMode == CreateMode )
    emit mParent->contactGroupStored( static_cast<ItemCreateJob*>( job )->item() );
}

void ContactGroupEditor::Private::itemChanged( const Item&, const QSet<QByteArray>& )
{
  QMessageBox dlg( mParent );

  dlg.setInformativeText( QLatin1String( "The contact group has been changed by anyone else\nWhat shall be done?" ) );
  dlg.addButton( QLatin1String( "Take over changes" ), QMessageBox::AcceptRole );
  dlg.addButton( QLatin1String( "Ignore and Overwrite changes" ), QMessageBox::RejectRole );

  if ( dlg.exec() == QMessageBox::AcceptRole ) {
    ItemFetchJob *job = new ItemFetchJob( mItem );
    job->fetchScope().fetchFullPayload();

    mParent->connect( job, SIGNAL( result( KJob* ) ), mParent, SLOT( fetchDone( KJob* ) ) );
    new WaitingOverlay( job, mParent );
  }
}

void ContactGroupEditor::Private::loadContactGroup( const KABC::ContactGroup &group )
{
  gui.groupName->setText( group.name() );

  qDeleteAll( members );
  for ( unsigned int i = 0; i < group.dataCount(); ++i ) {
    const KABC::ContactGroup::Data data = group.data( i );
    KLineEdit *lineEdit = addMemberEdit();

    mParent->disconnect( lineEdit, SIGNAL( textChanged( const QString& ) ), mParent, SLOT( memberChanged() ) );
    lineEdit->setText( QString::fromLatin1( "%1 <%2>" ).arg( data.name() ).arg( data.email() ) );
    mParent->connect( lineEdit, SIGNAL( textChanged( const QString& ) ), SLOT( memberChanged() ) );
  }

  addMemberEdit();
}

bool ContactGroupEditor::Private::storeContactGroup( KABC::ContactGroup &group )
{
  if ( gui.groupName->text().isEmpty() ) {
    KMessageBox::error( mParent, i18n( "The name of the contact group must not be empty." ) );
    return false;
  }

  group.setName( gui.groupName->text() );

  group.removeAllContactData();
  for ( int i = 0; i < members.count(); ++i ) {
    const QString text = members.at( i )->text();
    if ( !text.isEmpty() ) {
      QString fullName, email;
      KABC::Addressee::parseEmailAddress( text, fullName, email );

      if ( fullName.isEmpty() ) {
        KMessageBox::error( mParent, i18n( "The contact '%1' is missing a name.", text ) );
        return false;
      }
      if ( email.isEmpty() ) {
        KMessageBox::error( mParent, i18n( "The contact '%1' is missing an email address.", text ) );
        return false;
      }

      group.append( KABC::ContactGroup::Data( fullName, email ) );
    }
  }

  return true;
}

void ContactGroupEditor::Private::setupMonitor()
{
  delete mMonitor;
  mMonitor = new Monitor;
  mMonitor->ignoreSession( Session::defaultSession() );

  connect( mMonitor, SIGNAL( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ),
           mParent, SLOT( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ) );
}

KLineEdit* ContactGroupEditor::Private::addMemberEdit()
{
  KLineEdit *lineEdit = new KLineEdit( mParent );
  lineEdit->setToolTip( i18n( "Enter member in format: name <email>" ) );
  members.append( lineEdit );
  membersLayout->addWidget( lineEdit );
  mParent->connect( lineEdit, SIGNAL( textChanged( const QString& ) ), SLOT( memberChanged() ) );

  return lineEdit;
}

void ContactGroupEditor::Private::memberChanged()
{
  // remove last line edit iff last and second last line edits are both empty
  if ( members.count() >= 2 ) { // only if more than 1 line edit available
    if ( members.at( members.count() - 1 )->text().isEmpty() &&
         members.at( members.count() - 2 )->text().isEmpty() ) {

      // delete line edit with deleteLater as this slot might be called by
      // that line edit
      members.at( members.count() - 1 )->deleteLater();

      // remove line edit from list of known line edits
      members.removeAt( members.count() - 1 );
    }
  }

  // add new line edit if the last one contains text
  if ( !members.last()->text().isEmpty() )
    addMemberEdit();
}


ContactGroupEditor::ContactGroupEditor( Mode mode, QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  d->mMode = mode;
  d->gui.setupUi( this );
  d->gui.gridLayout->setRowStretch( 2, 1 );
  QVBoxLayout *layout = new QVBoxLayout( d->gui.memberWidget );
  d->membersLayout = new QVBoxLayout();
  layout->addLayout( d->membersLayout );
  layout->addStretch();

  if ( d->mMode == CreateMode )
    d->addMemberEdit();
}


ContactGroupEditor::~ContactGroupEditor()
{
}

void ContactGroupEditor::loadContactGroup( const Akonadi::Item &item )
{
  if ( d->mMode == CreateMode )
    Q_ASSERT_X( false, "ContactGroupEditor::loadContactGroup", "You are calling loadContactGroup in CreateMode!" );

  ItemFetchJob *job = new ItemFetchJob( item );
  job->fetchScope().fetchFullPayload();

  connect( job, SIGNAL( result( KJob* ) ), SLOT( fetchDone( KJob* ) ) );

  d->setupMonitor();
  d->mMonitor->setItemMonitored( item );

  new WaitingOverlay( job, this );
}

bool ContactGroupEditor::saveContactGroup()
{
  if ( d->mMode == EditMode ) {
    if ( !d->mItem.isValid() )
      return false;

    KABC::ContactGroup group = d->mItem.payload<KABC::ContactGroup>();

    if ( !d->storeContactGroup( group ) )
      return false;

    d->mItem.setPayload<KABC::ContactGroup>( group );

    ItemModifyJob *job = new ItemModifyJob( d->mItem );
    connect( job, SIGNAL( result( KJob* ) ), SLOT( storeDone( KJob* ) ) );
  } else if ( d->mMode == CreateMode ) {
    Q_ASSERT_X( d->mDefaultCollection.isValid(), "ContactGroupEditor::saveContactGroup", "Using invalid default collection for saving!" );

    KABC::ContactGroup group;
    if ( !d->storeContactGroup( group ) )
      return false;

    Item item;
    item.setPayload<KABC::ContactGroup>( group );
    item.setMimeType( KABC::ContactGroup::mimeType() );

    ItemCreateJob *job = new ItemCreateJob( item, d->mDefaultCollection );
    connect( job, SIGNAL( result( KJob* ) ), SLOT( storeDone( KJob* ) ) );
  }

  return true;
}

void ContactGroupEditor::setDefaultCollection( const Akonadi::Collection &collection )
{
  d->mDefaultCollection = collection;
}

#include "contactgroupeditor.moc"