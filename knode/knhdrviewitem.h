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

#ifndef KNHDRVIEWITEM_H
#define KNHDRVIEWITEM_H

#include <QTreeWidget>
#include "headerview.h"

class KNArticle;
class KNHeaderView;


/** Header view item. */
class KNHdrViewItem : public QTreeWidgetItem  {

  public:
    KNHdrViewItem( KNHeaderView *ref, KNArticle *a = 0 );
    KNHdrViewItem( KNHdrViewItem *ref, KNArticle *a = 0 );
    ~KNHdrViewItem();

    virtual int compare(QTreeWidgetItem *i, int col, bool ascending) const;
    int width(const QFontMetrics &fm, const QTreeWidget *lv, int column) const;

    virtual QString text( int col ) const;

    void expandChildren();
    int depth(QTreeWidgetItem *item);

    void setActive( bool b )  { mActive = b; }
    bool isActive() const     { return mActive; }

    // DND
    QMimeData* dragObject();

    KNArticle *art;
    int countUnreadInThread();

    bool showToolTip( int column ) const { return mShowToolTip[column]; }

  private:
    void init( KNArticle *a );

    bool greyOut();
    bool firstColBold();
    QColor normalColor();
    QColor greyColor();

    bool mActive;
    bool mShowToolTip[5]; // ### hardcoded column count :-(

};

#endif