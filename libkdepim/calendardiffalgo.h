/*
  This file is part of libkdepim.

  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KDEPIM_CALENDARDIFFALGO_H
#define KDEPIM_CALENDARDIFFALGO_H

#include "diffalgo.h"

namespace KCal {
  class Event;
  class Incidence;
  class IncidenceBase;
  class Todo;
}
using namespace KCal;

#include <QList>

namespace KPIM {

class KDEPIM_EXPORT CalendarDiffAlgo : public DiffAlgo
{
  public:
    CalendarDiffAlgo( Incidence *leftIncidence, Incidence *rightIncidence );

    void run();

  private:
    template <class L>
    void diffList( const QString &id, const QList<L> &left, const QList<L> &right );

    void diffIncidenceBase( IncidenceBase *, IncidenceBase * );
    void diffIncidence( Incidence *, Incidence * );
    void diffEvent( Event *, Event * );
    void diffTodo( Todo *, Todo * );

    Incidence *mLeftIncidence;
    Incidence *mRightIncidence;
};

}

#endif
