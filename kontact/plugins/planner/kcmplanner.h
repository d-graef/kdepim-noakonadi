/*
  This file is part of Kontact.
  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KCMPLANNER_H
#define KCMPLANNER_H

#include "ui_plannerconfig_base.h"
#include <KCModule>

class KCMPlanner : public KCModule, public Ui::PlannerSummaryConfig_Base
{
  Q_OBJECT

  public:
    explicit KCMPlanner( const KComponentData &inst, QWidget *parent = 0 );
    virtual ~KCMPlanner();

    void load();
    void save();
    void defaults();
    const KAboutData *aboutData() const;

  private slots:
    void modified();
    void buttonClicked( bool );
    void customDaysChanged( int value );
};

#endif
