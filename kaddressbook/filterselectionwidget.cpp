/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "filterselectionwidget.h"

#include <QtGui/QLabel>

#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kvbox.h>

FilterSelectionWidget::FilterSelectionWidget( QWidget *parent )
  : KHBox( parent )
{
  setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Filter:" ), this );
  label->setObjectName( "kde toolbar widget" );

  mFilterCombo = new KComboBox( this );
  mFilterCombo->setMaximumWidth( 200 );
  label->setBuddy( mFilterCombo );
  connect( mFilterCombo, SIGNAL( activated( int ) ),
           this, SIGNAL( filterActivated( int ) ) );
}

FilterSelectionWidget::~FilterSelectionWidget()
{
}

int FilterSelectionWidget::currentItem() const
{
  return mFilterCombo->currentIndex();
}

void FilterSelectionWidget::setCurrentItem( int index )
{
  mFilterCombo->setCurrentIndex( index );
}

void FilterSelectionWidget::setItems( const QStringList &names )
{
  mFilterCombo->clear();
  mFilterCombo->addItems( names );
}

#include "filterselectionwidget.moc"
