/* -*- Mode: C++ -*-
   KD Tools - a set of useful widgets for Qt
*/

/****************************************************************************
** Copyright (C) 2005 Klarälvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Tools library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KD Tools licenses may use this file in
** accordance with the KD Tools Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.klaralvdalens-datakonsult.se/?page=products for
**   information about KD Tools Commercial License Agreements.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
** In addition, as a special exception, the copyright holders give
** permission to link the code of this program with any edition of the
** Qt library by Trolltech AS, Norway (or with modified versions of Qt
** that use the same license as Qt), and distribute linked
** combinations including the two.  You must obey the GNU General
** Public License in all respects for all of the code used other than
** Qt.  If you modify this file, you may extend this exception to your
** version of the file, but you are not obligated to do so.  If you do
** not wish to do so, delete this exception statement from your
** version.
**
**********************************************************************/

#include "kdhorizontalline.h"

#include <qstyle.h>
#include <qpainter.h>
#ifdef QT_ACCESSIBILITY_SUPPORT
#include <qaccessible.h>
#endif
#include <qfontmetrics.h>
#include <qapplication.h>

KDHorizontalLine::KDHorizontalLine( QWidget * parent, const char * name, WFlags f )
  : QFrame( parent, name, f ),
    mAlign( Qt::AlignAuto ),
    mLenVisible( 0 )
{
  QFrame::setFrameStyle( HLine | Sunken );
}

KDHorizontalLine::KDHorizontalLine( const QString & title, QWidget * parent, const char * name, WFlags f )
  : QFrame( parent, name, f ),
    mAlign( Qt::AlignAuto ),
    mLenVisible( 0 )
{
  QFrame::setFrameStyle( HLine | Sunken );
  setTitle( title );
}

KDHorizontalLine::~KDHorizontalLine() {}

void KDHorizontalLine::setFrameStyle( int style ) {
  QFrame::setFrameStyle( ( style & ~MShape ) | HLine ); // force HLine
}

void KDHorizontalLine::setTitle( const QString & title ) {
  if ( mTitle == title )
    return;
  mTitle = title;
  calculateFrame();
  update();
  updateGeometry();
#ifdef QT_ACCESSIBILITY_SUPPORT
  QAccessible::updateAccessibility( this, 0, QAccessible::NameChanged );
#endif
}

void KDHorizontalLine::calculateFrame() {
  mLenVisible = mTitle.length();
#if 0
  if ( mLenVisible ) {
    const QFontMetrics fm = fontMetrics();
    while ( mLenVisible ) {
      const int tw = fm.width( mTitle, mLenVisible ) + 4*fm.width(QChar(' '));
      if ( tw < width() )
        break;
      mLenVisible--;
    }
    qDebug( "mLenVisible = %d (of %d)", mLenVisible, mTitle.length() );
    if ( mLenVisible ) { // but do we also have a visible label?
      QRect r = rect();
      const int va = style().styleHint( QStyle::SH_GroupBox_TextLabelVerticalAlignment, this );
      if( va & AlignVCenter )
        r.setTop( fm.height() / 2 );		// frame rect should be
      else if( va & AlignTop )
        r.setTop( fm.ascent() );
      setFrameRect( r );			//   smaller than client rect
      return;
    }
  }
  // no visible label
  setFrameRect( QRect(0,0,0,0) );		//  then use client rect
#endif
}

QSizePolicy KDHorizontalLine::sizePolicy() const {
  return QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}

QSize KDHorizontalLine::sizeHint() const {
  return minimumSizeHint();
}

QSize KDHorizontalLine::minimumSizeHint() const {
  const int w = 2 * 8 // margins on both sides
                + fontMetrics().width( mTitle, mLenVisible )
                + fontMetrics().width( QChar( ' ' ) );
  const int h = fontMetrics().height();
  return QSize( w, h );
}

void KDHorizontalLine::paintEvent( QPaintEvent * e ) {
  QPainter paint( this );

  if ( mLenVisible ) {	// draw title
    const QFontMetrics & fm = paint.fontMetrics();
    const int h = fm.height();
    const int tw = fm.width( mTitle, mLenVisible ) + fm.width(QChar(' '));
    int x;
    const int marg = 8;
    if ( mAlign & AlignHCenter )		// center alignment
      x = frameRect().width()/2 - tw/2;
    else if ( mAlign & AlignRight )	// right alignment
      x = frameRect().width() - tw - marg;
    else if ( mAlign & AlignLeft )       // left alignment
      x = marg;
    else { // auto align
      if( QApplication::reverseLayout() )
        x = frameRect().width() - tw - marg;
      else
        x = marg;
    }
    QRect r( x, 0, tw, h );
    int va = style().styleHint( QStyle::SH_GroupBox_TextLabelVerticalAlignment, this );
    if ( va & AlignTop )
      r.moveBy( 0, fm.descent() );
    const QColor pen( (QRgb) style().styleHint( QStyle::SH_GroupBox_TextLabelColor, this ) );
#if QT_VERSION >= 0x030300
    if ( !style().styleHint( QStyle::SH_UnderlineAccelerator, this ) )
      va |= NoAccel;
#endif
    style().drawItem( &paint, r, ShowPrefix | AlignHCenter | va, colorGroup(),
                      isEnabled(), 0, mTitle, -1, ownPalette() ? 0 : &pen );
    paint.setClipRegion( e->region().subtract( r ) ); // clip everything but title
  }
  drawFrame( &paint );
  drawContents( &paint );
}

#include "kdhorizontalline.moc"
