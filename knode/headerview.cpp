/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <QCursor>
#include <QTimer>
#include <QHeaderView>

//Added by qt3to4:
#include <QKeyEvent>
#include <QEvent>
#include <QMouseEvent>

#include <klocale.h>
#include <kdebug.h>
#include <kmenu.h>

#include "knglobals.h"
#include "headerview.h"
#include "knhdrviewitem.h"
#include "kngroupmanager.h"
#include "knarticle.h"
#include "knarticlemanager.h"
#include "knmainwidget.h"
#include "settings.h"


KNHeaderView::KNHeaderView( QWidget *parent ) :
  QTreeWidget(parent),
  mSortCol( -1 ),
  mSortAsc( true ),
  mSortByThreadChangeDate( false ),
  mDelayedCenter( -1 ),
  mActiveItem( 0 ),
  mShowingFolder( false ),
  mInitDone( false )
{
  setColumnCount(5);
  QStringList header_view_columns;
  header_view_columns << "Subject" << "From" << "Score" << "Lines" << "Date";
  setHeaderLabels(header_view_columns);
  setColumnWidth(0, 400);
  setColumnWidth(1, 200);

  mPaintInfo.subCol    = 0;   //   subject
  mPaintInfo.senderCol = 1;   //   from
  mPaintInfo.scoreCol  = 2;   //   score
  mPaintInfo.sizeCol   = 3;   //   lines
  mPaintInfo.dateCol   = 4;   //   date

//  setDropVisualizer( false );
//  setDropHighlighter( false );
//  setItemsRenameable( false );
//  setItemsMovable( false );
  setAcceptDrops( false );
  setDragEnabled( true );
  setAllColumnsShowFocus( true );
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setSortingEnabled(true);
//  setShadeSortColumn ( true );
  setRootIsDecorated( true );
  setSorting( mPaintInfo.dateCol );
//  header()->setMovingEnabled( true );

  // due to our own column text squeezing we need to repaint on column resizing
//  disconnect( header(), SIGNAL(sizeChange(int, int, int)) );
//  connect( header(), SIGNAL(sizeChange(int, int, int)),
//           SLOT(slotSizeChanged(int, int, int)) );
  
// column selection RMB menu
  mPopup = new QMenu( this );
  mPopup->setTitle( i18n("View Columns") );
  KnHwPmenuSize = mPopup->addAction( i18n("Line Count") );
  KnHwPmenuSize->setCheckable(true);
  KnHwPmenuScore = mPopup->addAction( i18n("Score") );
  KnHwPmenuScore->setCheckable(true);

  QHeaderView *columnHeader = header();
  columnHeader->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(columnHeader, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotMPopup(const QPoint&)));
  
  connect( KnHwPmenuSize, SIGNAL(toggled(bool)), this, SLOT(toggleColumnLines(bool)) );
  connect( KnHwPmenuScore, SIGNAL(toggled(bool)), this, SLOT(toggleColumnScore(bool)) );

  // connect to the article manager
  connect( knGlobals.articleManager(), SIGNAL(aboutToShowGroup()), SLOT(prepareForGroup()) );
  connect( knGlobals.articleManager(), SIGNAL(aboutToShowFolder()), SLOT(prepareForFolder()) );

#ifdef __GNUC__
#warning Port me!
#endif
//   new KNHeaderViewToolTip( this );

  installEventFilter( this );
}


KNHeaderView::~KNHeaderView()
{
  // ### crash because KNConfigManager is already deleted here
  // writeConfig();
}

void KNHeaderView::readConfig()
{

  if ( !mInitDone ) {
    KConfigGroup conf(knGlobals.config(), "HeaderView" );
    mSortByThreadChangeDate = conf.readEntry( "sortByThreadChangeDate", false );
//    restoreLayout( knGlobals.config(), "HeaderView" );
    mInitDone = true;
  }

  toggleColumnLines( knGlobals.settings()->showLines() );
  if ( !mShowingFolder ) // score column is always hidden when showing a folder
    toggleColumnScore( knGlobals.settings()->showScore() );

  mDateFormatter.setCustomFormat( knGlobals.settings()->customDateFormat() );
  mDateFormatter.setFormat( knGlobals.settings()->dateFormat() );

  QPalette p = palette();
  p.setColor( QPalette::Base, knGlobals.settings()->backgroundColor() );
  p.setColor( QPalette::Text, knGlobals.settings()->textColor() );
  setPalette( p );
//  setAlternateBackground( knGlobals.settings()->alternateBackgroundColor() );    //  function from K3ListView
  setFont( knGlobals.settings()->articleListFont() );

}


void KNHeaderView::writeConfigShowLines()
{
  KConfigGroup conf(knGlobals.config(), "HeaderView" );
  conf.writeEntry( "sortByThreadChangeDate", mSortByThreadChangeDate );
//  saveLayout( knGlobals.config(), "HeaderView" );        //    saveLayout() function from K3ListView class
  knGlobals.settings()->setShowLines( mPaintInfo.showLines );
}


void KNHeaderView::writeConfigShowScore()
{
  KConfigGroup conf(knGlobals.config(), "HeaderView" );
  conf.writeEntry( "sortByThreadChangeDate", mSortByThreadChangeDate );
//  saveLayout( knGlobals.config(), "HeaderView" );        //      saveLayout() function from K3ListView class

  if ( !mShowingFolder ) // score column is always hidden when showing a folder
    knGlobals.settings()->setShowScore( mPaintInfo.showScore );
}


void KNHeaderView::setActive( QTreeWidgetItem *i )
{

  KNHdrViewItem *item = static_cast<KNHdrViewItem*>( i );

  if ( !item || item->isActive() )
    return;

  if ( mActiveItem ) {
    mActiveItem->setActive( false );
//    repaintItem( mActiveItem );
    mActiveItem = 0;
  }

  item->setActive( true );
//  setSelected( item, true );
  setCurrentItem( i );
  ensureItemVisibleWithMargin( i );
  mActiveItem = item;
  emit( itemSelected(item) );
}


void KNHeaderView::clear()
{
  mActiveItem = 0;
  QTreeWidget::clear();
}


void KNHeaderView::ensureItemVisibleWithMargin( const QTreeWidgetItem *i )
{
  if ( !i )
    return;

  QTreeWidgetItem *parent = i->parent();
  while ( parent ) {
    if ( !parent->isExpanded() )
      parent->setExpanded( true );
    parent = parent->parent();
  }
/*
  mDelayedCenter = -1;
  int y = itemPos( i );
  int h = i->height();

  if ( knGlobals.settings()->smartScrolling() &&
      ((y + h + 5) >= (contentsY() + visibleHeight()) ||
       (y - 5 < contentsY())) )
  {
    ensureVisible( contentsX(), y + h/2, 0, h/2 );
    mDelayedCenter = y + h/2;
    QTimer::singleShot( 300, this, SLOT(slotCenterDelayed()) );
  } else {
    ensureVisible( contentsX(), y + h/2, 0, h/2 );
  }
*/
}


void KNHeaderView::slotCenterDelayed()
{
  /*     // Qt3 -> Qt4 fixme
  if ( mDelayedCenter != -1 )
    ensureVisible( contentsX(), mDelayedCenter, 0, visibleHeight() / 2 );
  */
}


void KNHeaderView::setSorting( int column, bool ascending )
{
  if ( column == mSortCol ) {
    mSortAsc = ascending;
    if ( mInitDone && column == mPaintInfo.dateCol && ascending )
      mSortByThreadChangeDate = !mSortByThreadChangeDate;
  } else {
    mSortCol = column;
    emit sortingChanged( column );
  }

  sortByColumn( column, Qt::DescendingOrder );
/*      // Qt3 -> Qt4 fixme
  if ( currentItem() )
    ensureItemVisible( currentItem() );

  if ( mSortByThreadChangeDate )
    setHeaderLabel( mPaintInfo.dateCol , i18n("Date (thread changed)") );
  else
    setHeaderLabel( mPaintInfo.dateCol, i18n("Date") );
*/
}


void KNHeaderView::nextArticle()
{
  KNHdrViewItem *it = static_cast<KNHdrViewItem*>( currentItem() );
  QTreeWidget *headerView = it->treeWidget();

  if (it) {
    if (it->isActive()) {  // take current article, if not selected
      if (it->childCount() > 0 )
        it->setExpanded(true);
      it = static_cast<KNHdrViewItem*>(headerView->itemBelow(it));
    }
  } else
    it = static_cast<KNHdrViewItem*>( topLevelItem(0) );

  if(it) {
    clearSelection();
    setActive( it );
//    setSelectionAnchor( currentItem() );
  }
}


void KNHeaderView::prevArticle()
{
  KNHdrViewItem *it = static_cast<KNHdrViewItem*>( currentItem() );
  QTreeWidget *headerView = it->treeWidget();

  if (it && it->isActive()) {  // take current article, if not selected
    it = static_cast<KNHdrViewItem*>(headerView->itemAbove(it));
    clearSelection();
    setActive( it );
//    setSelectionAnchor( currentItem() );
  }
//  */
}


void KNHeaderView::incCurrentArticle()
{
  QTreeWidgetItem *lvi = currentItem();
  QTreeWidget *headerView = lvi->treeWidget();

  if ( lvi && (lvi->childCount() > 0) )
    lvi->setExpanded( true );
  if ( lvi && headerView->itemBelow(lvi) ) {
    setCurrentItem( headerView->itemBelow(lvi) );
//    ensureItemVisible( currentItem() );
    setFocus();
  }
}

void KNHeaderView::decCurrentArticle()
{
  QTreeWidgetItem *lvi = currentItem();
  QTreeWidget *headerView = lvi->treeWidget();

  if ( lvi && headerView->itemAbove(lvi) ) {
    if ( headerView->itemAbove(lvi)->childCount() > 0)
      headerView->itemAbove(lvi)->setExpanded( true );
    setCurrentItem( headerView->itemAbove(lvi) );
//    ensureItemVisible( currentItem() );
    setFocus();
  }
}


void KNHeaderView::selectCurrentArticle()
{
  clearSelection();
  setActive( currentItem() );
}


bool KNHeaderView::nextUnreadArticle()
{
  if ( !knGlobals.groupManager()->currentGroup() )
    return false;

  KNHdrViewItem *next, *current;
  KNRemoteArticle *art;

  current = static_cast<KNHdrViewItem*>( currentItem() );
  if ( !current )
    current = static_cast<KNHdrViewItem*>( topLevelItem(0) );

  QTreeWidget *headerView = current->treeWidget();

  if(!current)
    return false;

  art = static_cast<KNRemoteArticle*>( current->art );

  if ( !current->isActive() && !art->isRead() ) // take current article, if unread & not selected
    next = current;
  else {
    if ( (current->childCount() > 0) && art->hasUnreadFollowUps() && !current->isExpanded() )
      current->setExpanded(true);
    next = static_cast<KNHdrViewItem*>( headerView->itemBelow(current) );
  }

  while ( next ) {
    art = static_cast<KNRemoteArticle*>( next->art );
    if ( !art->isRead() )
      break;
    else {
      if ( (next->childCount() > 0) && art->hasUnreadFollowUps() && !next->isExpanded() )
        next->setExpanded(true);
      next = static_cast<KNHdrViewItem*>( headerView->itemBelow(next) );
    }
  }

  if ( next ) {
    clearSelection();
    setActive( next );
//    setSelectionAnchor( currentItem() );
    return true;
  }
  return false;
}


bool KNHeaderView::nextUnreadThread()
{
  KNHdrViewItem *next, *current;
  KNRemoteArticle *art;

  if ( !knGlobals.groupManager()->currentGroup() )
    return false;

  current = static_cast<KNHdrViewItem*>( currentItem() );
  if ( !current )
    current = static_cast<KNHdrViewItem*>( topLevelItem(0) );

  if ( !current )
    return false;
  QTreeWidget *headerView = current->treeWidget();

  art = static_cast<KNRemoteArticle*>( current->art );

  int depth = 0;

  while(current != 0){
   depth++;
//   item = item->parent();
  }

//  if ( current->depth() == 0 && !current->isActive() && (!art->isRead() || art->hasUnreadFollowUps()) )
  if ( depth == 0 && !current->isActive() && (!art->isRead() || art->hasUnreadFollowUps()) )
    next = current; // take current article, if unread & not selected
  else

    next = static_cast<KNHdrViewItem*>( headerView->itemBelow(current) );

  while ( next ) {
    art = static_cast<KNRemoteArticle*>( next->art );
    depth = 0;
  while(next != 0){
   depth++;
//   item = item->parent();
  }

    if ( depth == 0 ) {
      if ( !art->isRead() || art->hasUnreadFollowUps() )
        break;
    }
    next = static_cast<KNHdrViewItem*>( headerView->itemBelow(next) );
  }

  if ( next ) {
    setCurrentItem( next );
    if ( art->isRead() )
      nextUnreadArticle();
    else {
      clearSelection();
      setActive( next );
//      setSelectionAnchor( currentItem() );
    }
    return true;
  }
  return false;
}

void KNHeaderView::slotMPopup(const QPoint &point)
{
  // Handle global position
  QPoint globalPos = mapToGlobal(point);
  // Show context menu at handling position
  mPopup->exec(globalPos);
}

void KNHeaderView::toggleColumnLines(bool mode)
{
  int  *col  = 0;
  int  width = 42;

  mPaintInfo.showLines = mode;
  col   = &mPaintInfo.sizeCol;

  KnHwPmenuSize->setChecked(mode);

  if (mode) {
    showColumn( *col );
    setColumnWidth( *col, width );
  }
  else {
//    header()->setResizeEnabled( false, *col );
//    header()->setStretchEnabled( false, *col );
    hideColumn( *col );
  }
    writeConfigShowLines();
}

void KNHeaderView::toggleColumnScore(bool mode)
{
  int  *col  = 0;
  int  width = 42;

  mPaintInfo.showScore = mode;
  col   = &mPaintInfo.scoreCol;

  KnHwPmenuScore->setChecked(mode);

  if (mode) {
//    header()->setResizeEnabled( true, *col );
    showColumn( *col );
    header()->setResizeMode(*col, QHeaderView::Interactive);
    setColumnWidth( *col, width );
  }
  else {
//  header()->setResizeEnabled( false, *col );
    header()->setResizeMode(*col, QHeaderView::Fixed);
//    header()->setStretchEnabled( false, *col );
//   setStretchLastSection(false);
    hideColumn( *col );
  }
    writeConfigShowScore();
}

void KNHeaderView::prepareForGroup()
{
  mShowingFolder = false;
//  header()->setLabel( mPaintInfo.senderCol, i18n("From") );
//  header()->setHeaderLabel( i18n("From") );
  toggleColumnScore( knGlobals.settings()->showScore() );
}


void KNHeaderView::prepareForFolder()
{
  mShowingFolder = true;
//  header()->setLabel( mPaintInfo.senderCol, i18n("Newsgroups / To") );
  toggleColumnScore( false ); // local folders have no score
}


bool KNHeaderView::event( QEvent *e )
{
  // we don't want to have the alternate list background restored
  // to the system defaults!
  if (e->type() == QEvent::ApplicationPaletteChange)
    return QTreeWidget::event(e);
  else
    return QTreeWidget::event(e);
}

void KNHeaderView::contentsMousePressEvent( QMouseEvent *e )
{
  if (!e) return;

  bool selectMode=(( e->modifiers() & Qt::ShiftModifier ) || ( e->modifiers() & Qt::ControlModifier ));

//  QPoint vp = contentsToViewport(e->pos());
//  QPoint vp = viewportEvent(e->pos());

//  Q3ListViewItem *i = itemAt(vp);
//  QTreeWidgetItem *i = itemAt(vp);

  QTreeWidget::mousePressEvent( e );
/*
  if ( i ) {
    int decoLeft = header()->sectionPos( 0 ) +
        treeStepSize() * ( (i->depth() - 1) + ( rootIsDecorated() ? 1 : 0) );
    int decoRight = qMin( decoLeft + treeStepSize() + itemMargin(),
        header()->sectionPos( 0 ) + header()->sectionSize( 0 ) );
    bool rootDecoClicked = vp.x() > decoLeft && vp.x() < decoRight;

    if( !selectMode && i->isSelected() && !rootDecoClicked )
      setActive( i );
  }
  */
}


void KNHeaderView::contentsMouseDoubleClickEvent( QMouseEvent *e )
{
  if (!e) return;

/*
  QTreeWidgetItem *i = itemAt( contentsToViewport(e->pos()) );

  if (i) {
    emit doubleClick( i );
    return;
  }
*/
  contentsMouseDoubleClickEvent(e);
}


void KNHeaderView::keyPressEvent(QKeyEvent *e)
{
  if (!e) return;

  QTreeWidgetItem *i = currentItem();

  switch(e->key()) {
    case Qt::Key_Space:
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
      e->ignore(); // don't eat them
    break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      setActive( i );
    break;

    default:
      keyPressEvent(e);
  }
}


#ifdef __GNUC__
#warning Port this to QDrag once the view doesnot derive from K3ListView any more
#endif

QMimeData* KNHeaderView::dragObject()
{
  KNHdrViewItem *item = static_cast<KNHdrViewItem*>( itemAt(viewport()->mapFromGlobal(QCursor::pos())) );
  if (item)
    return item->dragObject();
  else
    return 0;
}


void KNHeaderView::slotSizeChanged( int section, int, int newSize )
{
/* //  Qt3 -> Qt4
  viewport()->repaint( header()->sectionPos(section), 0, newSize, visibleHeight() );
 */
}


bool KNHeaderView::eventFilter(QObject *o, QEvent *e)
{
  // right click on header
  if ( e->type() == QEvent::MouseButtonPress &&
       static_cast<QMouseEvent*>(e)->button() == Qt::RightButton &&
       qobject_cast<QHeaderView*>( o ) )
  {
    mPopup->popup( static_cast<QMouseEvent*>(e)->globalPos() );
    return true;
  }
  return QTreeWidget::eventFilter(o, e);
}


void KNHeaderView::resetCurrentTime()
{
  mDateFormatter.reset();
  QTimer::singleShot( 1000, this, SLOT(resetCurrentTime()) );
}


//BEGIN: KNHeaderViewToolTip ==================================================

#ifdef __GNUC__
#warning Port me!
#endif
#if 0
KNHeaderViewToolTip::KNHeaderViewToolTip( KNHeaderView *parent ) :
  QToolTip( parent->viewport() ),
  listView( parent )
{
}


void KNHeaderViewToolTip::maybeTip( const QPoint &p )
{
  const KNHdrViewItem *item = static_cast<KNHdrViewItem*>( listView->itemAt( p ) );
  if ( !item )
    return;
  const int column = listView->header()->sectionAt( p.x() );
  if ( column == -1 )
    return;

  if ( !item->showToolTip( column ) )
    return;

  const QRect itemRect = listView->itemRect( item );
  if ( !itemRect.isValid() )
    return;
  const QRect headerRect = listView->header()->sectionRect( column );
  if ( !headerRect.isValid() )
    return;

  tip( QRect( headerRect.left(), itemRect.top(), headerRect.width(), itemRect.height() ),
       Qt::escape( item->text( column ) ) );
}
#endif

//END: KNHeaderViewToolTip ====================================================

#include "headerview.moc"
