/*
    This file is part of KAddressBook.
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

#ifndef RESOURCESELECTION_H
#define RESOURCESELECTION_H

#include <QtGui/QTreeWidget>

#include "extensionwidget.h"

class QAbstractButton;

namespace KABC { class ResourceABC; }

class KABCResourceItem;

class ResourceSelection : public KAB::ExtensionWidget
{
  Q_OBJECT

  public:
    ResourceSelection( KAB::Core*, QWidget *parent );
    virtual ~ResourceSelection();

    QString title() const;
    QString identifier() const;

    void contactsSelectionChanged() {}

  private Q_SLOTS:
    void add();
    void edit();
    void remove();
    void currentChanged( QTreeWidgetItem * );

    void updateView();

    void slotSubresourceAdded( KABC::ResourceABC *resource,
                               const QString& /*type*/,
                               const QString& subResource );
    void slotSubresourceRemoved( KABC::ResourceABC* /*resource*/,
                                 const QString& /*type*/,
                                 const QString& subResource );
    void slotSubresourceChanged( KABC::ResourceABC* /*resource*/,
                                 const QString& /*type*/,
                                 const QString& subResource );

  private:
    void initGUI();

    KABCResourceItem* selectedItem() const;

    KABCResourceItem* findSubResourceItem( KABC::ResourceABC *resource,
                                           const QString &subResource );

    QTreeWidget *mListView;
    QAbstractButton *mAddButton;
    QAbstractButton *mEditButton;
    QAbstractButton *mRemoveButton;

    QString mLastResource;

    KRES::Manager<KABC::Resource> *mManager;
};

#endif
