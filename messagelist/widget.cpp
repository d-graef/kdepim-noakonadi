/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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
*/

#include "widget.h"

#include <akonadi/collection.h>
#include <akonadi/item.h>
#include <akonadi/itemcopyjob.h>
#include <akonadi/itemmovejob.h>

#include "storagemodel.h"
#include "core/messageitem.h"
#include "core/view.h"

#include <QtCore/QTimer>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QDrag>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>

#include <KDE/KDebug>
#include <KDE/KIcon>
#include <KDE/KIconLoader>
#include <KDE/KLocale>
#include <KDE/KMenu>

namespace MessageList
{

class Widget::Private
{
public:
  Private( Widget *owner )
    : q( owner ) { }

  Akonadi::Item::List selectionAsItems() const;
  Akonadi::Item itemForRow( int row ) const;

  Widget * const q;

  int mLastSelectedMessage;
};

} // namespace MessageList

using namespace MessageList;
using namespace Akonadi;

struct DragPayload
{
  QList<Collection::Id> sourceCollections;
  QList<Item::Id> items;
  bool readOnly;

  DragPayload() : readOnly( false ) { }

  static QString mimeType()
  {
    return "x-akonadi-drag/message-list";
  }
};

QDataStream &operator<<(QDataStream &stream, const DragPayload &payload)
{
  stream << payload.sourceCollections;
  stream << payload.items;
  stream << payload.readOnly;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, DragPayload &payload)
{
  stream >> payload.sourceCollections;
  stream >> payload.items;
  stream >> payload.readOnly;

  return stream;
}

Widget::Widget( QWidget *parent )
  : Core::Widget( parent ), d( new Private( this ) )
{
  QTimer::singleShot( 0, this, SLOT( populateStatusFilterCombo() ) );
}

Widget::~Widget()
{
}

bool Widget::canAcceptDrag( const QDragMoveEvent * e )
{
  Collection::List collections = static_cast<const StorageModel*>( storageModel() )->displayedCollections();

  if ( collections.size()!=1 )
    return false; // no folder here or too many (in case we can't decide where the drop will end)

  Collection c = collections.first();

  if ( ( c.rights() & Collection::CanCreateItem ) == 0 )
    return false; // no way to drag into

  if ( !e->mimeData()->hasFormat( DragPayload::mimeType() ) )
    return false; // no way to decode it

  DragPayload payload;
  QDataStream stream( e->mimeData()->data( DragPayload::mimeType() ) );
  stream >> payload;

  foreach ( Collection::Id id, payload.sourceCollections ) {
    if ( id == c.id() ) {
      return false;
    }
  }

  return true;
}

bool Widget::selectNextMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter,
                                    MessageList::Core::ExistingSelectionBehaviour existingSelectionBehaviour,
                                    bool centerItem,
                                    bool loop )
{
  return view()->selectNextMessageItem( messageTypeFilter, existingSelectionBehaviour, centerItem, loop );
}

bool Widget::selectPreviousMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter,
                                        MessageList::Core::ExistingSelectionBehaviour existingSelectionBehaviour,
                                        bool centerItem,
                                        bool loop )
{
  return view()->selectPreviousMessageItem( messageTypeFilter, existingSelectionBehaviour, centerItem, loop );
}

bool Widget::focusNextMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem, bool loop )
{
  return view()->focusNextMessageItem( messageTypeFilter, centerItem, loop );
}

bool Widget::focusPreviousMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem, bool loop )
{
  return view()->focusNextMessageItem( messageTypeFilter, centerItem, loop );
}

void Widget::selectFocusedMessageItem( bool centerItem )
{
  view()->selectFocusedMessageItem( centerItem );
}

bool Widget::selectFirstMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem )
{
  return view()->selectFirstMessageItem( messageTypeFilter, centerItem );
}

void Widget::selectAll()
{
  view()->setAllGroupsExpanded( true );
  view()->selectAll();
}

void Widget::viewMessageSelected( MessageList::Core::MessageItem *msg )
{
  int row = -1;
  if ( msg ) {
    row = msg->currentModelIndexRow();
  }

  if ( !msg || !msg->isValid() || !storageModel() ) {
    d->mLastSelectedMessage = -1;
    emit messageSelected( Item() );
    return;
  }

  Q_ASSERT( row >= 0 );

  d->mLastSelectedMessage = row;

  emit messageSelected( d->itemForRow( row ) ); // this MAY be null
}

void Widget::viewMessageActivated( MessageList::Core::MessageItem *msg )
{
  Q_ASSERT( msg ); // must not be null
  Q_ASSERT( storageModel() );

  if ( !msg->isValid() ) {
    return;
  }

  int row = msg->currentModelIndexRow();
  Q_ASSERT( row >= 0 );

  // The assert below may fail when quickly opening and closing a non-selected thread.
  // This will actually activate the item without selecting it...
  //Q_ASSERT( d->mLastSelectedMessage == row );

  if ( d->mLastSelectedMessage != row ) {
    // Very ugly. We are activating a non selected message.
    // This is very likely a double click on the plus sign near a thread leader.
    // Dealing with mLastSelectedMessage here would be expensive: it would involve releasing the last selected,
    // emitting signals, handling recursion... ugly.
    // We choose a very simple solution: double clicking on the plus sign near a thread leader does
    // NOT activate the message (i.e open it in a toplevel window) if it isn't previously selected.
    return;
  }

  emit messageActivated( d->itemForRow( row ) ); // this MAY be null
}

void Widget::viewSelectionChanged()
{
  emit selectionChanged();
  if ( !currentMessageItem() ) {
    emit messageSelected( Item() );
  }
}

void Widget::viewMessageListContextPopupRequest( const QList< MessageList::Core::MessageItem * > &selectedItems, const QPoint &globalPos )
{
  //FIXME: To implement once the other infrastructure is ready for a KMail independent implementation
  kWarning() << "Needs to be reimplemented";
}

void Widget::viewMessageStatusChangeRequest( MessageList::Core::MessageItem *msg, const KPIM::MessageStatus &set, const KPIM::MessageStatus &clear )
{
  Q_ASSERT( msg ); // must not be null
  Q_ASSERT( storageModel() );

  if ( !msg->isValid() ) {
    return;
  }

  int row = msg->currentModelIndexRow();
  Q_ASSERT( row >= 0 );

  Item item = d->itemForRow( row );
  Q_ASSERT( item.isValid() );

  emit messageStatusChangeRequest( item, set, clear );
}

void Widget::viewGroupHeaderContextPopupRequest( MessageList::Core::GroupHeaderItem *ghi, const QPoint &globalPos )
{
  Q_UNUSED( ghi );

  KMenu menu( this );

  QAction *act;

  menu.addSeparator();

  act = menu.addAction( i18n( "Expand All Groups" ) );
  connect( act, SIGNAL( triggered( bool ) ),
           view(), SLOT( slotExpandAllGroups() ) );

  act = menu.addAction( i18n( "Collapse All Groups" ) );
  connect( act, SIGNAL( triggered( bool ) ),
           view(), SLOT( slotCollapseAllGroups() ) );

  menu.exec( globalPos );
}

void Widget::viewDragEnterEvent( QDragEnterEvent *e )
{
  if ( !canAcceptDrag( e ) ) {
    e->ignore();
    return;
  }

  e->accept();
}

void Widget::viewDragMoveEvent( QDragMoveEvent *e )
{
  if ( !canAcceptDrag( e ) ) {
    e->ignore();
    return;
  }

  e->accept();
}

enum DragMode
{
  DragCopy,
  DragMove,
  DragCancel
};

void Widget::viewDropEvent( QDropEvent *e )
{
  Collection::List collections = static_cast<const StorageModel*>( storageModel() )->displayedCollections();

  if ( collections.size()!=1 || !e->mimeData()->hasFormat( DragPayload::mimeType() ) ) {
    // no folder here or too many (in case we can't decide where the drop will end), or we can't decode
    e->ignore();
    return;
  }

  DragPayload payload;
  QDataStream stream( e->mimeData()->data( DragPayload::mimeType() ) );
  stream >> payload;

  if ( payload.items.isEmpty() ) {
    kWarning() << "Could not decode drag data!";
    e->ignore();
    return;
  }

  e->accept();

  int action;
  if ( payload.readOnly ) {
    action = DragCopy;
  } else {
    action = DragCancel;
    int keybstate = QApplication::keyboardModifiers();
    if ( keybstate & Qt::CTRL ) {
      action = DragCopy;

    } else if ( keybstate & Qt::SHIFT ) {
      action = DragMove;

    } else {
      // FIXME: This code is duplicated almost exactly in FolderView... shouldn't we share ?
      KMenu menu;
      QAction *moveAction = menu.addAction( KIcon( "go-jump"), i18n( "&Move Here" ) );
      QAction *copyAction = menu.addAction( KIcon( "edit-copy" ), i18n( "&Copy Here" ) );
      menu.addSeparator();
      menu.addAction( KIcon( "dialog-cancel" ), i18n( "C&ancel" ) );

      QAction *menuChoice = menu.exec( QCursor::pos() );
      if ( menuChoice == moveAction ) {
        action = DragMove;
      } else if ( menuChoice == copyAction ) {
        action = DragCopy;
      } else {
        action = DragCancel;
      }
    }
  }

  Collection target = collections.first();
  Item::List items;
  foreach ( Item::Id id, payload.items ) {
    items << Item( id );
  }

  if ( action == DragCopy ) {
    new ItemCopyJob( items, target, this );
  } else if ( action == DragMove ) {
    new ItemMoveJob( items, target, this );
  }
}


void Widget::viewStartDragRequest()
{
  Collection::List collections = static_cast<const StorageModel*>( storageModel() )->displayedCollections();

  if ( collections.isEmpty() )
    return; // no folder here

  QList<Item> items = d->selectionAsItems();
  if ( items.isEmpty() )
    return;

  DragPayload payload;

  foreach ( const Collection c, collections ) {
    payload.sourceCollections << c.id();
    // We won't be able to remove items from this collection
    if ( ( c.rights() & Collection::CanDeleteItem ) == 0 ) {
      // So the drag will be read-only
      payload.readOnly = true;
    }
  }

  foreach ( const Item i, items ) {
    payload.items << i.id();
  }

  QByteArray data;
  {
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << payload;
  }

  QMimeData *mimeData = new QMimeData;
  mimeData->setData( DragPayload::mimeType(), data );

  QDrag *drag = new QDrag( view()->viewport() );
  drag->setMimeData( mimeData );

  // Set pixmap
  QPixmap pixmap;
  if( items.size() == 1 ) {
    pixmap = QPixmap( DesktopIcon("mail-message", KIconLoader::SizeSmall) );
  } else {
    pixmap = QPixmap( DesktopIcon("document-multiple", KIconLoader::SizeSmall) );
  }

  // Calculate hotspot (as in Konqueror)
  if( !pixmap.isNull() ) {
    drag->setHotSpot( QPoint( pixmap.width() / 2, pixmap.height() / 2 ) );
    drag->setPixmap( pixmap );
  }

  if ( payload.readOnly )
    drag->exec( Qt::CopyAction );
  else
    drag->exec( Qt::CopyAction | Qt::MoveAction );
}

Item::List Widget::Private::selectionAsItems() const
{
  Item::List res;
  QList<Core::MessageItem *> selection = q->view()->selectionAsMessageItemList();

  foreach ( Core::MessageItem *mi, selection ) {
    Item i = itemForRow( mi->currentModelIndexRow() );
    Q_ASSERT( i.isValid() );
    res << i;
  }

  return res;
}

Item Widget::Private::itemForRow( int row ) const
{
  return static_cast<const StorageModel*>( q->storageModel() )->itemForRow( row );
}
