/*
    This file is part of KitchenSync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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

#include <QtGui/QFormLayout>

#include <klineedit.h>
#include <klocale.h>

#include "configlocalizationwidget.h"

ConfigLocalizationWidget::ConfigLocalizationWidget( const QSync::PluginLocalization &localization, QWidget *parent )
  : QWidget( parent ),
    mLocalization( localization )
{
  QFormLayout *layout = new QFormLayout( this );

  mEncoding = new KLineEdit( this );
  mTimeZone = new KLineEdit( this );
  mLanguage = new KLineEdit( this );

  layout->addRow( i18n( "Encoding:" ), mEncoding );
  layout->addRow( i18n( "Timezone:" ), mTimeZone );
  layout->addRow( i18n( "Language:" ), mLanguage );
}

void ConfigLocalizationWidget::load()
{
  mEncoding->setText( mLocalization.encoding() );
  mTimeZone->setText( mLocalization.timeZone() );
  mLanguage->setText( mLocalization.language() );
}

void ConfigLocalizationWidget::save()
{
  mLocalization.setEncoding( mEncoding->text() );
  mLocalization.setTimeZone( mTimeZone->text() );
  mLocalization.setLanguage( mLanguage->text() );
};