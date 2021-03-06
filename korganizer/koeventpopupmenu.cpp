/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koeventpopupmenu.h"
#include <kmimetypetrader.h>
#include "actionmanager.h"
#include "calendarview.h"
#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif
#include "kocorehelper.h"
#include "koglobals.h"
#include "stdcalendar.h"
#include "incidencechanger.h"

#include <KCal/CalendarResources>
#include <KCal/CalFormat>
#include <KCal/Event>
#include <KCal/Incidence>
#include <KCal/Journal>
#include <KCal/Todo>

#include <KActionCollection>
#include <KLocale>

KOEventPopupMenu::KOEventPopupMenu()
  : mCopyToCalendarMenu( 0 ), mMoveToCalendarMenu( 0 )
{
  mCalendar = 0;
  mCurrentIncidence = 0;
  mCurrentDate = QDate();
  mHasAdditionalItems = false;

  addAction( KOGlobals::self()->smallIcon( "document-preview" ), i18n( "&Show" ),
             this, SLOT( popupShow() ) );
  mEditOnlyItems.append(
    addAction( KOGlobals::self()->smallIcon( "document-edit" ), i18n( "&Edit..." ),
               this, SLOT( popupEdit() ) ) );
#ifndef KORG_NOPRINTER
  mEditOnlyItems.append( addSeparator() );
  addAction( KOGlobals::self()->smallIcon( "document-print" ), i18n( "&Print..." ),
             this, SLOT( print() ) );
  QAction *preview = addAction( KOGlobals::self()->smallIcon( "document-print-preview" ), i18n( "Print Previe&w..." ),
             this, SLOT( printPreview() ) );
  preview->setEnabled( !KMimeTypeTrader::self()->query("application/pdf", "KParts/ReadOnlyPart").isEmpty() );

#endif
  //------------------------------------------------------------------------
  mEditOnlyItems.append( addSeparator() );
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( "edit-cut" ),
                                    i18nc( "cut this event", "C&ut" ),
                                    this, SLOT(popupCut()) ) );
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( "edit-copy" ),
                                    i18nc( "copy this event", "&Copy" ),
                                    this, SLOT(popupCopy()) ) );
  // paste is always possible
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( "edit-paste" ),
                                    i18n( "&Paste" ),
                                    this, SLOT(popupPaste()) ) );
  mEditOnlyItems.append( addAction( KOGlobals::self()->smallIcon( "edit-delete" ),
                                    i18nc( "delete this incidence", "&Delete" ),
                                    this, SLOT(popupDelete()) ) );
  //------------------------------------------------------------------------
  mEditOnlyItems.append( addSeparator() );
  mTodoOnlyItems.append( addAction( KOGlobals::self()->smallIcon( "task-complete" ),
                                    i18n( "Togg&le To-do Completed" ),
                                    this, SLOT(toggleTodoCompleted()) ) );
  mToggleReminder = addAction( QIcon( KOGlobals::self()->smallIcon( "appointment-reminder" ) ),
                                    i18n( "&Toggle Reminder" ), this, SLOT(toggleAlarm()));
  mEditOnlyItems.append( mToggleReminder );
  //------------------------------------------------------------------------
  mRecurrenceItems.append( addSeparator() );
  mDissociateOccurrences = addAction( i18n( "&Dissociate From Recurrence..." ),
                                      this, SLOT(dissociateOccurrences()) );
  mRecurrenceItems.append( mDissociateOccurrences );

  addSeparator();
  addAction( KOGlobals::self()->smallIcon( "mail-forward" ),
             i18n( "Send as iCalendar..." ),
             this, SLOT(forward()) );
}

void KOEventPopupMenu::showIncidencePopup( Calendar *cal, Incidence *incidence, const QDate &qd )
{
  mCalendar = cal;
  mCurrentIncidence = incidence;
  mCurrentDate = qd;

  if ( mCopyToCalendarMenu ) {
    removeAction( mCopyToCalendarMenu->menuAction() );
    delete mCopyToCalendarMenu;
    mCopyToCalendarMenu = 0;
  }
  if ( mMoveToCalendarMenu ) {
    removeAction( mMoveToCalendarMenu->menuAction() );
    delete mMoveToCalendarMenu;
    mMoveToCalendarMenu = 0;
  }
  if ( mCurrentIncidence /*&& qd.isValid()*/ ) {

    if ( mCurrentIncidence->recurs() ) {
      KDateTime thisDateTime( qd, KOPrefs::instance()->timeSpec() );
      bool isLastOccurrence =
        !mCurrentIncidence->recurrence()->getNextDateTime( thisDateTime ).isValid();
      bool isFirstOccurrence =
        !mCurrentIncidence->recurrence()->getPreviousDateTime( thisDateTime ).isValid();
      mDissociateOccurrences->setEnabled(
        !( isFirstOccurrence && isLastOccurrence ) &&
        !mCurrentIncidence->isReadOnly() );
    }

    // Enable/Disabled menu items only valid for editable events.
    QList<QAction *>::Iterator it;
    for ( it = mEditOnlyItems.begin(); it != mEditOnlyItems.end(); ++it ) {
      (*it)->setEnabled( !mCurrentIncidence->isReadOnly() );
    }
    mToggleReminder->setVisible( ( mCurrentIncidence->type() != "Journal" ) );

    for ( it = mRecurrenceItems.begin(); it != mRecurrenceItems.end(); ++it ) {
      (*it)->setVisible( mCurrentIncidence->recurs() );
    }
    for ( it = mTodoOnlyItems.begin(); it != mTodoOnlyItems.end(); ++it ) {
      (*it)->setVisible( mCurrentIncidence->type() == "Todo" );
      (*it)->setEnabled( !mCurrentIncidence->isReadOnly() );
    }

    // If we can determine the resource for the selected incidence, also add
    // menu selections for moving/copying that incidence to another resource.
    if ( KOrg::StdCalendar::self()->resource( mCurrentIncidence ) ) {
      if ( hasOtherWriteableCalendars() ) {
        mCopyToCalendarMenu = buildCalendarCopyMenu();
        addMenu( mCopyToCalendarMenu );
      }
      if ( !mCurrentIncidence->isReadOnly() && hasOtherWriteableCalendars() ) {
        mMoveToCalendarMenu = buildCalendarMoveMenu();
        addMenu( mMoveToCalendarMenu );
      }
    }

    popup( QCursor::pos() );
  } else {
    kDebug() << "No event selected";
  }
}

void KOEventPopupMenu::popupShow()
{
  if ( mCurrentIncidence ) {
    emit showIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupEdit()
{
  if ( mCurrentIncidence ) {
    emit editIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::print( bool preview )
{
#ifndef KORG_NOPRINTER
  KOCoreHelper helper;
  CalPrinter printer( this, mCalendar, &helper, true );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  Incidence::List selectedIncidences;
  selectedIncidences.append( mCurrentIncidence );

  printer.print( KOrg::CalPrinterBase::Incidence,
                 mCurrentDate, mCurrentDate, selectedIncidences, preview );
#endif
}

void KOEventPopupMenu::print()
{
  print( false );
}

void KOEventPopupMenu::printPreview()
{
  print( true );
}

void KOEventPopupMenu::popupDelete()
{
  if ( mCurrentIncidence ) {
    emit deleteIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupCut()
{
  if ( mCurrentIncidence ) {
    emit cutIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupCopy()
{
  if ( mCurrentIncidence ) {
    emit copyIncidenceSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::popupPaste()
{
  emit pasteIncidenceSignal();
}

void KOEventPopupMenu::toggleAlarm()
{
  if ( mCurrentIncidence ) {
    emit toggleAlarmSignal( mCurrentIncidence );
  }
}

void KOEventPopupMenu::dissociateOccurrences()
{
  if ( mCurrentIncidence ) {
    emit dissociateOccurrencesSignal( mCurrentIncidence, mCurrentDate );
  }
}

void KOEventPopupMenu::forward()
{
  KOrg::MainWindow *w = ActionManager::findInstance( KUrl() );
  if ( !w || !mCurrentIncidence ) {
    return;
  }

  KActionCollection *ac = w->getActionCollection();
  QAction *action = ac->action( "schedule_forward" );
  if ( action ) {
    action->trigger();
  } else {
    kError() << "What happened to the schedule_forward action?";
  }
}

void KOEventPopupMenu::toggleTodoCompleted()
{
  if ( mCurrentIncidence && mCurrentIncidence->type() == "Todo" ) {
    emit toggleTodoCompletedSignal( mCurrentIncidence );
  }
}

bool KOEventPopupMenu::isResourceWritable( const ResourceCalendar *resource ) const
{
  const bool isCurrentIncidenceOwner =
      KOrg::StdCalendar::self()->resource( mCurrentIncidence ) == resource;

  // FIXME: only take into account writable subresources here
  return
    resource->isActive() &&
    !resource->readOnly() &&
    ( !isCurrentIncidenceOwner || resource->subresources().size() > 2 );
}

QMenu *KOEventPopupMenu::buildCalendarCopyMenu()
{
  QMenu *const resourceMenu = new QMenu( i18n( "C&opy to Calendar" ), this );
  resourceMenu->setIcon( KIcon( "edit-copy" ) );
  KCal::CalendarResourceManager *const manager = KOrg::StdCalendar::self()->resourceManager();
  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    const ResourceCalendar *const resource = *it;

    if ( !isResourceWritable( resource ) ) {
      continue;
    }

    QAction *const resourceAction = new QAction( resource->resourceName(), resourceMenu );
    resourceAction->setData( QVariant::fromValue( resource->identifier() ) );
    resourceMenu->addAction( resourceAction );

  }
  connect( resourceMenu, SIGNAL(triggered(QAction*)),
           this, SLOT(copyIncidenceToResource(QAction*)) );
  return resourceMenu;
}

QMenu *KOEventPopupMenu::buildCalendarMoveMenu()
{
  QMenu *const resourceMenu = new QMenu( i18n( "&Move to Calendar" ), this );
  resourceMenu->setIcon( KIcon( "go-jump" ) );
  KCal::CalendarResourceManager *const manager = KOrg::StdCalendar::self()->resourceManager();
  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    const ResourceCalendar *const resource = *it;

    if ( !isResourceWritable( resource ) ) {
      continue;
    }

    QAction *const resourceAction = new QAction( resource->resourceName(), resourceMenu );
    resourceAction->setData( QVariant::fromValue( resource->identifier() ) );
    resourceMenu->addAction( resourceAction );

  }
  connect( resourceMenu, SIGNAL(triggered(QAction*)),
           this, SLOT(moveIncidenceToResource(QAction*)) );
  return resourceMenu;
}

bool KOEventPopupMenu::hasOtherWriteableCalendars() const
{
  KCal::CalendarResourceManager *const manager = KOrg::StdCalendar::self()->resourceManager();
  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    const ResourceCalendar *const resource = *it;
    if ( isResourceWritable( resource ) ) {
      return true;
    }
  }
  return false;
}

void KOEventPopupMenu::copyIncidenceToResource( QAction *action )
{
  const QString resourceId = action->data().toString();
  if ( mCurrentIncidence && !resourceId.isEmpty() ) {
    emit copyIncidenceToResourceSignal( mCurrentIncidence, resourceId );
  }
}

void KOEventPopupMenu::moveIncidenceToResource( QAction *action )
{
  const QString resourceId = action->data().toString();
  if ( mCurrentIncidence && !resourceId.isEmpty() ) {
    emit moveIncidenceToResourceSignal( mCurrentIncidence, resourceId );
  }
}

#include "koeventpopupmenu.moc"
