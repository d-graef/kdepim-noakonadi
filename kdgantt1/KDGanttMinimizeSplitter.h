/* -*- Mode: C++ -*-
   $Id: KDGanttMinimizeSplitter.h,v 1.8 2005/10/11 13:59:02 lutz Exp $
*/

/****************************************************************************
 ** Copyright (C)  2001-2004 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDGANTTMINIMIZESPLITTER_H
#define KDGANTTMINIMIZESPLITTER_H


#ifndef QT_H
#include "qframe.h"
#endif // QT_H

#include "kdgantt_qt3_compat.h"

#ifndef QT_NO_SPLITTER

#if QT_VERSION < 0x040000 

#include "qvaluelist.h"

class QSplitterData;
class QSplitterLayoutStruct;

class KDGanttMinimizeSplitter : public QFrame
{
    Q_OBJECT
    Q_ENUMS( Direction )
    Q_PROPERTY( Orientation orientation READ orientation WRITE setOrientation )
    Q_PROPERTY( Direction minimizeDirection READ minimizeDirection WRITE setMinimizeDirection )

public:
    enum ResizeMode { Stretch, KeepSize, FollowSizeHint };
    enum Direction { Left, Right, Up, Down };

    KDGanttMinimizeSplitter( QWidget* parent=0, const char* name=0 );
    KDGanttMinimizeSplitter( Qt::Orientation, QWidget* parent=0, const char* name=0 );
    ~KDGanttMinimizeSplitter();

    virtual void setOrientation( Orientation );
    Orientation orientation() const { return orient; }

    void setMinimizeDirection( Direction );
    Direction minimizeDirection() const;

#if QT_VERSION >= 300
    virtual void setResizeMode( QWidget *w, ResizeMode );
    virtual void setOpaqueResize( bool = true );
    bool opaqueResize() const;

    void moveToFirst( QWidget * );
    void moveToLast( QWidget * );

    void refresh() { recalc( true ); }
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    QValueList<int> sizes() const;
    void setSizes( QValueList<int> );

    void expandPos( int id, int* min, int* max );
protected:
    void childEvent( QChildEvent * );

    bool event( QEvent * );
    void resizeEvent( QResizeEvent * );

    int idAfter( QWidget* ) const;

    void moveSplitter( QCOORD pos, int id );
    virtual void drawSplitter( QPainter*, QCOORD x, QCOORD y,
			       QCOORD w, QCOORD h );
    void styleChange( QStyle& );
    int adjustPos( int , int );
    virtual void setRubberband( int );
    void getRange( int id, int*, int* );

private:
    void init();
    void recalc( bool update = false );
    void doResize();
    void storeSizes();
    void processChildEvents();
    QSplitterLayoutStruct *addWidget( QWidget*, bool first = false );
    void recalcId();
    void moveBefore( int pos, int id, bool upLeft );
    void moveAfter( int pos, int id, bool upLeft );
    void setG( QWidget *w, int p, int s, bool isSplitter = false );

    QCOORD pick( const QPoint &p ) const
    { return orient == Qt::Horizontal ? p.x() : p.y(); }
    QCOORD pick( const QSize &s ) const
    { return orient == Qt::Horizontal ? s.width() : s.height(); }

    QCOORD trans( const QPoint &p ) const
    { return orient == Qt::Vertical ? p.x() : p.y(); }
    QCOORD trans( const QSize &s ) const
    { return orient == Qt::Vertical ? s.width() : s.height(); }

    QSplitterData *data;
#endif

private:
    Qt::Orientation orient;
    Direction _direction;
#ifndef DOXYGEN_SKIP_INTERNAL
    friend class KDGanttSplitterHandle;
#endif
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    KDGanttMinimizeSplitter( const KDGanttMinimizeSplitter & );
    KDGanttMinimizeSplitter& operator=( const KDGanttMinimizeSplitter & );
#endif
};

#ifndef DOXYGEN_SKIP_INTERNAL
// This class was continued from a verbatim copy of the
// QSplitterHandle pertaining to the Qt Enterprise License and the
// GPL. It has only been renamed to KDGanttSplitterHandler in order to
// avoid a symbol clash on some platforms.
class KDGanttSplitterHandle : public QWidget
{
    Q_OBJECT
#if QT_VERSION >= 300
public:
    KDGanttSplitterHandle( Qt::Orientation o,
		       KDGanttMinimizeSplitter *parent, const char* name=0 );
    void setOrientation( Qt::Orientation o );
    Qt::Orientation orientation() const { return orient; }

    bool opaque() const { return s->opaqueResize(); }

    QSize sizeHint() const;

    int id() const { return myId; } // data->list.at(id())->wid == this
    void setId( int i ) { myId = i; }

protected:
    QValueList<QPointArray> buttonRegions();
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    int onButton( const QPoint& p );
    void updateCursor( const QPoint& p );

private:
    Qt::Orientation orient;
    bool opaq;
    int myId;

    KDGanttMinimizeSplitter *s;
    int _activeButton;
    bool _collapsed;
    int _origPos;
#endif
};
#endif

#else  //QT_VERSION < 0x040000

#include <QSplitter>

#define KDGanttMinimizeSplitter QSplitter


#endif // QT_VERSION < 0x040000

#endif // QT_NO_SPLITTER

#endif // KDGANTTMINIMIZESPLITTER_H
