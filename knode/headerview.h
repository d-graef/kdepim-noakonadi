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

#ifndef KNHEADERVIEW_H
#define KNHEADERVIEW_H

#include <QToolTip>
//Added by qt3to4:
#include <QMouseEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QEvent>

#include <k3listview.h>
#include <kfoldertree.h>
#include <kmime/kmime_util.h>

class KMenu;
class KNHdrViewItem;

/** Header view, displays the article listing of the currently selected
 *  news group or folder.
 */
class KNHeaderView : public K3ListView  {

  Q_OBJECT

  friend class KNHdrViewItem;

  public:
    KNHeaderView( QWidget *parent );
    ~KNHeaderView();

    void setActive( Q3ListViewItem *item );
    void clear();

    void ensureItemVisibleWithMargin( const Q3ListViewItem *i );

    virtual void setSorting( int column, bool ascending = true );
    bool sortByThreadChangeDate() const      { return mSortByThreadChangeDate; }
    void setSortByThreadChangeDate( bool b ) { mSortByThreadChangeDate = b; }

    bool nextUnreadArticle();
    bool nextUnreadThread();

    void readConfig();
    void writeConfig();

    const KPaintInfo* paintInfo() const { return &mPaintInfo; }

  signals:
    void itemSelected( Q3ListViewItem* );
    void doubleClick( Q3ListViewItem* );
    void sortingChanged( int );

  public slots:
    void nextArticle();
    void prevArticle();
    void incCurrentArticle();
    void decCurrentArticle();
    void selectCurrentArticle();

    void toggleColumn( int column, int mode = -1 );
    void prepareForGroup();
    void prepareForFolder();

  protected:
    void activeRemoved()            { mActiveItem = 0; }
    /**
     * Reimplemented to avoid that KListview reloads the alternate
     * background on palette changes.
     */
    virtual bool event( QEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseDoubleClickEvent( QMouseEvent *e );
    void keyPressEvent( QKeyEvent *e );
    bool eventFilter( QObject *, QEvent * );
    virtual Q3DragObject* dragObject();

  private:
    int mSortCol;
    bool mSortAsc;
    bool mSortByThreadChangeDate;
    int mDelayedCenter;
    KNHdrViewItem *mActiveItem;
    KPaintInfo mPaintInfo;
    KMime::DateFormatter mDateFormatter;
    KMenu *mPopup;
    bool mShowingFolder;
    bool mInitDone;

  private slots:
    void slotCenterDelayed();
    void slotSizeChanged( int, int, int );
    void resetCurrentTime();

};

#if 0
class KNHeaderViewToolTip : public QToolTip {

  public:
    KNHeaderViewToolTip( KNHeaderView *parent );

  protected:
    void maybeTip( const QPoint &p );

  private:
    KNHeaderView *listView;

};
#endif

#endif
