/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/


#include <QDrag>
#include <QPainter>
#include <QPixmap>

#include <kdebug.h>

#include <libkdepim/kdepimprotocols.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "knhdrviewitem.h"
#include "knarticle.h"
#include "headerview.h"
#include "settings.h"


KNHdrViewItem::KNHdrViewItem( KNHeaderView *ref, KNArticle *a ) :
  QTreeWidgetItem( ref )
{
  init( a );
}


KNHdrViewItem::KNHdrViewItem( KNHdrViewItem *ref, KNArticle *a ) :
  QTreeWidgetItem( ref )
{
  init( a );
}


void KNHdrViewItem::init( KNArticle *a )
{
  art = a;
  mActive = false;
  for ( int i = 0; i < 5; ++i) // FIXME hardcoded column count
    mShowToolTip[i] = false;
}


KNHdrViewItem::~KNHdrViewItem()
{
  if (mActive) {
    QTreeWidget *lv = treeWidget();
    if (lv)
      static_cast<KNHeaderView*>( lv )->activeRemoved();
  }

  if (art) art->setListItem( 0 );
}


void KNHdrViewItem::expandChildren()
{
  QTreeWidgetItemIterator it( child(0) );

  for ( ; *it; ++it) {
    if (static_cast<KNHdrViewItem*>(*it)->depth(*it) <= depth(this))
      break;
    (*it)->setExpanded( true );
  }
}

int KNHdrViewItem::depth(QTreeWidgetItem *item) {

   int depth = 0;
   QTreeWidgetItem *QTWitem = item;

   while(QTWitem != 0){
   depth++;
   QTWitem = QTWitem->parent();
   }
   return depth;
}


int KNHdrViewItem::compare( QTreeWidgetItem *i, int col, bool ) const
{
  KNArticle *otherArticle = static_cast<KNHdrViewItem*>( i )->art;
  int diff = 0;
  time_t date1 = 0, date2 = 0;

  switch (col) {
    case 0:
    case 1:
       return text( col ).localeAwareCompare( i->text(col) );

    case 2:
       if (art->type() == KNArticle::ATremote) {
         diff = static_cast<KNRemoteArticle*>( art )->score() - static_cast<KNRemoteArticle*>( otherArticle )->score();
         return (diff < 0 ? -1 : diff > 0 ? 1 : 0);
       } else
         return 0;

    case 3:
       diff = art->lines()->numberOfLines() - otherArticle->lines()->numberOfLines();
       return (diff < 0 ? -1 : diff > 0 ? 1 : 0);

    case 4:
       date1 = art->date()->dateTime().toTime_t();
       date2 = otherArticle->date()->dateTime().toTime_t();
       if (art->type() == KNArticle::ATremote && static_cast<KNHeaderView*>( treeWidget() )->sortByThreadChangeDate()) {    //   @dg
         if (static_cast<KNRemoteArticle*>( art )->subThreadChangeDate() > date1)
           date1 = static_cast<KNRemoteArticle*>( art )->subThreadChangeDate();
         if (static_cast<KNRemoteArticle*>( otherArticle )->subThreadChangeDate() > date2)
           date2 = static_cast<KNRemoteArticle*>( otherArticle )->subThreadChangeDate();
       }
       diff = date1 - date2;
       return (diff < 0 ? -1 : diff > 0 ? 1 : 0);

    default:
       return 0;
  }
}


// int KNHdrViewItem::width( const QFontMetrics &fm, const Q3ListView *, int column ) const
int KNHdrViewItem::width( const QFontMetrics &fm, const QTreeWidget *, int column ) const       //    @dg
{
  int ret = fm.boundingRect( text(column) ).width();
  const KPaintInfo *paintInfo = static_cast<KNHeaderView*>( treeWidget() )->paintInfo();    //    @dg

  // all pixmaps are drawn in the first column
  if ( column == paintInfo->subCol ) {
    const QPixmap *pm;
    for (int i = 0; i < 4; ++i) {
//      pm = pixmap( i );
      if (pm && !pm->isNull())
        ret += pm->width() + 3;
    }
  }

  return ret;
}


QString KNHdrViewItem::text( int col ) const
{
  if ( !art )
    return QString();

  KNHeaderView *hv = static_cast<KNHeaderView*>( treeWidget() );

  if ( col == hv->paintInfo()->subCol ) {
    return art->subject()->asUnicodeString();
  }

  if ( col == hv->paintInfo()->sizeCol ) {
    if ( art->lines()->numberOfLines() != 0 ) { // invalid values are read as '0' in KNGroup::insortHeaders()
      return QString::number( art->lines()->numberOfLines() );
    } else {
      return QString();
    }
  }

  if ( col == hv->paintInfo()->scoreCol ) {
    if ( art->type() == KNArticle::ATremote )
      return QString::number( static_cast<KNRemoteArticle*>( art )->score() );
    else
      return QString();
  }

  if ( col == hv->paintInfo()->dateCol ) {
    return hv->mDateFormatter.dateString( art->date()->dateTime().toTime_t() );
  } else
  return QTreeWidgetItem::text( col );
}


QMimeData* KNHdrViewItem::dragObject()       //    @dg
{
#ifdef __GNUC__
#warning Enable this section again, once KNHdrView does not derive from K3ListView any more and can process QDrag (not Q3DragObject)
#endif
return 0;
#if 0
  QDrag *drag = new QDrag( listView()->viewport() );
  QMimeData *md = new QMimeData;
  drag->setMimeData( md );

  KUrl::List list;
  QString mid = art->messageID()->asUnicodeString();
  // for some obscure reason it returns messageid in <>s
  mid = mid.mid( 1, mid.length() - 2 );
  list.append( KDEPIMPROTOCOL_NEWSARTICLE + mid );
  QMap<QString,QString> metadata;
  metadata["labels"] = KUrl::toPercentEncoding( art->subject()->asUnicodeString() );
  list.populateMimeData( md, metadata );
  md->setData( "x-knode-drag/article" , QByteArray() );

  drag->setPixmap( knGlobals.configManager()->appearance()->icon( KNode::Appearance::posting ) );
  return drag;
#endif
}


int KNHdrViewItem::countUnreadInThread()
{
  int count = 0;
  if ( knGlobals.settings()->showUnread() ) {
    if (art->type() == KNArticle::ATremote) {
      count = static_cast<KNRemoteArticle*>( art )->unreadFollowUps();
    }
  }
  return count;
}


bool KNHdrViewItem::greyOut()
{
  if (art->type() == KNArticle::ATremote) {
    return !static_cast<KNRemoteArticle*>( art )->hasUnreadFollowUps()
        && static_cast<KNRemoteArticle*>( art )->isRead();
  } else
    return false;
}


bool KNHdrViewItem::firstColBold()
{
  if(art->type() == KNArticle::ATremote)
    return static_cast<KNRemoteArticle*>( art )->isNew();
  else
    return false;
}


QColor KNHdrViewItem::normalColor()
{
  if (art->type()==KNArticle::ATremote)
    return static_cast<KNRemoteArticle*>( art )->color();
  else
    return knGlobals.settings()->unreadThreadColor();
}


QColor KNHdrViewItem::greyColor()
{
  return knGlobals.settings()->readThreadColor();
}

