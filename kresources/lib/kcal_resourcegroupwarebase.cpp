/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "kcal_resourcegroupwarebase.h"
#include "kresources_groupwareprefs.h"

#include "folderlister.h"
#include "calendaradaptor.h"
#include "groupwaredownloadjob.h"
#include "groupwareuploadjob.h"

#include <kcal/icalformat.h>
#include <kcal/calendarlocal.h>
#include <kcal/confirmsavedialog.h>
#include <kio/job.h>
#include <klocale.h>

#include <QApplication>

using namespace KCal;

ResourceGroupwareBase::ResourceGroupwareBase()
  : ResourceCached(), mPrefs(0), mFolderLister(0),
    mLock( true ), mAdaptor(0), mDownloadJob(0), mUploadJob(0)
{
}

ResourceGroupwareBase::ResourceGroupwareBase( const KConfigGroup &group )
  : ResourceCached( group ), mPrefs(0), mFolderLister(0),
    mLock( true ), mAdaptor(0), mDownloadJob(0), mUploadJob(0)
{
  readConfig( group );
}

ResourceGroupwareBase::~ResourceGroupwareBase()
{
  disableChangeNotification();

  delete mPrefs;
  mPrefs = 0;
}

KPIM::GroupwarePrefsBase *ResourceGroupwareBase::createPrefs()
{
  return new KPIM::GroupwarePrefsBase();
}


bool ResourceGroupwareBase::addEvent( Event *event )
{
  if ( adaptor() && ( adaptor()->supports( KPIM::FolderLister::Event ) ||
                      adaptor()->supports( KPIM::FolderLister::All ) ) ) {
    return ResourceCached::addEvent( event );
  } else return false;
}

bool ResourceGroupwareBase::addTodo( Todo *todo )
{
  if ( adaptor() && ( adaptor()->supports( KPIM::FolderLister::Todo ) ||
                      adaptor()->supports( KPIM::FolderLister::All ) ) ) {
    return ResourceCached::addTodo( todo );
  } else return false;
}

bool ResourceGroupwareBase::addJournal( Journal *journal )
{
  if ( adaptor() && ( adaptor()->supports( KPIM::FolderLister::Journal ) ||
                      adaptor()->supports( KPIM::FolderLister::All ) ) ) {
    return ResourceCached::addJournal( journal );
  } else return false;
}


KPIM::GroupwareDownloadJob *ResourceGroupwareBase::createDownloadJob(
                            CalendarAdaptor *adaptor )
{
  return new KPIM::GroupwareDownloadJob( adaptor );
}

KPIM::GroupwareUploadJob *ResourceGroupwareBase::createUploadJob(
                          CalendarAdaptor *adaptor )
{
  return new KPIM::GroupwareUploadJob( adaptor );
}

void ResourceGroupwareBase::setPrefs( KPIM::GroupwarePrefsBase *newprefs )
{
  if ( !newprefs ) return;
  if ( mPrefs ) delete mPrefs;
  mPrefs = newprefs;
  mPrefs->addGroupPrefix( identifier() );

  mPrefs->readConfig();
  if ( mFolderLister ) mFolderLister->readConfig( mPrefs );
}

void ResourceGroupwareBase::setFolderLister( KPIM::FolderLister *folderLister )
{
  if ( !folderLister ) return;
  if ( mFolderLister ) delete mFolderLister;
  mFolderLister = folderLister;
  if ( mPrefs ) mFolderLister->readConfig( mPrefs );
  if ( adaptor() ) {
    adaptor()->setFolderLister( mFolderLister );
    mFolderLister->setAdaptor( adaptor() );
  }
}

void ResourceGroupwareBase::setAdaptor( CalendarAdaptor *adaptor )
{
  if ( !adaptor ) return;
  if ( mAdaptor ) delete mAdaptor;
  mAdaptor = adaptor;
  mAdaptor->setFolderLister( mFolderLister );
  if ( mFolderLister ) mFolderLister->setAdaptor( mAdaptor );
  // TODO: Set the prgreess status in the up/download jobs, and set
  //       the resource name as the label. This should be done in the
  //       up/download jobs, but these don't have access to the resource's
  //       name, do they?
/*  mAdaptor->setDownloadProgressMessage( i18n("Downloading calendar") );
  mAdaptor->setUploadProgressMessage( i18n("Uploading calendar") );*/
  if ( prefs() ) {
    mAdaptor->setBaseURL( prefs()->url() );
    mAdaptor->setUser( prefs()->user() );
    mAdaptor->setPassword( prefs()->password() );
  }
  mAdaptor->setIdMapper( &idMapper() );
  mAdaptor->setResource( this );
}

void ResourceGroupwareBase::init()
{
  mDownloadJob = 0;
  mIsShowingError = false;

  enableChangeNotification();
}

KPIM::GroupwarePrefsBase *ResourceGroupwareBase::prefs()
{
  return mPrefs;
}

void ResourceGroupwareBase::readConfig( const KConfigGroup &group )
{
  kDebug(5800);
  ResourceCached::readConfig( group );
  if ( mPrefs ) {
    mPrefs->readConfig();
    if ( mFolderLister )
      mFolderLister->readConfig( mPrefs );
  }
}

void ResourceGroupwareBase::writeConfig( KConfigGroup &group )
{
  kDebug(5800);
  ResourceCalendar::writeConfig( group );
  ResourceCached::writeConfig( group );

  if ( mPrefs ) {
    if ( mFolderLister )
      mFolderLister->writeConfig( mPrefs );
    mPrefs->writeConfig();
  }
}

bool ResourceGroupwareBase::doOpen()
{
  if ( !adaptor() )
    return false;
  if ( adaptor()->flags() & KPIM::GroupwareDataAdaptor::GWResNeedsLogon ) {
    KIO::Job *loginJob = adaptor()->createLoginJob( prefs()->url(), prefs()->user(), prefs()->password() );
    if ( !loginJob ) {
      return false;
    } else {
      mLoginFinished = false;
      connect( loginJob, SIGNAL( result( KJob * ) ),
               SLOT( slotLoginJobResult( KJob* ) ) );
      enter_loop();
      return mLoginFinished;
    }
  }
  return true;
}

// BEGIN:COPIED
// TODO: Get rid of this hack, which is copied from KIO::NetAccess, which is
// LGPL'ed and
//     Copyright (C) 1997 Torben Weis (weis@kde.org)
//     Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
//     Copyright (C) 1999 David Faure (faure@kde.org)
// If a troll sees this, he kills me
//
// It's no longer a hack but still taken from KIO::NetAccess

void ResourceGroupwareBase::enter_loop()
{
  QEventLoop eventLoop;
  connect( this, SIGNAL( leaveModality() ), &eventLoop, SLOT( quit() ) );
  eventLoop.exec( QEventLoop::ExcludeUserInputEvents );
}
// END:COPIED

void ResourceGroupwareBase::slotLoginJobResult( KJob *job )
{
  if ( !adaptor() ) return;
  mLoginFinished = adaptor()->interpretLoginJobResult( static_cast<KIO::Job*>(job) );
  emit leaveModality();
}

void ResourceGroupwareBase::doClose()
{
  ResourceCached::doClose();
  if ( mDownloadJob ) mDownloadJob->kill();

  if ( adaptor() &&
       adaptor()->flags() & KPIM::GroupwareDataAdaptor::GWResNeedsLogoff ) {
    KIO::Job *logoffJob = adaptor()->createLogoffJob( prefs()->url(), prefs()->user(), prefs()->password() );
    connect( logoffJob, SIGNAL( result( KJob * ) ),
             SLOT( slotLogoffJobResult( KJob* ) ) );
    // TODO: Do we really need to block while waiting for the job to return?
    enter_loop();
  }
}

void ResourceGroupwareBase::slotLogoffJobResult( KJob *job )
{
  if ( !adaptor() ) return;
  adaptor()->interpretLogoffJobResult( static_cast<KIO::Job*>(job) );
  // TODO: Do we really need to block while waiting for the job to return?
  emit leaveModality();
}

bool ResourceGroupwareBase::doLoad( bool syncCache )
{
  kDebug(5800);

  Q_UNUSED( syncCache );
  if ( mIsShowingError ) {
    kDebug(5800) << "Still showing error";
    return true;
  }

  if ( mDownloadJob ) {
    kWarning() << "Download still in progress";
    return false;
  }

  calendar()->close();
  clearChanges();
  disableChangeNotification();
  loadFromCache();
  enableChangeNotification();
  emit resourceChanged( this );

  mDownloadJob = createDownloadJob( adaptor() );
  connect( mDownloadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
    SLOT( slotDownloadJobResult( KPIM::GroupwareJob * ) ) );

  return true;
}

void ResourceGroupwareBase::slotDownloadJobResult( KPIM::GroupwareJob *job )
{
  kDebug(5800);

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
  } else {
    kDebug(5800) << "Successfully downloaded data";

    clearChanges();
    saveToCache();
    enableChangeNotification();

    emit resourceChanged( this );
    emit resourceLoaded( this );
  }

  mDownloadJob = 0;
}

bool ResourceGroupwareBase::doSave( bool syncCache )
{
  kDebug(5800);

  Q_UNUSED( syncCache );
  saveToCache();

  if ( !hasChanges() ) {
    kDebug(5800) << "No changes";
    return true;
  }
  // TODO: Implement confirming of single changes i.e. it should be possible
  //       to upload only certain changes and discard the rest. This is
  //       particularly important for resources like the blogging resource,
  //       where uploading would mean a republication of the blog, not only
  //       a modifications.
  if ( !confirmSave() ) return false;

  mUploadJob = createUploadJob( adaptor() );
  connect( mUploadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
    SLOT( slotUploadJobResult( KPIM::GroupwareJob * ) ) );

  Incidence::List inc;
  Incidence::List::Iterator it;
  KPIM::GroupwareUploadItem::List addedItems, changedItems, deletedItems;

  inc = addedIncidences();
  for( it = inc.begin(); it != inc.end(); ++it ) {
    addedItems.append( adaptor()->newUploadItem( *it,
                                           KPIM::GroupwareUploadItem::Added ) );
  }
  // TODO: Check if the item has changed on the server...
  // In particular, check if the version we based our change on is still current
  // on the server
  inc = changedIncidences();
  for( it = inc.begin(); it != inc.end(); ++it ) {
    changedItems.append( adaptor()->newUploadItem( *it,
                                         KPIM::GroupwareUploadItem::Changed ) );
  }
  inc = deletedIncidences();
  for( it = inc.begin(); it != inc.end(); ++it ) {
    deletedItems.append( adaptor()->newUploadItem( *it,
                                         KPIM::GroupwareUploadItem::Deleted ) );
  }

  mUploadJob->setAddedItems( addedItems );
  mUploadJob->setChangedItems( changedItems );
  mUploadJob->setDeletedItems( deletedItems );

  // FIXME: Calling clearChanges() here is not the ideal way since the
  // upload might fail, but there is no other place to call it...
  clearChanges();
  return true;
}

bool ResourceGroupwareBase::doSave( bool syncCache, Incidence *incidence )
{
  Q_UNUSED( syncCache );
  Q_UNUSED( incidence );
  return true;
}

void ResourceGroupwareBase::slotUploadJobResult( KPIM::GroupwareJob *job )
{
  kDebug(5800);

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
  } else {
    kDebug(5800) << "Successfully uploaded data";
    /*
     * After the put the server might have expanded recurring events and will
     * also change the uids of the uploaded events. Remove them from the cache
     * and get the fresh delta and download.
     */

    if ( !mDownloadJob ) {
      mDownloadJob = createDownloadJob( adaptor() );
      connect( mDownloadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
          SLOT( slotDownloadJobResult( KPIM::GroupwareJob * ) ) );
    } else {
      kWarning() << "Download still in progress. Can't happen. (TM)";
    }
  }

  mDownloadJob = 0;
}

// FIXME: Put this into ResourceCached
bool ResourceGroupwareBase::confirmSave()
{
  if ( !hasChanges() ) return true;

  ConfirmSaveDialog dlg( resourceName(), 0 );

  dlg.addIncidences( addedIncidences(), i18n("Added") );
  dlg.addIncidences( changedIncidences(), i18n("Changed") );
  dlg.addIncidences( deletedIncidences(), i18n("Deleted") );

  int result = dlg.exec();
  return result == QDialog::Accepted;
}

KABC::Lock *ResourceGroupwareBase::lock()
{
  return &mLock;
}

#include "kcal_resourcegroupwarebase.moc"
