/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KORG_CELLITEM_H
#define KORG_CELLITEM_H

#include "korganizer/korganizer_export.h"
#include <QString>
#include <QList>

namespace KOrg {

class KORGANIZER_CORE_EXPORT CellItem
{
  public:
    CellItem()
      : mSubCells( 0 ), mSubCell( -1 )
    {
    }
    virtual ~CellItem() {}
    void setSubCells( int v ) { mSubCells = v; }
    int subCells() const { return mSubCells; }

    void setSubCell( int v ) { mSubCell = v; }
    int subCell() const { return mSubCell; }

    virtual bool overlaps( CellItem *other ) const = 0;

    virtual QString label() const;

    /**
      Place item @p placeItem into stripe containing items @p cells in a
      way that items don't overlap.
      @param cells The list of other cell items to be laid out parallel to the placeItem.
      @param placeItem The item to be laid out.

      @return Placed items
    */
    static QList<CellItem*> placeItem( QList<CellItem*> cells, CellItem *placeItem );

  private:
    int mSubCells;
    int mSubCell;
};

}

#endif
