/* -*- Mode: C++ -*-
   $Id$
   KDGantt - a multi-platform charting engine
*/

/****************************************************************************
 ** Copyright (C)  2002-2004 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KDGantt library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 **
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** As a special exception, permission is given to link this program
 ** with any edition of Qt, and distribute the resulting executable,
 ** without including the source code for Qt in the source distribution.
 **
 **********************************************************************/


#include "KDGanttViewSubwidgets.h"
#include "KDGanttViewTaskItem.h"
#include "KDGanttViewSummaryItem.h"
#include "KDGanttViewEventItem.h"
#if QT_VERSION < 0x040000
#include "itemAttributeDialog.h"
#endif
#include <QtDebug>

#include "qpainter.h"

#include "KDGanttXMLTools.h"


/*!
  \class KDGanttViewItem KDGanttViewItem.h
  This class represents an item in a Gantt chart.

  This class is an abstract base class, it cannot be instantiated
  directly. Instead, you should create items of one of the
  subclasses. This class provides methods common to all Gantt items.

  The initialization of the shapes/colors of the item works as follows:

  Shapes:
  When a new item is created, the shapes are set to the default values
  for items of the type of this item, defined in the KDGanttView class with
  void setShapes( KDGanttViewItem::Type type,
  KDGanttViewItem::Shape start,
  KDGanttViewItem::Shape middle,
  KDGanttViewItem::Shape end );
  If there is no default value defined for this type,
  the shapes are set as follows:
  For TaskViewItems all three shapes are set to Square.
  For SummaryViewItems all three shapes are set to TriangleDown.
  For EventViewItems all three shapes are set to Diamond.

  Colors:
  When a new item is created, the colors are set to the default values
  for items of the type of this item, defined in the KDGanttView class with
  void setColors( KDGanttViewItem::Type type,
  const QColor& start,
  const QColor& middle,
  const QColor& end );
  If there is no default value defined for this type,
  the colors of the shapes are set to the default color for items of this type,
  defined in the KDGanttView class with:
  void setDefaultColor( KDGanttViewItem::Type type, const QColor& );
  The initial default color in the KDGanttView class is set to
  blue for KDGanttViewItem::Event,
  green for KDGanttViewItem::Task,
  cyan for KDGanttViewItem::Summary.

  Highlight Colors:
  When a new item is created, the highlight colors are set to the default
  values for items of the type of this item,
  defined in the KDGanttView class with:
  void setHighlightColors( KDGanttViewItem::Type type,
  const QColor& start,
  const QColor& middle,
  const QColor& end );
  If there is no default value defined for this type,
  the highlight colors of the shapes are set to the default color for
  items of this type, defined in the KDGanttView class with:
  void setDefaultHighlightColor( KDGanttViewItem::Type type, const QColor& );
  The initial default highlight color in the KDGanttView class is set to red
  for all types.

  Start/End time:
  When a new item is created, the start time and the end time is set
  automatically. The time, which is currently displayed in the middle
  of the Gantt View, is set as start/end time. At startup of a newly
  created Gantt view, this is the current time.

  The Priority:
  The priority is set with \a setPriority().
  The priority determines which items are painted over which other items.
  The item with the highest priority is painted on top of all others.
  The priority for an item can be between 1 and 199.
  A priority less than 100 means that the item is painted below the
  grid in the Gantt chart.
  For Task items, the default priority is 50, for all other items it is 150.
  This feature only makes sense for an item which is a child of another item,
  for which \a displaySubitemsAsGroup() property is set to true.

  The Display Mode:
  The display mode is set with \a setDisplaySubitemsAsGroup().
  In the normal view mode (set with setDisplaySubitemsAsGroup( false ); ),
  an item is displayed in the same manner, when its child items are
  shown or not.
  In the other mode (set with setDisplaySubitemsAsGroup( true ); ),
  called "calendar mode", the item is displayed as follows:
  If the item has no children, it is displayed as usual.
  If the item is opened (i.e., its children are displayed), the
  start/end time of this item is computed automatically according to
  the earliest start time/latest end time of its children. The item
  and its children are displayed as usual.
  If the item is closed (i.e., its children are hidden in the left
  list view), the item itself is hidden, and its children are displayed
  on the timeline of this item instead. To control the painting of
  overlapping children, call \a setPriority() for the children.

  Blocking of user interaction to open item:
  If you want to block users to open items used as parents of calendar items,
  call \a KDGanttView::setCalendarMode( true );

  Example 1, Color:
  If you create an instance of a KDGanttView class and add a SummaryViewItem
  without setting any color/shape values, you get an item with three shapes
  of the form TriangleDown in the color magenta. If the item is highlighted,
  the color will change to the highlight color red.

  Example 2, Calender View:
  To use a Gantt view as a calendar view, call
  \a KDGanttView::setCalendarMode( true );
  \a KDGanttView::setDisplaySubitemsAsGroup( true );
  Insert root items in the Gantt view.
  Insert items as children of these root item in the Gantt view.
  You may use any item type as parent and child; there are no limitations.
  It is, however, recommended to use KDGanttViewTaskItems
  Actually, you may add child items to the children themselves.
  Such a child behaves then like a parent.
  Now set the start/end time of the children to specify a time
  interval for these items.
*/


QDict<KDGanttViewItem> KDGanttViewItem::sItemDict;

/*!
  Constructs an empty Gantt item.

  \param type the type of the item to insert
  \param view the Gantt view to insert this item into
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/
KDGanttViewItem::KDGanttViewItem( Type type, KDGanttView* view,
                                  const QString& lvtext,
                                  const QString& name ) :
    QListViewItem(view->myListView,lvtext)
{
    initColorAndShapes(type);
    generateAndInsertName( name );

}


/*!
  Constructs an empty Gantt item.

  \param type the type of the item to insert
  \param parentItem a parent item under which this one goes
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/

KDGanttViewItem::KDGanttViewItem( Type type, KDGanttViewItem* parentItem,
                                  const QString& lvtext,
                                  const QString& name ) :
    QListViewItem(parentItem,lvtext)
{
    initColorAndShapes(type);
    generateAndInsertName( name );
}


/*!
  Constructs an empty Gantt item.

  \param type the type of the item to insert
  \param view the Gantt view to insert this item into
  \param after another item at the same level behind which this one should go
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/

KDGanttViewItem::KDGanttViewItem( Type type, KDGanttView* view,
                                  KDGanttViewItem* after,
                                  const QString& lvtext,
                                  const QString& name ) :
    QListViewItem(view->myListView,after, lvtext)
{
    initColorAndShapes(type);
    generateAndInsertName( name );
}


/*!
  Constructs an empty Gantt item.

  \param type the type of the item to insert
  \param parentItem a parent item under which this one goes
  \param after another item at the same level behind which this one should go
  \param lvtext the text to show in the list view
  \param name the name by which the item can be identified. If no name
  is specified, a unique name will be generated
*/

KDGanttViewItem::KDGanttViewItem( Type type, KDGanttViewItem* parentItem,
                                  KDGanttViewItem* after,
                                  const QString& lvtext,
                                  const QString& name ) :
    QListViewItem( parentItem, after, lvtext )
{
    initColorAndShapes(type);
    generateAndInsertName( name );
}


/*!
  Destroys the object and frees any allocated resources.
*/
KDGanttViewItem::~KDGanttViewItem()
{
  myGanttView->itemAboutToBeDeleted( this );
  if ( startLine ) delete startLine;
  if ( endLine ) delete endLine  ;
  if ( startLineBack ) delete startLineBack  ;
  if ( endLineBack ) delete  endLineBack ;
  if ( actualEnd ) delete actualEnd  ;
  if ( mTextCanvas  ) delete mTextCanvas   ;
  if ( startShape ) delete  startShape ;
  if ( midShape ) delete midShape  ;
  if ( endShape ) delete endShape  ;
  if ( startShapeBack ) delete startShapeBack  ;
  if ( midShapeBack ) delete midShapeBack   ;
  if ( endShapeBack ) delete endShapeBack  ;
  if ( progressShape ) delete  progressShape ;
  if ( floatStartShape ) delete  floatStartShape ;
  if ( floatEndShape ) delete  floatEndShape ;
  bool block = myGanttView->myTimeTable->blockUpdating();
  myGanttView->myTimeTable->setBlockUpdating();
  myGanttView->myTimeTable->removeItemFromTasklinks( this );
  myGanttView->myCanvasView->resetCutPaste( this );
  if ( listView() ) {
      if ( isOpen() )
          setOpen( false );
      if ( parent() )
          parent()->takeItem( this );
      else
          myGanttView->myListView->takeItem( this );
      myGanttView->myTimeTable->setBlockUpdating( block );
      myGanttView->myTimeTable->updateMyContent();
  } else
    myGanttView->myTimeTable->setBlockUpdating( block );
  if( !_name.isEmpty() )
      sItemDict.remove( _name );
}

/*!
  Generates a unique name if necessary and inserts it into the item
  dictionary.
*/
void KDGanttViewItem::generateAndInsertName( const QString& name )
{
    // First check if we already had a name. This can be the case if
    // the item was reconstructed from an XML file.
    if( !_name.isEmpty() )
        // We had a name, remove it
        sItemDict.remove( _name );
    QString newName;
    if ( name.isEmpty() )
        // create unique name
        newName.sprintf( "%p", (void* )this );
    else {
        if ( sItemDict.find( name ) ) {
            newName.sprintf( "%p", (void* )this );
        }
        else
            newName = name;
    }
    while (  sItemDict.find( newName ) )
        newName += "_";
    sItemDict.insert( newName, this );
    _name = newName;

}

/*!
  Moves the connector c to point p.
  This virtual method is reimplemented in the subclasses.

  \param  c the connector to move
  \param  p point for connector where to move to
  \return true if some value of the item was changed
  \sa getConnector()
*/
bool KDGanttViewItem::moveConnector( KDGanttViewItem::Connector c, QPoint p )
{

    Q_UNUSED( p )
    Q_UNUSED( c )
    return false;
}

/*!
  Returns the region of the item for the position p.
  A region is a connector and it is used for changing item in the gantt view.
  This virtual method is reimplemented in the subclasses.

  \param  p point to check for a connector
  \return This virtual method does nothing and returns NoConnector
  \sa moveConnector()
*/
KDGanttViewItem::Connector  KDGanttViewItem::getConnector( QPoint p )
{

    Q_UNUSED( p )
            return KDGanttViewItem::NoConnector;
}

KDGanttViewItem::Connector KDGanttViewItem::getConnector( QPoint p, bool linkMode )
{

    Q_UNUSED( p )
    Q_UNUSED( linkMode )
    return KDGanttViewItem::NoConnector;
}

/*!
  Returns the item with the specified name.

  \param name the name to search for
  \return the item with the specified name; 0 if no item with that name exists
  \sa name() uid()
*/

KDGanttViewItem* KDGanttViewItem::find( const QString& name )
{
    return sItemDict.find( name );
}


/*!
  Returns the unique name that can identify the item.
  You can look up for an item with a specific unique name uName with find( uName ).

  \return the unique item name
  \sa uid() find()
*/
QString KDGanttViewItem::name() const
{
    return _name;
}


/*!
  Returns the type of the item as string value.
  This may be Event, Task, Summary.
  Reimplement this virtual method in your subclass of a
  KDGanttViewTaskItem, KDGanttViewEventItem or KDGanttViewSummaryItem
  to know what type your items is of.

  \return the type of the item as string value
*/
QString KDGanttViewItem::typeString() const
{
    return typeToString( myType );
}


/*!
  Returns the type of the item.
  This may be Event, Task, Summary.

  \return the type of the item
*/

KDGanttViewItem::Type KDGanttViewItem::type() const
{

    return myType;

}

/*!
  Computes a list of direct subitems of this item which do have some duration during the timespan.
  It call for every direct subitem getTimeForTimespan() and appends it to the list
  if this call does not return 0.
  Please read getTimeForTimespan() for an example when a call of getTimeForTimespan() does not return 0.

  \param  start the start QDateTime of the interval. If an invalid QDateTime is passed it is an open interval,
                i.e. all items before the end datetime will be computed
          end   the end QDateTime of the interval. If an invalid QDateTime is passed it is an open interval,
                i.e. all items after the start datetime will be computed.
                if start and end are invalid all items will be computed.
  \return  a list of direct subitems of this item which do have some duration during the timespan
  \sa getTimeForTimespan() getChildTimeForTimespan() getAllSubChildTimeForTimespan()
*/


QPtrList <KDGanttViewItem>  KDGanttViewItem::getChildListForTimespan( const QDateTime& start, const QDateTime& end )
{
    QPtrList <KDGanttViewItem> childList;
    KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
    while (temp != 0) {
        if ( temp->getTimeForTimespan( start, end ) )
            childList.append( temp );
        temp = temp->nextSibling();
    }
    return childList;
}

/*!
  The virtual method computes for this item the amount of time this item do have in the specified timespan.
  (Specified by the items start datetime and end datetime).
  The default implementation of this method returns 0.
  This method is reimplemented in  the subclass KDGanttViewTaskItem only.
  Example: Let eTask be a task item with
           start date time 24. March 2005 - 17:00
           and end date time 24. March 2005 - 19:30
           i.e. it has a duration of 2:30 hours , which are 9000 seconds.

           eTask->getTimeForTimespan( QDateTime(),QDateTime() )
                  returns the complete duration of 9000, because an open interval was specified
           eTask->getTimeForTimespan( QDateTime(),QDateTime( 24. March 2005 - 18:00 ) )
                  returns 3600.
           eTask->getTimeForTimespan( QDateTime( 24. March 2005 - 17:30 ),QDateTime( 24. March 2005 - 18:00 ) )
                  returns 1800.
           eTask->getTimeForTimespan( QDateTime( 24. March 2005 - 17:30 ),QDateTime( 28. April 2005 - 18:00 ) )
                  returns 7200 ( 2 hours ).
           eTask->getTimeForTimespan( QDateTime( 1. March 2005 - 0:30 ),QDateTime( 28. April 2005 - 18:00 ) )
                  returns 9000 ( complete duration ).
           eTask->getTimeForTimespan( QDateTime( 1. Jan 2004 - 12:30 ),QDateTime( 1. April 2004 - 1:00 ) )
                  returns 0.

  \param  start the start QDateTime of the interval. If an invalid QDateTime is passed it is an open interval.
          end   the end QDateTime of the interval. If an invalid QDateTime is passed it is an open interval.
                if start and end are invalid the duration (start time to end time in seconds) is returned.
  \return the time of this item in the interval in seconds
  \sa getChildTimeForTimespan()  getAllSubChildTimeForTimespan() getChildListForTimespan()
*/
unsigned int KDGanttViewItem::getTimeForTimespan( const QDateTime&, const QDateTime&)
{
    return 0;
}

/*!
  Computes for a set of items recursively the amount of time each item do have in the specified timespan.
  (Specified by the items start datetime and end datetime).
  The set of items in this method is the list of all children of this item.
  I.e. all subchildren ( children of children ) are in this list.
  The item itself is in the list as well.
  It calls for each item getTimeForTimespan() and returns the sum of the computed times.
  Please read getTimeForTimespan() for an example.

  \param  start the start QDateTime of the interval. If an invalid QDateTime is passed it is an open interval,
                i.e. all items before the end datetime will be computed
          end   the end QDateTime of the interval. If an invalid QDateTime is passed it is an open interval,
                i.e. all items after the start datetime will be computed.
                if start and end are invalid all items will be computed.
  \return the sum of times of computed items in seconds
  \sa getTimeForTimespan() getChildTimeForTimespan()  getChildListForTimespan()
*/
unsigned int KDGanttViewItem::getAllSubChildTimeForTimespan(  const QDateTime& start,  const QDateTime& end )
{
    unsigned int retval = 0;
    KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
    while (temp != 0) {
        retval += temp->getAllSubChildTimeForTimespan( start, end );
        temp = temp->nextSibling();
    }
    retval += getTimeForTimespan( start, end );
    return retval;
}

/*!
  Computes for a set of items the amount of time each item do have in the specified timespan.
  (Specified by the items start datetime and end datetime).
  The set of items in this method is the list of direct children of this item.
  No subchildren ( children of children ) are in this list.
  The item itself is not in the list.
  It calls for each item getTimeForTimespan() and returns the sum of the computed times.
  Please read getTimeForTimespan() for an example.

  \param  start the start QDateTime of the interval. If an invalid QDateTime is passed it is an open interval,
                i.e. all items before the end datetime will be computed
          end   the end QDateTime of the interval. If an invalid QDateTime is passed it is an open interval,
                i.e. all items after the start datetime will be computed.
                if start and end are invalid all items will be computed.
  \return the sum of times of computed items in seconds
  \sa getTimeForTimespan() getAllSubChildTimeForTimespan() getChildListForTimespan()
*/

unsigned int KDGanttViewItem::getChildTimeForTimespan(  const QDateTime& start,  const QDateTime& end )
{
    unsigned int retval = 0;
    KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
    while (temp != 0) {
        retval += temp->getTimeForTimespan( start, end );
        temp = temp->nextSibling();
    }
    return retval;
}

/*!
  Sets this item and all subitems in the list view recursively to expanded
  ( if parameter expand is true ) or to closed.

  \param  expand pass true to set item and subitems to expanded
                 pass false to set item and subitems to closed
*/
void KDGanttViewItem::setAllSubitemsExpanded ( bool expand )
{
    bool block = myGanttView->myTimeTable->blockUpdating();
    myGanttView->myTimeTable->setBlockUpdating( true );
    setOpen( expand );
    KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
    while (temp != 0) {
        temp->setAllSubitemsExpanded(  expand );
        temp = temp->nextSibling();
    }
    myGanttView->myTimeTable->setBlockUpdating( block );

}

/*!
  Specifies whether this item is enabled. If disabled, the item stays in the
  Gantt view and the item is shown in gray to show that the item is disabled.
  All signals of this item (like itemLeftClicked( this )) are blocked.
  If the item displays its subitems (children) as a group,
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.

  \param on pass true to make this item editable
  \sa enabled ()
*/
void KDGanttViewItem::setEnabled( bool on )
{
  _enabled = on;
  if ( displaySubitemsAsGroup() ) {
    myGanttView->myTimeTable->inc_blockUpdating();
    KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
    while (temp != 0) {
      temp->setEnabled(  on );
      temp = temp->nextSibling();
    }
    QListViewItem::setEnabled( on );
    myGanttView->myTimeTable->dec_blockUpdating();
  }
  updateCanvasItems();
}


/*!
  Returns whether this item is enabled.

  \return true if this item is enabled, false otherwise
  \sa setEnabled()
*/
bool KDGanttViewItem::enabled () const
{
  return _enabled;
}


// *********************************
/*!
  Specifies whether this item is visible.

  \param on pass true to make this item visible
  \sa itemVisible ()
*/
void KDGanttViewItem::setItemVisible( bool on )
{
  if ( on ) {
    resetSubitemVisibility();
  } else
    setVisible( false );
  //updateCanvasItems();
  myGanttView->myTimeTable->updateMyContent();
}


/*!
  Returns whether this item is visible.

  \return true if this item is visible, false otherwise
  \sa setItemVisible()
*/
bool KDGanttViewItem::itemVisible () const
{
  return QListViewItem::isVisible();
}


// *************************************

/*!
  Specifies whether this item is editable. The whole Gantt view needs
  to be editable as well for this to have any effect.

  \param editable pass true to make this item editable
  \sa editable(), KDGanttView::setEditable(), KDGanttView::editable()
*/
void KDGanttViewItem::setEditable( bool editable )
{
    isEditable = editable;
}


/*!
  Returns whether this item is editable.

  \return true if this item is editable, false otherwise
  \sa setEditable(), KDGanttView::setEditable(), KDGanttView::editable()
*/

bool KDGanttViewItem::editable() const
{
    return isEditable;
}


/*!
  Specifies whether this item shows hidden subitems on its timeline.
  Useful to get a so called "calendar view" with many items in one row.
  When \a displaySubitemsAsGroup() is set to true, this item has a normal view,
  when it is expanded. If it is not expanded (and has at least one child),
  the item itself is hidden, and all children are displayed instead.
  To manage the painting priority of the children (if overlapping),
  you may set \a priority() of these items.

  \param show pass true to make this item displaying hidden subitems
  \sa editable(), KDGanttView::setEditable(), KDGanttView::editable(), setPriority()
*/
void KDGanttViewItem::setDisplaySubitemsAsGroup( bool show )
{
  if ( !show && _displaySubitemsAsGroup)
    isVisibleInGanttView = true;
  _displaySubitemsAsGroup = show;
  if ( parent() )
    if ( parent()->isOpen() )
	 parent()->setOpen( true );
  if ( isOpen() )
    setOpen( true );
  updateCanvasItems();
}


/*!
  Returns whether this item displays hidden subitems.
  Initial set to false.

  \return true if this item displays hidden subitems, false otherwise
  \sa setDisplaySubitemsAsGroup()
*/
bool KDGanttViewItem::displaySubitemsAsGroup() const
{
    return _displaySubitemsAsGroup;
}


/*!
  Specifies the priority of this item.
  Valid values are between 1 and 199.
  A priority less than 100 means that the item is painted in the
  Gantt chart below the grid. A priority more than 100 means
  that the item is painted in the Gantt chart over the grid.
  For a value of 100, the behavior is unspecified.
  An item with a higher priority is painted over an item with a lower
  priority in the Gantt chart. The painting order of items with the
  same priority is unspecified.
  For Calendar items, the default priority is 50, for all other items
  it is 150.
  This feature makes only sense for an item which is a child of
  another item, which \a displaySubitemsAsGroup() property is set to
  true.

  \param prio the new priority of this item.
  \sa priority(), displaySubitemsAsGroup()
*/
void KDGanttViewItem::setPriority( int prio )
{
  if ( prio < 1 )
    prio = 1;
  if (prio > 199 )
    prio = 199;
  _priority = prio;
  updateCanvasItems();
}


/*!
  Returns the priority of this item.
  \return the priority of this item
  \sa setDisplaySubitemsAsGroup()
*/
int KDGanttViewItem::priority()
{
    return _priority;
}


/*!
  Specifies the start time of this item. The parameter must be valid
  and non-null. If the parameter is invalid or null, no value is set.
  Reimplemented in the subclasses.

  \param start the start time
  \sa startTime(), setEndTime(), endTime()
*/
void KDGanttViewItem::setStartTime( const QDateTime&  )
{
}


/*!
  Returns the start time of this item.

  \return the start time of this item
  \sa setStartTime(), setEndTime(), endTime()
*/
QDateTime KDGanttViewItem::startTime() const
{
    return myStartTime;
}


/*!
  Specifies the end time of this item. The parameter must be valid
  and non-null. If the parameter is invalid or null, no value is set.
  Reimplemented in the subclasses

  \param end the end time
  \sa endTime(), setStartTime(), startTime()
*/
void KDGanttViewItem::setEndTime( const QDateTime& end )
{
    switch( type() ) {
    case Event:
        qDebug( "KDGantt:Event Item has no end time" );
        break;
    case Summary:
        ((KDGanttViewSummaryItem*)this)->setEndTime( end );
        break;
    case Task:
        qDebug( "KDGantt:Task Item has no end time" );
        break;
    default:
        qDebug( "Unknown type in KDGanttViewItem::typeToString()" );
    }
}


/*!
  Returns the end time of this item.

  \return the end time of this item
  \sa setEndTime(), setStartTime(), startTime()
*/
QDateTime KDGanttViewItem::endTime() const
{
    return myEndTime;
}


/*!
  Sets the text to be shown in this item in the Gantt view.
  For a KDGanttViewTaskItem witht displaySubitemsAsGroup() == true,
  the text is shown in the item itself and
  the text is truncated automatically, if it does not fit in the item.
  For all other item types, the text is shown to the right of the item.

  \param text the text to be shown
  \sa text(), setTextColor(), textColor(), setListViewText(),
  listViewText()
*/
void KDGanttViewItem::setItemText( const QString& text )
{
    textcanvas()->setText(text);
    textCanvasText = text;
    updateCanvasItems();
}


/*!
  Returns the text to be shown in this item in the Gantt view.

  \return the text to be shown in this item
  \sa setText(), setTextColor(), textColor(), setListViewText(),
  listViewText()
*/
QString KDGanttViewItem::itemText() const
{
    return textCanvasText;
}


/*!
  \deprecated Use setListViewTest( int, const QString& ) instead
*/
void KDGanttViewItem::setListViewText( const QString& text, int column )
{
    QListViewItem::setText( column, text );
}


/*!
  Sets the text to be shown in this item in the list view.

  \param column the column in which the text will be shown
  \param text the text to be shown
  \sa text(), setTextColor(), textColor(), setText(), listViewText()
*/
void KDGanttViewItem::setListViewText( int column, const QString& text )
{
    QListViewItem::setText( column, text );
}


/*!
  Returns the text to be shown in this item in the list view.

  \param column the column in which the text will be shown
  \return the text to be shown in this item
  \sa setText(), setTextColor(), textColor(), text(),
  setListViewText()
*/
QString KDGanttViewItem::listViewText( int column ) const
{
    return QListViewItem::text( column );
}


/*!
  Sets the font to be used for the text in this item.

  \param font the font to be shown
  \sa font()
*/
void KDGanttViewItem::setFont( const QFont& font )
{
    textcanvas()->setFont(font);
    updateCanvasItems();
}


/*!
  Returns the font used for the text in this item.

  \return the font used for the text in this item
  \sa setFont()
*/
QFont KDGanttViewItem::font() const
{

    if (mTextCanvas)
        return mTextCanvas->font();
    return myGanttView->font();
}


/*!
  Sets the text to show in a tooltip for this item.

  \param text the tooltip text
  \sa tooltipText()
*/
void KDGanttViewItem::setTooltipText( const QString& text )
{
    myToolTipText = text;
}


/*!
  Returns the tooltip text of this item

  \return the tooltip text
  \sa setTooltipText()
*/
QString KDGanttViewItem::tooltipText() const
{

    return myToolTipText;
}


/*!
  Sets the text to show in a What's This window for this item.

  \param text the what's this text
  \sa whatsThisText()
*/
void KDGanttViewItem::setWhatsThisText( const QString& text )
{
    myWhatsThisText = text;
}


/*!
  Returns the what's this text of this item

  \return the what's this text
  \sa setWhatsThisText()

*/
QString KDGanttViewItem::whatsThisText() const
{
    return myWhatsThisText;
}


/*!
  Specifies whether this item should be shown highlighted. The user
  can also highlight items with the mouse.
  If the item displays its subitems (children) as a group
  (displaySubitemsAsGroup() == true),
  all changes apply to all subitems as well.

  \param highlight true in order to highlight, false in order to turn
  highlighting off for this item
  \sa highlight()
*/
void KDGanttViewItem::setHighlight( bool highlight )
{

    isHighlighted = highlight;
    if ( displaySubitemsAsGroup() ) {
      myGanttView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
	temp->setHighlight( highlight );
	temp = temp->nextSibling();
      }
      myGanttView->myTimeTable->dec_blockUpdating();
    }

    updateCanvasItems();
}


/*!
  Returns whether this item is highlighted, either programmatically
  with setHighlight() or by the user with the mouse.

  \return true if the item is highlighted
  \sa setHighlight()
*/
bool KDGanttViewItem::highlight() const
{
    return isHighlighted;
}


/*!
  Specifies the shapes to be used for this item.

  It is advisable not to use this method, but rather set the shapes
  for all items of a type with KDGanttView::setShapes() in order to
  get a uniform Gantt view.

  \param start the start shape
  \param middle the middle shape
  \param end the end shape
  \sa shapes(), setColors(), colors()
*/
void KDGanttViewItem::setShapes( Shape start, Shape middle, Shape end )
{

    myStartShape =  start;
    myMiddleShape= middle;
    myEndShape=  end;
    createShape(startShape,startShapeBack,start);
    createShape(midShape,midShapeBack,middle);
    midShape->setZ( 4 );
    createShape(endShape,endShapeBack,end);
    updateCanvasItems();
}


/*!
  Creates shapes of the specified type \a shape.
  The background shape color is set to black and the background shape
  is slightly bit bigger than the foreground shape to have a black border
  around the foreground shape.

  \param itemShape the foreground shape
  \param middle itemShapeBack  the background shape
  \param shape the type of the shape
   (may be TriangleDown, TriangleUp, Diamond, Square, Circle)
  \sa shapes(), setColors(), colors()
*/
void KDGanttViewItem::createShape( KDCanvasPolygonItem* &itemShape,
                                   KDCanvasPolygonItem* &itemShapeBack,
                                   Shape shape )
{
  if ( itemShape && type() ==  Task )
    return;
    if (itemShape) delete itemShape;
    if (itemShapeBack) delete itemShapeBack;

    QCanvasPolygonalItem * item;
    QCanvasPolygonalItem * itemBack;
    int size = myItemSize+2;
    int hei = (myItemSize/3)/2;
    switch (shape) {
    case TriangleDown:
        {
            item = new KDCanvasPolygon(myGanttView->myTimeTable, this,Type_is_KDGanttViewItem);
            QPointArray arr = QPointArray(3);
            arr.setPoint(0,-size/2,-hei);
            arr.setPoint(1,size/2,-hei);
            arr.setPoint(2,0,((size/2)-hei));
            ((QCanvasPolygon*)item)->setPoints(arr);
            size += 4;hei +=1;
            itemBack = new KDCanvasPolygon(myGanttView->myTimeTable, this,Type_is_KDGanttViewItem);
            arr.setPoint(0,-size/2,-hei);
            arr.setPoint(1,size/2,-hei);
            arr.setPoint(2,0,((size/2)-hei));
            ((QCanvasPolygon*)itemBack)->setPoints(arr);

            break;
        }
    case TriangleUp:
        {
            // I really do not know why, but  we get only an TriangleUp-icon
            // of the same size as a TriangleDown-icon, if we increment the size by 2
            size+=2;
            item = new KDCanvasPolygon(myGanttView->myTimeTable, this,Type_is_KDGanttViewItem);
            QPointArray arr = QPointArray(3);
            arr.setPoint(0,-size/2,hei);
            arr.setPoint(1,size/2,hei);
            arr.setPoint(2,0,(-size/2)+hei);
            ((QCanvasPolygon*)item)->setPoints(arr);
            size += 4;hei +=1;
            itemBack = new KDCanvasPolygon(myGanttView->myTimeTable, this,Type_is_KDGanttViewItem);
            arr.setPoint(0,-size/2,hei);
            arr.setPoint(1,size/2,hei);
            arr.setPoint(2,0,(-size/2)+hei);
            ((QCanvasPolygon*)itemBack)->setPoints(arr);

            break;
        }

    case Diamond:
        {
            item = new KDCanvasPolygon(myGanttView->myTimeTable, this,Type_is_KDGanttViewItem);
            QPointArray arr = QPointArray(4);
            arr.setPoint(0,0,-size/2);
            arr.setPoint(1,size/2,0);
            arr.setPoint(2,0,size/2);
            arr.setPoint(3,-size/2,0);
            ((QCanvasPolygon*)item)->setPoints(arr);
            size += 2;hei +=1;
            itemBack = new KDCanvasPolygon(myGanttView->myTimeTable, this,Type_is_KDGanttViewItem);
            arr.setPoint(0,0,-size/2);
            arr.setPoint(1,size/2,0);
            arr.setPoint(2,0,size/2);
            arr.setPoint(3,-size/2,0);
            ((QCanvasPolygon*)itemBack)->setPoints(arr);
            break;
        }

    case Square:
        {
            size -=2;
            item = new KDCanvasPolygon(myGanttView->myTimeTable, this,Type_is_KDGanttViewItem);
            QPointArray arr = QPointArray(4);
            arr.setPoint(0,-size/2,-size/2);
            arr.setPoint(1,size/2,-size/2);
            arr.setPoint(2,size/2,size/2);
            arr.setPoint(3,-size/2,size/2);
            ((QCanvasPolygon*)item)->setPoints(arr);
            size += 2;hei +=1;
            itemBack = new KDCanvasPolygon(myGanttView->myTimeTable, this,Type_is_KDGanttViewItem);
            arr.setPoint(0,-size/2,-size/2);
            arr.setPoint(1,size/2,-size/2);
            arr.setPoint(2,size/2,size/2);
            arr.setPoint(3,-size/2,size/2);
            ((QCanvasPolygon*)itemBack)->setPoints(arr);
            break;
        }

    case Circle:
        {
            size -= 2;
            item = new KDCanvasEllipse(myGanttView->myTimeTable, this,Type_is_KDGanttViewItem);
            ((KDCanvasEllipse*)item)->setSize(size,size);
            size += 2;hei +=1;
            itemBack = new KDCanvasEllipse(myGanttView->myTimeTable, this,Type_is_KDGanttViewItem);
            ((KDCanvasEllipse*)itemBack)->setSize(size,size);
            break;
        }
    default:
        // Uninitialized shape, can e.g. be the case with free-busy
        // items which don't have any shapes
        return;
    }
    item->setBrush(Qt::SolidPattern);
    item->setZ(5);
    itemShape = (KDCanvasPolygonItem*) item;
    itemBack->setBrush(Qt::SolidPattern);
    itemBack->setZ(3);
    itemShapeBack = (KDCanvasPolygonItem*) itemBack;

}


/*!
  Returns the shapes used for this item

  \param start returns the start shape
  \param middle returns the middle shape
  \param end returns the end shape
  \sa setShapes(), setColors(), colors()
*/
void KDGanttViewItem::shapes( Shape& start, Shape& middle, Shape& end ) const
{
    start = myStartShape;
    middle = myMiddleShape;
    end = myEndShape;
}


/*!
  Specifies the colors in which to draw the shapes of this item.

  It is advisable not to use this method, but rather set the colors
  for all items of a type with KDGanttView::setColors() in order to
  get a uniform Gantt view.

  \param start the color for the start shape
  \param middle the color for the middle shape
  \param end the color for the end shape
  \sa colors(), setShapes(), shapes(), setDefaultColor(), defaultColor()
*/
void KDGanttViewItem::setColors( const QColor& start, const QColor& middle,
                                 const QColor& end )
{

    myStartColor=start ;
    myMiddleColor= middle;
    myEndColor= end;
    if ( displaySubitemsAsGroup() ) {
      myGanttView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
          if ( !temp->displaySubitemsAsGroup() )
              temp->setColors( start, middle, end );
          temp = temp->nextSibling();
      }
      myGanttView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}


/*!
  Returns the colors used for this item

  \param start returns the start color
  \param middle returns the middle color
  \param end returns the end color
  \sa setColors(), setShapes(), shapes(), setDefaultColor(), defaultColor()
*/
void KDGanttViewItem::colors( QColor& start, QColor& middle, QColor& end ) const
{
    start = myStartColor ;
    middle =  myMiddleColor;
    end  = myEndColor;

}


/*!
  Specifies the highlight colors in which to draw the shapes of this item.

  It is advisable not to use this method, but rather set the highlight
  colors for all items of a type with
  KDGanttView::setHighlightColors() in order to get a uniform Gantt
  view.

  If the item displays its subitems (children) as a group,
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.

  \param start the highlight color for the start shape
  \param middle the highlight color for the middle shape
  \param end the highlight color for the end shape
  \sa highlightColors(), setShapes(), shapes()
*/
void KDGanttViewItem::setHighlightColors( const QColor& start, const QColor& middle, const QColor& end )
{
    myStartColorHL=start ;
    myMiddleColorHL= middle;
    myEndColorHL= end;
    if ( displaySubitemsAsGroup() ) {
      myGanttView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
          if ( !temp->displaySubitemsAsGroup() )
              temp->setHighlightColors( start,  middle,  end );
          temp = temp->nextSibling();
      }
      myGanttView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}


/*!
  Returns the highlight colors used for this item

  \param start returns the start highlight color
  \param middle returns the middle highlight color
  \param end returns the end highlight color
  \sa setHighlightColors(), setShapes(), shapes()
*/
void KDGanttViewItem::highlightColors( QColor& start, QColor& middle, QColor& end ) const
{
    start = myStartColorHL ;
    middle =  myMiddleColorHL;
    end  = myEndColorHL;
}


/*!
  Specifies the color to be used for the text of this item.

  It is advisable not to use this method, but rather set the text color
  for all items with KDGanttView::setTextColor() in order to get a
  uniform Gantt view.
  If the item displays its subitems (children) as a group,
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.
  \param color the text color
  \sa textColor(), setText(), text()
*/
void KDGanttViewItem::setTextColor( const QColor& color )
{

   myTextColor = color;
   if ( displaySubitemsAsGroup() ) {
      myGanttView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
	temp->setTextColor(color);
	temp = temp->nextSibling();
      }
      myGanttView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}


/*!
  Returns the color used for the text of this item.

  \return the text color
  \sa setTextColor(), setText(), text()
*/
QColor KDGanttViewItem::textColor() const
{
    return myTextColor;
}


/*!
  \enum KDGanttViewItem::Shape

  This enum is used in order to specify the shapes of a Gantt chart
  item.
*/


/*!
  \enum KDGanttViewItem::Type

  This enum is used in order to return the type of a Gantt chart item.
 */

/*!
  \enum KDGanttViewItem::Connector

  This enum is used to define the types of connectors items can have.
 */


/*!
  Sets the pixmap that is shown in the listview.

  \param column the column in which the pixmap is shown
  \param pixmap the pixmap to show
  \sa pixmap()
*/
void KDGanttViewItem::setPixmap( int column, const QPixmap& pixmap )
{
    QListViewItem::setPixmap( column, pixmap );
}

/*!
  \deprecated use setPixmap( int, const QPixmap& ) instead
*/
void KDGanttViewItem::setPixmap( const QPixmap& pixmap )
{
    QListViewItem::setPixmap( 0, pixmap );
}


/*!
  Returns a pixmap that is shown in the listview.

  \param column the column for which to query the pixmap
  \return a pointer to the pixmap shown
  \sa setPixmap()
*/
const QPixmap* KDGanttViewItem::pixmap( int column ) const
{
    return QListViewItem::pixmap( column );
}


/*!
  Sets the default color that is used for the item if no specific
  start, middle, or end colors are set.

  It is advisable not to use this method, but rather set the colors
  for all items of a type with KDGanttView::setDefaultColor() in order to
  get a uniform Gantt view.

  If the item displays its subitems (children) as a group,
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.

  \param color the default color to use
  \sa defaultColor(), setColors(), colors()
*/
void KDGanttViewItem::setDefaultColor( const QColor& color )
{
    myDefaultColor = color;


   if ( displaySubitemsAsGroup() ) {
      myGanttView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
	temp->setDefaultColor( color );
	temp = temp->nextSibling();
      }
      myGanttView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}



/*!
  Returns the default color that is used for the item if no specific
  start, middle, or end colors are set.

  \return color the default color used
  \sa setDefaultColor(), setColors(), colors()
*/
QColor KDGanttViewItem::defaultColor() const
{
    return myDefaultColor;
}


/*!
  Sets the default highlighting color that is used for the item if no
  specific start, middle, or end colors are set.

  It is advisable not to use this method, but rather set the colors
  for all items of a type with KDGanttView::setDefaultHighlightColor()
  in order to get a uniform Gantt view.

  If the item displays its subitems (children) as a group,
  (displaySubitemsAsGroup() == true)
  all changes apply to all subitems as well.

  \param color the default highlighting color to use
  \sa defaultHighlightColor(), setHighlightColors(), highlightColors()
*/
void KDGanttViewItem::setDefaultHighlightColor( const QColor& color )
{
    myDefaultColorHL = color;
   if ( displaySubitemsAsGroup() ) {
      myGanttView->myTimeTable->inc_blockUpdating();
      KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
      while (temp != 0) {
	temp->setDefaultHighlightColor( color );
	temp = temp->nextSibling();
      }
      myGanttView->myTimeTable->dec_blockUpdating();
    }
    updateCanvasItems();
}


/*!
  Returns the default highlighting color that is used for the item if
  no specific start, middle, or end colors are set.

  \return color the default highlighting color used
  \sa setDefaultHighlightColor(), setHighlightColors(), highlightColors()
*/
QColor KDGanttViewItem::defaultHighlightColor() const
{
    return myDefaultColorHL;
}


/*!
  Returns the first child of this item.

  \return the first child of this item, 0 if this item has no children
*/
KDGanttViewItem* KDGanttViewItem::firstChild() const
{

    return (KDGanttViewItem* )QListViewItem::firstChild();
}


/*!
  Returns the next sibling item of this item

  \return the next sibling item of this item, 0 if this item has no
  more siblings
*/
KDGanttViewItem* KDGanttViewItem::nextSibling() const
{
    return (KDGanttViewItem* )QListViewItem::nextSibling();
}


/*!
  Returns the parent item of this item

  \return the parent item of this item, 0 if this item is a top-level
  item
*/
KDGanttViewItem* KDGanttViewItem::parent() const
{
    return (KDGanttViewItem*)QListViewItem::parent();
}


/*!
  Returns the item above this item in the listview

  \return the item above this item, 0 if this is the first item
*/
KDGanttViewItem* KDGanttViewItem::itemAbove()
{
    return (KDGanttViewItem* )QListViewItem::itemAbove();
}


/*!
  Returns the item below this item in the listview.
  It can be specified whether the disabled items are taken
  into account as well.

  \param includeDisabled if true, disabled items are considered as well
  \return the item below this item, 0 if this is the last item
*/
KDGanttViewItem* KDGanttViewItem::itemBelow( bool includeDisabled )
{

  KDGanttViewItem*  retItem = (KDGanttViewItem* )QListViewItem::itemBelow();
  if ( !includeDisabled ) {
    return  retItem;
  }
  if ( retItem ) {
    if (itemPos() + height() == retItem->itemPos() ) {
      return retItem;
    }
  }
  KDGanttViewItem*  Item2 = (KDGanttViewItem* )QListViewItem::listView()->itemAt(QPoint (2, QListViewItem::itemPos() + QListViewItem::height() +2) );
  if ( Item2 != 0 )
    if (!Item2->enabled() )
      return Item2;
  return retItem;
}


/*!
  Updates the colors of the item, but not the coordinates.
*/
void KDGanttViewItem::updateCanvasItems()
{
    if (blockUpdating) return;
    QPen p,pBack;
    QBrush b;
    b.setStyle(Qt::SolidPattern);
    if ( type() == Task ) {
        if ( enabled() ) {
            if ( mTextCanvas )
                mTextCanvas->setColor(myTextColor);
            if (isHighlighted) {
                b.setStyle(Qt::SolidPattern);
                b.setColor(myStartColorHL);
                startShape->setBrush(b);
            } else {
                b.setStyle(Qt::SolidPattern);
                b.setColor(myStartColor);
                startShape->setBrush(b);
            }
        } else {
            QColor discol = QColor(232,232,232);
            if ( mTextCanvas )
                mTextCanvas->setColor( QColor(150,150,150) );
            b.setStyle(Qt::SolidPattern);
            b.setColor(discol);
            startShape->setBrush(b);
        }
    } else {
        if ( enabled() ) {
            if ( mTextCanvas )
                mTextCanvas->setColor(myTextColor);
            if (isHighlighted) {
                b.setStyle(Qt::SolidPattern);
                b.setColor(myStartColorHL);
                startShape->setBrush(b);
                b.setColor(myMiddleColorHL);
                midShape->setBrush(b);
                b.setColor(myEndColorHL);
                endShape->setBrush(b);
                p.setWidth(myItemSize/3 -1);
                p.setColor(myStartColorHL);
                startLine->setPen(p);
                p.setColor(myEndColorHL);
                endLine->setPen(p);
            } else {
                b.setStyle(Qt::SolidPattern);
                b.setColor(myStartColor);
                //  qDebug("update color  %s %s", listViewText().toLatin1(),myStartColor.name().toLatin1() );
                startShape->setBrush(b);
                b.setColor(myMiddleColor);
                midShape->setBrush(b);
                b.setColor(myEndColor);
                endShape->setBrush(b);
                p.setWidth(myItemSize/3-1);
                p.setColor(myStartColor);
                startLine->setPen(p);
                p.setColor(myEndColor);
                endLine->setPen(p);
            }
        } else {
            //QColor discol = Qt::lightGray;
            QColor discol = QColor(232,232,232);
            if ( mTextCanvas )
                mTextCanvas->setColor( QColor(150,150,150) );
            b.setStyle(Qt::SolidPattern);
            b.setColor(discol);
            startShape->setBrush(b);
            midShape->setBrush(b);
            endShape->setBrush(b);
            p.setWidth(myItemSize/3 -1);
            p.setColor(discol);
            startLine->setPen(p);
            endLine->setPen(p);
        }

        pBack.setWidth((myItemSize/3-1)+2);
        startLineBack->setPen(pBack);
        endLineBack->setPen(pBack);
    }
    if ( mTextCanvas ) {
        QFont f = mTextCanvas->font();
        f.setPixelSize(myItemSize);
        mTextCanvas->setFont(f);
    }
    updateItemsOnCanvas();
}


void KDGanttViewItem::initItem()
{
}


/*!
  This method is reimplemented for internal purposes.
*/
void KDGanttViewItem::setOpen( bool open )
{
  if ( _callListViewOnSetOpen ) {
    // notify the listview about a programatically called setOpen()
    if ( listView () )
      listView ()->setOpen( this, open );

  } else {

    QListViewItem::setOpen( open );
  }
}


void KDGanttViewItem::showItem( bool, int )
{
}


QPoint KDGanttViewItem::getTaskLinkStartCoord(QPoint p)
{

    if ( mTextCanvas )
        mTextCanvas->move(p.x()+myItemSize, itemPos() + height()/2-myItemSize/2);
    return QPoint (myGanttView->myTimeHeader->getCoordX(myEndTime) +myItemSize/2,itemPos()+height()/2);
}


QPoint KDGanttViewItem::getTaskLinkEndCoord()
{
    return QPoint (myGanttView->myTimeHeader->getCoordX(myStartTime)-myItemSize/2 ,itemPos()-myItemSize/2+height()/2-2);
}


void KDGanttViewItem::hideSubtree()
{
  if (firstChild())
    firstChild()->hideSubtree();
  if ( nextSibling () )
    nextSibling ()->hideSubtree();
  showItem(false);
}


void KDGanttViewItem::setCallListViewOnSetOpen( bool call )
{
  _callListViewOnSetOpen = call;
}


QCanvasText* KDGanttViewItem::textcanvas()
{
    if ( !mTextCanvas ) {
        mTextCanvas = new KDCanvasText(myGanttView->myTimeTable,this,Type_is_KDGanttViewItem);
        mTextCanvas->setText("");
        mTextCanvas->setZ(10);
        mTextCanvas->setFont(myGanttView->font());
        // set default textcolor
        setTextColor( myGanttView->textColor());
    }
    return (QCanvasText*) mTextCanvas;
}

void KDGanttViewItem::updateItemsOnCanvas( bool forceUpdate  )
{
    if ( !forceUpdate && myGanttView->myTimeTable->blockUpdating() )
        return;
    showItem( isVisibleInGanttView, mCurrentCoord_Y );
    KDGanttViewItem * par = parent();
    while ( par ) {
        par->showItem( par->isVisibleInGanttView, par->mCurrentCoord_Y );
        par = par->parent();
    }
    myGanttView->myTimeTable->computeTaskLinksForItem( this );
    myGanttView->myTimeTable->simpleUpdate();
}
void KDGanttViewItem::initColorAndShapes(Type t)
{
    //qDebug("KDGanttViewItem::initColorAndShapes ");
    //_isCalendar = false;
    mCurrentConnectorCoordX = -1;
    mCurrentConnectorDiffX = 0;
    mCurrentCoord_Y = 0;
    _isMoveable = false;
    _isResizeable = false;
    setTextOffset(QPoint(0,0));
  //_isCalendar = false;
    _callListViewOnSetOpen = true;
    myType = t;
    blockUpdating = true;
    isVisibleInGanttView = false;
    startShape = 0;
    midShape = 0;
    endShape = 0;
    startShapeBack = 0;
    midShapeBack = 0;
    endShapeBack = 0;
    startLine = 0;
    endLine = 0;
    startLineBack = 0;
    endLineBack = 0;
    actualEnd = 0;
    myProgress = 0;
    progressShape = 0;
    floatStartShape = 0;
    floatEndShape = 0;
    myItemSize = 10;
    mTextCanvas = 0;
    myGanttView = ((KDListView *)listView())->myGanttView;
    myGanttView->myTimeHeader->saveCenterDateTime();
    myStartTime = myGanttView->myTimeHeader->myCenterDateTime;
    myEndTime = myStartTime;
    myToolTipText =QListViewItem::text(0);
    myWhatsThisText = QListViewItem::text(0);
    isHighlighted = false;
    isEditable = true;
    _displaySubitemsAsGroup = myGanttView->displaySubitemsAsGroup();

    // set default color
    setDefaultColor( myGanttView->defaultColor(myType));
    // set default highlight color
    setDefaultHighlightColor(myGanttView->defaultHighlightColor(myType));
    // set shapes
    shapeDefined = (myGanttView->shapes(myType,myStartShape,myMiddleShape,myEndShape));
    if ( type() == Task ) {
        //qDebug("new task %s ", listViewText().toLatin1());
        if ( startShape )
            delete startShape;
        startShape = (KDCanvasPolygonItem*)new  KDCanvasRectangle(myGanttView->myTimeTable,this,Type_is_KDGanttViewItem);
        if (progressShape)
            delete progressShape;
        progressShape = (KDCanvasPolygonItem*)new  KDCanvasRectangle(myGanttView->myTimeTable,this,Type_is_KDGanttViewItem);
    } else {
        startLine = new KDCanvasLine(myGanttView->myTimeTable,this,Type_is_KDGanttViewItem);//KDGanttViewItem );
        endLine = new KDCanvasLine(myGanttView->myTimeTable,this,Type_is_KDGanttViewItem);
        startLine->setZ(2);endLine->setZ(2);
        startLineBack = new KDCanvasLine(myGanttView->myTimeTable,this,Type_is_KDGanttViewItem);//KDGanttViewItem );
        endLineBack = new KDCanvasLine(myGanttView->myTimeTable,this,Type_is_KDGanttViewItem);
        startLineBack->setZ(1);endLineBack->setZ(1);
        actualEnd = new KDCanvasLine(myGanttView->myTimeTable,this,Type_is_KDGanttViewItem);
        actualEnd->setZ(5);
        actualEnd->setPen( QPen ( Qt::red, 3 ) );
        //setShapes(myStartShape,myMiddleShape,myEndShape);
        setShapes(myStartShape,myMiddleShape,myEndShape);
    }
    if (floatStartShape)
        delete floatStartShape;
    floatStartShape = (KDCanvasPolygonItem*)new  KDCanvasRectangle(myGanttView->myTimeTable,this,Type_is_KDGanttViewItem);
    if (floatEndShape)
        delete floatEndShape;
    floatEndShape = (KDCanvasPolygonItem*)new  KDCanvasRectangle(myGanttView->myTimeTable,this,Type_is_KDGanttViewItem);

    // set color of shapes
    if (!( colorDefined = (myGanttView->colors(myType,myStartColor,myMiddleColor,myEndColor)))) {

    };
    setColors(defaultColor(),defaultColor(), defaultColor());
    // set highlight color of shapes
    if (!( colorHLDefined = (myGanttView->highlightColors(myType,myStartColorHL,myMiddleColorHL,myEndColorHL)))) {

    };
    setHighlightColors(defaultHighlightColor(),defaultHighlightColor(), defaultHighlightColor());
    // if (type() ==  Task)
    //setText(QListViewItem::text(0)); // testing only
    //isvisible = true;
    _priority = 150;
    _showNoInformation = false;
    _showNoInformationBeforeAndAfter = false;
    _enabled = true;
    blockUpdating = false;
    updateCanvasItems();
}


QString KDGanttViewItem::shapeToString( Shape shape )
{
    switch( shape ) {
    case TriangleDown:
        return "TriangleDown";
    case TriangleUp:
        return "TriangleUp";
    case Diamond:
        return "Diamond";
    case Square:
        return "Square";
    case Circle:
        return "Circle";
    }
    return "";
}


KDGanttViewItem::Shape KDGanttViewItem::stringToShape( const QString& string )
{
    if( string == "TriangleDown" )
        return TriangleDown;
    else if( string == "TriangleUp" )
        return TriangleUp;
    else if( string == "Diamond" )
        return Diamond;
    else if( string == "Square" )
        return Square;
    else if( string == "Circle" )
        return Circle;
    else
        return TriangleDown;
}
/*!
  This virtual method does nothing.
  Reimplement it to save your own data to the QDomElement.
  The data can be read with userReadFromElement().
  You should reimplement it in a subclass to make Drag&Drop working properly.
  The body contains a small example to write an int value and a QString,
  which is commented out.
  This method is automatically called from  KDGanttViewItem::createNode()
  after a start Drag operation or if a configuration file is saved via KDGanttView::saveProject().

  \param doc the DOM document to which the node belongs
  \param parentElement the element into which to insert user defined data
  \sa userReadFromElement() createNode() KDGanttView::saveProject()
*/
void KDGanttViewItem::userWriteToElement( QDomDocument& doc,
                                  QDomElement& userElement )
{
    Q_UNUSED( doc );
    Q_UNUSED( userElement );
    // example for writing user defined data
    /*
    int userNumber = 815;
    QString userData = "this is saved text from the user";
    KDGanttXML::createStringNode( doc, userElement, "ExampleText", userData );
    KDGanttXML::createIntNode( doc, userElement, "ExampleNumber", userNumber );
    */

}

/*!
  Creates a DOM node that describes this item.

  \param doc the DOM document to which the node belongs
  \param parentElement the element into which to insert this node
*/
void KDGanttViewItem::createNode( QDomDocument& doc,
                                  QDomElement& parentElement )
{
    QDomElement itemElement = doc.createElement( "Item" );
    parentElement.appendChild( itemElement );
    itemElement.setAttribute( "Type", typeString() );

    KDGanttXML::createDateTimeNode( doc, itemElement, "StartTime", startTime() );
    KDGanttXML::createDateTimeNode( doc, itemElement, "EndTime", endTime() );
    KDGanttXML::createFontNode( doc, itemElement, "Font", font() );
    KDGanttXML::createStringNode( doc, itemElement, "Text", itemText() );
    KDGanttXML::createStringNode( doc, itemElement, "TooltipText", tooltipText() );
    KDGanttXML::createStringNode( doc, itemElement, "WhatsThisText",whatsThisText() );
    if( pixmap() ) {
        KDGanttXML::createPixmapNode( doc, itemElement, "Pixmap", *pixmap() );
    }
    //PENDING save pixmaps for all clomuns with
    //KDGanttXML::createPixmapNode( doc, itemElement, "Pixmap", *pixmap(), column );
    KDGanttXML::createStringNode( doc, itemElement, "ListViewText", listViewText() );
    KDGanttXML::createBoolNode( doc, itemElement, "Open", isOpen() );
    KDGanttXML::createBoolNode( doc, itemElement, "Enabled", enabled() );
    KDGanttXML::createBoolNode( doc, itemElement, "Visible", itemVisible() );
    KDGanttXML::createBoolNode( doc, itemElement, "Editable", editable() );
    KDGanttXML::createBoolNode( doc, itemElement, "ShowNoInformation", showNoInformation() );
    KDGanttXML::createBoolNode( doc, itemElement, "Highlight", highlight() );
    KDGanttXML::createBoolNode( doc, itemElement, "DisplaySubitemsAsGroup", displaySubitemsAsGroup() );
    KDGanttXML::createIntNode( doc, itemElement, "Priority", priority() );
    Shape startShape, middleShape, endShape;
    shapes( startShape, middleShape, endShape );
    KDGanttXML::createStringNode( doc, itemElement, "StartShape",
                             shapeToString( startShape ) );
    KDGanttXML::createStringNode( doc, itemElement, "MiddleShape",
                             shapeToString( middleShape ) );
    KDGanttXML::createStringNode( doc, itemElement, "EndShape",
                             shapeToString( endShape ) );
    KDGanttXML::createColorNode( doc, itemElement, "DefaultColor", defaultColor() );
    QColor startColor, middleColor, endColor;
    colors( startColor, middleColor, endColor );
    KDGanttXML::createColorNode( doc, itemElement, "StartColor", startColor );
    KDGanttXML::createColorNode( doc, itemElement, "MiddleColor", middleColor );
    KDGanttXML::createColorNode( doc, itemElement, "EndColor", endColor );
    KDGanttXML::createColorNode( doc, itemElement, "DefaultHighlightColor",
                            defaultHighlightColor() );
    highlightColors( startColor, middleColor, endColor );
    KDGanttXML::createColorNode( doc, itemElement, "StartHighlightColor",
                            startColor );
    KDGanttXML::createColorNode( doc, itemElement, "MiddleHighlightColor",
                            middleColor );
    KDGanttXML::createColorNode( doc, itemElement, "EndHighlightColor", endColor );
    KDGanttXML::createColorNode( doc, itemElement, "TextColor", textColor() );
    KDGanttXML::createStringNode( doc, itemElement, "Name", name() );
    KDGanttXML::createStringNode( doc, itemElement, "Uid", uid() );
    QDomElement userElement = doc.createElement( "UserSavedData" );
    itemElement.appendChild( userElement );
    userWriteToElement( doc, userElement );
    QDomElement itemsElement = doc.createElement( "Items" );
    itemElement.appendChild( itemsElement );
    KDGanttViewItem* currentItem = firstChild();
    while( currentItem ) {
        currentItem->createNode( doc, itemsElement );
        currentItem = currentItem->nextSibling();
    }

}


/*!
  Creates a KDGanttViewItem according to the specification in a DOM
  element.

  \param view the view in which the item will be inserted
  \param element the DOM element from which to read the specification
  \return the newly created item
*/
KDGanttViewItem* KDGanttViewItem::createFromDomElement( KDGanttView* view,
                                                        QDomElement& element )
{
    QString typeString = element.attribute( "Type" );
    Q_ASSERT( !typeString.isEmpty() );
    KDGanttViewItem* item = view->createNewItem( typeString,
                                                 0, 0 );
    if ( item )
        item->loadFromDomElement( element );
    return item;
}


/*!
  Creates a KDGanttViewItem according to the specification in a DOM
  element.

  \param view the view in which the item will be inserted
  \param previous to item behind this one should appear
  \param element the DOM element from which to read the specification
  \return the newly created item
*/
KDGanttViewItem* KDGanttViewItem::createFromDomElement( KDGanttView* view,
                                                        KDGanttViewItem* previous,
                                                        QDomElement& element )
{
    QString typeString = element.attribute( "Type" );
    Q_ASSERT( !typeString.isEmpty() );
    KDGanttViewItem* item = view->createNewItem( typeString,
                                                 0,
                                                 previous );
    if ( item )
        item->loadFromDomElement( element );
    return item;
}




/*!
  Creates a KDGanttViewItem according to the specification in a DOM
  element.

  \param parent the parent item under which the item will be inserted
  \param element the DOM element from which to read the specification
  \return the newly created item
*/
KDGanttViewItem* KDGanttViewItem::createFromDomElement( KDGanttViewItem* parent,
                                                        QDomElement& element )
{
    QString typeString = element.attribute( "Type" );
    Q_ASSERT( !typeString.isEmpty() );
    KDGanttViewItem* item = parent->myGanttView->createNewItem( typeString,
                                                 parent, 0 );
    if ( item )
        item->loadFromDomElement( element );
    return item;
}


/*!
  Creates a KDGanttViewItem according to the specification in a DOM
  element.

  \param parent the parent item under which the item will be inserted
  \param previous to item behind this one should appear
  \param element the DOM element from which to read the specification
  \return the newly created item
*/
KDGanttViewItem* KDGanttViewItem::createFromDomElement( KDGanttViewItem* parent,
                                                        KDGanttViewItem* previous,
                                                        QDomElement& element )
{
    QString typeString = element.attribute( "Type" );
    Q_ASSERT( !typeString.isEmpty() );
    KDGanttViewItem* item = parent->myGanttView->createNewItem( typeString,
                                                 parent, previous );
    if ( item )
        item->loadFromDomElement( element );
    return item;
}

/*!
  This virtual method does nothing.
  Reimplement it to read your own data from the QDomElement.
  The data was written from userWriteToElement().
  You should reimplement it in a subclass to make Drag&Drop working properly.
  The body contains a small example to read  an int value and a QString,
  which is commented out.
  This method is automatically called from createFromDomElement()=>loadFromDomElement()
  after a Drop operation or if a configuration file is loaded via KDGanttView::loadProject().

  \param doc the DOM document to which the node belongs
  \param parentElement the element which contains user defined data
  \sa userWriteToElement() createFromDomElement() loadFromDomElement()
*/
void KDGanttViewItem::userReadFromElement( QDomElement& element )
{
    Q_UNUSED( element );
    // example for reading user defined data
    /*
    int userNumber = 0;
    QString userData = "";
    QDomNode node = element.firstChild();
    while( !node.isNull() ) {
        QDomElement userElement = node.toElement();
        QString tagName = userElement.tagName();
        if ( tagName == "ExampleNumber" ) {
                int value;
                if( KDGanttXML::readIntNode( userElement, value ) )
                    userNumber = value;
        } else if ( tagName == "ExampleText" ) {
                QString value;
                if( KDGanttXML::readStringNode( userElement, value ) )
                    userData = value;

        }
        node = node.nextSibling();
    }
    qDebug("User data read: %d  %s ", userNumber,userData.toLatin1() );
    */
}

/*
  Fills in the values in the item by reading the DOM element.
*/
void KDGanttViewItem::loadFromDomElement( QDomElement& element )
{
    QDomNode node = element.firstChild();
    Shape startShape = TriangleDown, middleShape = TriangleDown,
            endShape = TriangleDown;
    QColor startColor, middleColor, endColor;
    QColor startHighlightColor, middleHighlightColor, endHighlightColor;
    QString tempName;
    blockUpdating = true;
    bool temp_visible = true;
    while( !node.isNull() ) {
        QDomElement element = node.toElement();
        if( !element.isNull() ) { // was really an element
            QString tagName = element.tagName();
            if( tagName == "StartTime" ) {
                QDateTime value;
                if( KDGanttXML::readDateTimeNode( element, value ) )
                    setStartTime( value );
            } else if( tagName == "EndTime" ) {
                QDateTime value;
                if( KDGanttXML::readDateTimeNode( element, value ) )
                    setEndTime( value );
            } else if( tagName == "Text" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setItemText( value );
            } else if( tagName == "Font" ) {
                QFont value;
                if( KDGanttXML::readFontNode( element, value ) )
                    setFont( value );
            } else if( tagName == "TooltipText" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setTooltipText( value );
            } else if( tagName == "WhatsThisText" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setWhatsThisText( value );
            } else if( tagName == "Pixmap" ) {
                QPixmap value;
                if( KDGanttXML::readPixmapNode( element, value ) )
                    setPixmap( value );
            } else if( tagName == "ListViewText" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setListViewText( value );
            } else if( tagName == "Editable" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setEditable( value );
            } else if( tagName == "ShowNoInformation" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    _showNoInformation = value;
            } else if( tagName == "Visible" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    temp_visible = value;
            } else if( tagName == "Enabled" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    _enabled = value;
            } else if( tagName == "Open" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    setOpen( value );
            } else if( tagName == "Priority" ) {
                int value;
                if( KDGanttXML::readIntNode( element, value ) )
                    setPriority( value );
            } else if( tagName == "DisplaySubitemsAsGroup" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    _displaySubitemsAsGroup = value;
            } else if( tagName == "Highlight" ) {
                bool value;
                if( KDGanttXML::readBoolNode( element, value ) )
                    isHighlighted = value ;
            } else if( tagName == "StartShape" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    startShape = stringToShape( value );
            } else if( tagName == "MiddleShape" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    middleShape = stringToShape( value );
            } else if( tagName == "EndShape" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    endShape = stringToShape( value );
            } else if( tagName == "DefaultColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    setDefaultColor( value );
            } else if( tagName == "StartColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    startColor = value;
            } else if( tagName == "MiddleColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    middleColor = value;
            } else if( tagName == "EndColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    endColor = value;
            } else if( tagName == "DefaultHighlightColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    setDefaultHighlightColor( value );
            } else if( tagName == "StartHighlightColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    startHighlightColor = value;
            } else if( tagName == "MiddleHighlightColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    middleHighlightColor = value;
            } else if( tagName == "EndHighlightColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    endHighlightColor = value;
            } else if( tagName == "TextColor" ) {
                QColor value;
                if( KDGanttXML::readColorNode( element, value ) )
                    setTextColor( value );
            } else if( tagName == "Uid" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    setUid ( value );
            } else if( tagName == "Name" ) {
                QString value;
                if( KDGanttXML::readStringNode( element, value ) )
                    tempName = value;
            }  else if( tagName == "UserSavedData" ) {
                userReadFromElement( element );
            } else if( tagName == "Items" ) {
                QDomNode node = element.firstChild();
                KDGanttViewItem* previous = 0;
                while( !node.isNull() ) {
                    QDomElement element = node.toElement();
                    if( !element.isNull() ) { // was really an element
                        QString tagName = element.tagName();
                        if( tagName == "Item" ) {
                            KDGanttViewItem* newItem;
                            if( previous )
                                newItem =
                                    KDGanttViewItem::createFromDomElement( this,
                                                                           previous,
                                                                           element );
                            else
                                newItem =
                                    KDGanttViewItem::createFromDomElement( this,
                                                                           element );
                            if ( newItem )
                                previous = newItem;
                        } else {
                            qDebug() << "Unrecognized tag name: " << tagName;
                            Q_ASSERT( false );
                        }
                    }

                    node = node.nextSibling();
                }
            } else {
                qDebug() << "Unrecognized tag name: " << tagName;
                Q_ASSERT( false );
            }
        }

        node = node.nextSibling();
    }
    QListViewItem::setVisible( temp_visible );
    QListViewItem::setEnabled( _enabled );
    myStartColor = startColor;
    myMiddleColor = middleColor;
    myEndColor = endColor;
    myStartColorHL = startHighlightColor ;
    myMiddleColorHL = middleHighlightColor;
    myEndColorHL = endHighlightColor;
    blockUpdating = false;
    setShapes( startShape, middleShape, endShape );
    generateAndInsertName( tempName );
}


QString KDGanttViewItem::typeToString( Type type )
{
    switch( type ) {
    case Event:
        return "Event";
    case Summary:
        return "Summary";
    case Task:
        return "Task";
    default:
        qDebug( "Unknown type in KDGanttViewItem::typeToString()" );
        return "Summary";
    }
    return "";
}

KDGanttViewItem::Type KDGanttViewItem::stringToType( const QString& type )
{
    if ( type == "Event")
        return  Event;
    if ( type == "Summary" )
        return Summary;
    if ( type == "Task" )
        return Task;
    //qDebug( "Unknown type in KDGanttViewItem::stringToType(). Returning type Task" );
    return UnknownType;
}

/*!
  Returns the y coordinate of this item.

  \return the y coordinate of this item
*/
int KDGanttViewItem::getCoordY()
{
    return itemPos() + height()/2;
}


void KDGanttViewItem::showSubItems()
{
  showSubitemTree( getCoordY() );
}


void KDGanttViewItem::showSubitemTree( int CoordY )
{

  KDGanttViewItem* temp = firstChild();
  if (temp) {
    while (temp != 0) {
      if (temp->isOpen() || !temp->displaySubitemsAsGroup() ) {
	temp->showItem( true, CoordY );
	if ( temp->firstChild() )
	  temp->firstChild()->hideSubtree();
      } else {
       	if ( temp->displaySubitemsAsGroup() && temp->firstChild() )
	  temp->hideSubtree();
	else {
	  temp->showSubitemTree( CoordY );
	}
      }
      temp = temp->nextSibling();
    }
    showItem( false );
  } else {
    showItem( true, CoordY );
  }
}


/*!
  Returns the earliest start time of the children of this item.
  If for a child the displaySubitemsAsGroup() property is set
  the start time of this child is computed recursively.

  \return the earliest start time of the direct children of this item
  \sa displaySubitemsAsGroup()
*/
QDateTime KDGanttViewItem::myChildStartTime()
{
    QDateTime ret, tempTime;
    bool set = true;
    KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
    if (temp) {
        while (temp != 0) {
            if ( !temp->displaySubitemsAsGroup() ) {
                tempTime = temp->startTime();
            } else {
                tempTime = temp->myChildStartTime();
            }
            if ( tempTime.isValid() ) {
                if ( set ) {
                    set = false;
                    ret = tempTime;
                } else {
                    if ( tempTime < ret ) {
                        ret = tempTime;
                    }
                }
            }
            temp = temp->nextSibling();
        }
    } else {
        if ( displaySubitemsAsGroup() && ( startTime() == endTime() || !endTime().isValid() ) )
            return QDateTime();
        ret = startTime();
    }
    return ret;
}


/*!
  Returns the latest end time of the direct children of this item.
  If for a child the displaySubitemsAsGroup() property is set
  the end time of this child is computed recursively.

  \return the latest end time of the direct children of this item
  \sa displaySubitemsAsGroup()
*/
QDateTime KDGanttViewItem::myChildEndTime()
{
    QDateTime ret, tempTime;
    bool set = true;
    KDGanttViewItem* temp = (KDGanttViewItem*) firstChild();
    if (temp) {
        while (temp != 0) {
            if ( !temp->displaySubitemsAsGroup() ) {
                tempTime = temp->endTime();
            } else {
                tempTime = temp->myChildEndTime();
            }
            if ( tempTime.isValid() ) {
                if ( set ) {
                    set = false;
                    ret = tempTime;
                } else {
                    if ( tempTime > ret ) {
                        ret = tempTime;
                    }
                }
            }
            temp = temp->nextSibling();
        }
    } else {
        if ( displaySubitemsAsGroup() && ( startTime() == endTime() || !startTime().isValid() ) )
            return QDateTime();
        ret = endTime();
    }
    return ret;
}


/*!
  Returns whether the 'showNoInformation' line  should be shown for this item

  \return true if showNoInformation line should be shown
  \sa setShowNoInformation(),  KDGanttView::setNoInformationBrush(), KDGanttView::noInformationBrush()
*/
bool KDGanttViewItem::showNoInformation()
{
  return _showNoInformation;
}

/*!
  Returns whether the 'showNoInformation' line  should be shown for the time
  periods before and after this item

  \return true if showNoInformation line should be shown
  \sa setShowNoInformationBeforeAndAfter(),  KDGanttView::setNoInformationBrush(), KDGanttView::noInformationBrush()
*/
bool KDGanttViewItem::showNoInformationBeforeAndAfter()
{
  return _showNoInformationBeforeAndAfter;
}

/*!
  Specifies whether the 'showNoInformation' line  should be shown for
  this item.
  The 'showNoInformation' line is drawn over the whole timeline.
  The height of the line is the height of the item.
  The brush of the line is specified by KDGanttView::setNoInformationBrush().
  (i.e. the same brush for all items of the Gantt view).
  The default brush is QBrush(  QColor ( 100,100,100 ), Qt::FDiagPattern );
  \param show if true, the 'showNoInformation' line  is shown for this item
  \sa  showNoInformation(), KDGanttView::setNoInformationBrush(), KDGanttView::noInformationBrush()
*/
void KDGanttViewItem::setShowNoInformation( bool show )
{
  _showNoInformation = show;
  myGanttView->myTimeTable->updateMyContent();
}

/*!
  Specifies whether the 'showNoInformation' line  should be shown for
  periods before and after this item.
  The height of the line is the height of the item.
  The brush of the line is specified by KDGanttView::setNoInformationBrush().
  (i.e. the same brush for all items of the Gantt view).
  The default brush is QBrush(  QColor ( 100,100,100 ), Qt::FDiagPattern );
  \param show if true, the 'showNoInformation' line  is shown for this item
  \sa  showNoInformationBeforeAndAfter(), KDGanttView::setNoInformationBrush(), KDGanttView::noInformationBrush()
*/
void KDGanttViewItem::setShowNoInformationBeforeAndAfter( bool show )
{
  _showNoInformationBeforeAndAfter = show;
  myGanttView->myTimeTable->updateMyContent();
}

/*!
  If the name of this item is \a name (i.e., listViewText() == name),
  the pointer to this item is returned.  Otherwise, it looks for an
  item with name \a name in the set of children and subchildren of
  this item.
  \param  name the name of the item
  \return the pointer to the item with name \a name
*/
KDGanttViewItem* KDGanttViewItem::getChildByName( const QString& name )
{
  if ( listViewText() == name )
    return this;
  KDGanttViewItem* temp =  firstChild(),* ret;
  while (temp != 0) {
    if ( (ret = temp->getChildByName( name )))
      return ret;
    temp = temp->nextSibling();
  }
  return 0;
}


/*!
  If the uid of this item is \a uid (i.e. a user defined unique id),
  the pointer to this item is returned.  Otherwise, it looks for an
  item with uid \a uid in the set of children and subchildren of
  this item. The uid of an item is not set automatically,
  it has to be set by the programmer.

  \param  uid the uid of the item
  \return the pointer to the item with uid \a uid
  \sa setUid() uid() KDGanttView::getItemByUid()
*/
KDGanttViewItem* KDGanttViewItem::getChildByUid( const QString& uid )
{
  if ( mUid == uid )
    return this;
  KDGanttViewItem* temp =  firstChild(),* ret;
  while (temp != 0) {
    if ( (ret = temp->getChildByUid( uid )))
      return ret;
    temp = temp->nextSibling();
  }
  return 0;
}


/*
void  KDGanttViewItem::printinfo( QString s )
{
  KDGanttViewItem* temp =  firstChild();
  while (temp != 0) {
    temp->printinfo("  "+s );
    temp = temp->nextSibling();
  }
}
*/


/*!
  Returns whether this item has at least one subitem that is a calendar.
  A subitem is a calendar, if that item has at least one subitem or
  displaySubitemAsGroup() is true for that item.

  \return true if the item has at least one subitem that is a calendar.
*/
bool KDGanttViewItem::subitemIsCalendar() const
{
  KDGanttViewItem*   temp = firstChild();
  bool ret = false;
  while (temp) {
    if (temp->firstChild() || temp->displaySubitemsAsGroup() ) {
      ret = true;
      break;
    }
    temp = temp->nextSibling();
  }
  return ret;
}


int  KDGanttViewItem::computeHeight()
{
    int hei = 0;
    // if not visible, hide item and all subitems, return height = 0
    if ( !isVisible() ) {
        showItem( false );
        if ( firstChild() )
            firstChild()->hideSubtree();
        // qDebug("KDGanttViewItem::computeHeight() %s returns 0  ", QListViewItem::text(0).toLatin1());
        return 0;
    }

    KDGanttViewItem* temp;
    bool show = true;

    //  explanation of terms:
    //    display opened item as usual:
    //      display this item opened, display Gantt part on the timeline of this item.
    //      the same for all subitems: display all subitems on its own timeline
    //    display closed item as usual:
    //      display this item closed, display Gantt part on the timeline of this item.
    //      do not display any subitem.

    //  desired behaviour:
    //  if not in calendar mode( GanttView is NOT in calendar mode ):
    //    opened:
    //      display opened item as usual
    //    closed:
    //      if not displaySubitemsAsGroup()
    //        display closed item as usual
    //      else (  displaySubitemsAsGroup() == true )
    //        display not this item, display subitems on the timeline of this item
    //  else ( GanttView is in calendar mode )
    //    4 cases:
    //      opened && displaySubitemsAsGroup():
    //        display not this item, display subitems on the timeline of this item,
    //          which have the property displaySubitemsAsGroup() == false
    //        display the other items,
    //           which have the property displaySubitemsAsGroup() == true,
    //           as usual below this item on their own timeline
    //      opened && NOT displaySubitemsAsGroup():
    //        display opened item as usual
    //      closed && displaySubitemsAsGroup():
    //        display not this item, display subitems on the timeline of this item,
    //          which have the property displaySubitemsAsGroup() == false
    //      closed && NOT displaySubitemsAsGroup():
    //        display closed item as usual
    //
    if ( isOpen() ) {
        //qDebug("KDGanttViewItem::computeHeight() %s is open  ", QListViewItem::text(0).toLatin1());
        temp = firstChild();
        // if item opened, iterate over all subitems
        int tempHeight;
        // introduced special for performance reasons
        bool special = displaySubitemsAsGroup() && myGanttView->calendarMode();
        while (temp != 0) {
            tempHeight = temp->computeHeight();
            if ( special ) {
                if ( temp->displaySubitemsAsGroup() ) {
                    hei += tempHeight;
                    //qDebug(" hei added ");
                } else {
                    temp->showSubitemTree( getCoordY() );

                }
            } else {
                hei += tempHeight;
                //qDebug(" hei added ");
            }
            temp = temp->nextSibling();
        }
    } else { // closed!
        //qDebug("KDGanttViewItem::computeHeight() %s is closed  ", QListViewItem::text(0).toLatin1());

        if ( !displaySubitemsAsGroup() ) {
            if ( firstChild() ) {
                firstChild()->hideSubtree();
            }
        } else {
            if ( firstChild() ) {
                showSubitemTree( getCoordY() );
                show =  false ;
            }
        }

    }
    if ( show )
        showItem( true );

    hei += height();
    //qDebug("KDGanttViewItem::computeHeight() %s returns:  %d  ", QListViewItem::text(0).toLatin1(), hei);
    return hei;
}


// if this item has at least one subitem which has the property displaySubitemsAsGroup(),
//  a false is returned
bool  KDGanttViewItem::showNoCross()
{
  KDGanttViewItem * temp = firstChild();
  if ( !temp )
    return false;
  while ( temp ) {
    if ( temp->displaySubitemsAsGroup() ) {
      return false;
    }
    temp = temp->nextSibling();
 }
  return true;
}


void  KDGanttViewItem::paintBranches ( QPainter* p, const QColorGroup& cg,
                                       int w, int y, int h )
{
  QListViewItem::paintBranches (  p,  cg, w,  y,  h);
  if ( !myGanttView->calendarMode() )
    return;
  else {
    KDGanttViewItem * temp = firstChild();
    while ( temp ) {
      if ( temp->showNoCross() ) {
	//qDebug("paintNoCross %s ", temp->listViewText(0).toLatin1());
	int y_coord = temp->itemPos() -height ()- itemPos();
	int hei = temp->height();
	//qDebug(" y %d w %d h %d ", y,w,h);
	//qDebug("yc %d  hei %d",y_coord,hei );
	myGanttView->myListView->paintemptyarea( p, QRect( 0,y+y_coord,w,hei));
	int x_c = w/2;
	int y_c = y+y_coord+ temp->height ()/2;
	int y_ce ;
	if ( temp->itemBelow() && temp->itemBelow()->parent() == this )
	  y_ce =y+y_coord+ temp->height ();
	else
	  y_ce = y_c;
	int i;
	for (i = y+y_coord+1; i <= y_ce; i+=2 ) {
	  p->drawPoint( x_c, i );
	}
       	for (i = x_c+2; i < w; i+=2 ) {
	  p->drawPoint( i, y_c  );
	}
      }
      temp = temp->nextSibling();
    }
  }
}


// resets the visibility os the subitems according to the setting of calendar mode
void  KDGanttViewItem::resetSubitemVisibility()
{
    KDGanttViewItem* temp;
    temp = firstChild();
    bool allow = false;
    if ( myGanttView->calendarMode() ) {
        // in calendarmode only items can be opened which have subitems which have subitems
        if ( ! temp ) {
            if ( !parent() )
                // has no parent, has no child : show!
                setVisible( true );
            else
                // has parent, has no child : hide!
                setVisible( false );
            return;
        }

        setDisplaySubitemsAsGroup( true );
        setVisible( true );
        while (temp) {
            if (temp->firstChild()) {
                allow = true;
                temp->resetSubitemVisibility();
            }
            else {
                temp->setVisible(false);
            }
            temp = temp->nextSibling();
        }
    } else {
        setVisible( true );
        // all items can be opened
        allow = true;
        while (temp != 0) {
            temp->resetSubitemVisibility();
            temp = temp->nextSibling();
        }
    }
    if ( !allow && isOpen() )
        setOpen( false );

}
void  KDGanttViewItem::checkCoord( int * co )
{
    if ( *co < -100 )
        *co = -100;
    else if ( *co > 31000 )
        *co = 31000;
}
/*!
  Sets the uid of this item (i.e. in general a user defined unique id).
  The uid of an item is not set automatically,
  it has to be set by the programmer.
  Please read uid() for details.

  \param  uid the uid of the item
  \sa uid()  getChildByUid() KDGanttView::getItemByUid()
*/
void  KDGanttViewItem::setUid( const QString& text )
{
    mUid = text;
}
/*!
  Returns the uid of this item (i.e. in general a user defined unique id)
  The uid of an item is not set automatically,
  it has to be set by the programmer.
  Note that there may be more than one item in the gantt view with the same uid.
  That depends on how the programmer handles the uid management.
  You can look up for items with a specific uid with
  getChildByUid() and KDGanttView::getItemByUid().
  When there is more than one subitem with the same uid for an item then the
  result of getChildByUid() and KDGanttView::getItemByUid() is not well defined.
  Note that after a Drag&Drop (DnD) drop operation you may get another
  item somewhere in the gantt view with the same uid.
  If you want to use a unique id which is unique in every case
  (even after a DnD drop operation) please use KDGanttViewItem::name()
  which is set automatically and which has a very fast lookup compared to
  KDGanttViewItem::getChildByUid().

  \return the uid of the item
  \sa  setUid() getChildByUid() KDGanttView::getItemByUid() name()
*/
QString  KDGanttViewItem::uid() const
{
    return mUid;
}

/*!
  Specifies whether this item should behave like a calendar.
  In calendar mode, only those items can be opened
  which have subitems which have subitems.
  An item which has subitems which have no subitems is called a calendar.
  I.e., an item that contains
  multiple calendars can be opened, while a calendar item itself cannot.
  But if all calendars of an item do not have any subitem (e.g at startup),
  the program cannot detect automatically that it should be possible to open
  this item.
  To enable this, call setIsCalendar( true ); for at least one calendar

  \param cal true in order behave this item like a calendar
  highlighting off for this item
  \sa isCalendar()
*/

/* removed
void KDGanttViewItem::setIsCalendar( bool cal )
{
    _isCalendar = cal;
    updateCanvasItems();
}
*/

/*!
  Returns whether this item behaves like a calendar,
  even though it has no subitem which has subitems; when highlighting
  with setHighlight() or by the user with the mouse.

  \return true if the item behaves like a calendar
  \sa setIsCalendar()
*/
/* removed
bool KDGanttViewItem::isCalendar( ) const
{
    return _isCalendar;
}
*/


/*!
  \var KDGanttViewItem::startLine

  the line at the beginning of the item
*/

/*!
  \var KDGanttViewItem::endLine

  the line at the end of the item
*/


/*!
  \var KDGanttViewItem::startLineBack

  the background line at the beginning of the item
*/

/*!
  \var KDGanttViewItem::endLineBack

  the background line at the end of the item
*/


/*!
  \var KDGanttViewItem::actualEnd

  the line at the actual end of the item
*/

/*!
  \var KDGanttViewItem::startShape

  the shape at the beginning of the item
*/

/*!
  \var KDGanttViewItem::midShape

  the shape in the middle of the item
*/

/*!
  \var KDGanttViewItem::endShape

  the shape at the end of the item
*/

/*!
  \var KDGanttViewItem::startShapeBack

  the background shape at the beginning of the item
*/

/*!
  \var KDGanttViewItem::midShapeBack

  the background shape in the middle of the item
*/

/*!
  \var KDGanttViewItem::endShapeBack

  the background shape at the end of the item
*/


/*!
  \var KDGanttViewItem::myGanttView

  a pointer to the KDGanttView object to which this item belongs
*/

/*!
  \var KDGanttViewItem::textCanvas

  the text object that is used for this item
*/

/*!
  \var KDGanttViewItem::textCanvasText

  the actual string that is displayed in the text object for this item
*/

/*!
  \var KDGanttViewItem::myStartTime

  the starting time of this item
*/

/*!
  \var KDGanttViewItem::myEndTime

  the ending time of this item
*/

/*!
  \var KDGanttViewItem::isHighlighted

  whether this item is currently highlighted or not
*/

/*!
  \var KDGanttViewItem::isEditable

  whether this item is currently editable or not
*/

/*!
  \var KDGanttViewItem::myItemSize

  the current size of this item
*/

/*!
  \var KDGanttViewItem::blockUpdating

  if true, updates to this item are currently blocked, to reduce
  flicker or speed up redraws
*/

/*!
  \var KDGanttViewItem::isVisibleInGanttView

  this instance variable is true if the item is visible in the Gantt
  view
*/


QPoint KDGanttViewItem::middleLeft()
{
    return QPoint(myGanttView->myTimeHeader->getCoordX(myStartTime), itemPos()+height()/2);
}

QPoint KDGanttViewItem::middleRight()
{
    return QPoint(myGanttView->myTimeHeader->getCoordX(myEndTime), itemPos()+height()/2);
}

/*!
  Moves this items text.
 */
void KDGanttViewItem::moveTextCanvas(int x, int y)
{
    int mx = x + myTextOffset.x();
    int my = y + myTextOffset.y();
    if (myTextOffset.x() != 0)
        mx -= 2*myItemSize; // keep old behaviour

    mTextCanvas->move(mx+2*myItemSize,my-myItemSize/2);
    //qDebug("%s: moveTextCanvas(%d,%d) offset: %d,%d moved to  %d,%d",listViewText(0).toLatin1(),x,y,myTextOffset.x(),myTextOffset.y(),mx+2*myItemSize,my-myItemSize/2);
}

void KDGanttViewItem::moveTextCanvas()
{
    QPoint m = myTextOffset+middleRight();
    mTextCanvas->move(m.x(), m.y()-myItemSize/2);
}

/*!
  Sets with how much the item text is offset from the end of the item.
 */
void KDGanttViewItem::setTextOffset(QPoint p)
{
    //qDebug("%s: setTextOffset() offset: %d,%d",listViewText(0).toLatin1(),p.x(),p.y());
    myTextOffset.setX(p.x());
    myTextOffset.setY(p.y());
}

bool KDGanttViewItem::isMyTextCanvas(QCanvasItem *tc)
{
    return tc == mTextCanvas;
}

/*!
  \fn void KDGanttViewItem::setProgress(int percent)

  Specifies the progress of this item in percent.
  Progress is limited to minimum 0, maximum 100.

  \param percent the progress in percent.
 */

void KDGanttViewItem::setProgress(int percent)
{
    myProgress = qMax(0, percent);
    myProgress = qMin(100, myProgress);
}

/*!
  \fn void KDGanttViewItem::setFloatStartTime(const QDateTime &start)

  Specifies the float start time of this item.
  If the time is invalid, the start float is not shown.

  \param start the float start time
 */
void KDGanttViewItem::setFloatStartTime(const QDateTime &start)
{
    myFloatStartTime = start;
}

/*!
  \fn void KDGanttViewItem::setFloatEndTime(const QDateTime &end)

  Specifies the float end time of this item.
  If the time is invalid, the end float is not shown.

  \param end the float end time
 */
void KDGanttViewItem::setFloatEndTime(const QDateTime &end)
{
    myFloatEndTime = end;
}

void KDGanttViewItem::setMoveable(bool m)
{
  _isMoveable = m;
}

bool KDGanttViewItem::isMoveable() const
{
  return _isMoveable;
}

bool KDGanttViewItem::isResizeable() const
{
  return _isResizeable;
}

void KDGanttViewItem::setResizeable(bool r)
{
  _isResizeable = r;
}
