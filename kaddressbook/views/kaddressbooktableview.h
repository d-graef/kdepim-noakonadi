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

#ifndef KADDRESSBOOKTABLEVIEW_H
#define KADDRESSBOOKTABLEVIEW_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QWidget>
#include <Qt3Support/Q3ListView>
#include <Qt3Support/Q3TabDialog>

#include "kaddressbookview.h"

class Q3ListViewItem;
class QVBoxLayout;
class KConfig;
class KIMProxy;
class K3ListView;

class ContactListView;

namespace KABC { class AddressBook; }

/**
 * This class is the table view for kaddressbook. This view is a K3ListView
 * with multiple columns for the selected fields.
 *
 * @short Table View
 * @author Don Sanders <dsanders@kde.org>
 * @version 0.1
 */
class KAddressBookTableView : public KAddressBookView
{
friend class ContactListView;

  Q_OBJECT

  public:
    KAddressBookTableView( KAB::Core *core, QWidget *parent );
    virtual ~KAddressBookTableView();

    virtual void refresh( const QString &uid = QString() );
    virtual QStringList selectedUids();
    virtual void setSelected( const QString &uid = QString(), bool selected = false );
    virtual void setFirstSelected( bool selected = true );
    virtual KABC::Field *sortField() const;

    virtual void readConfig( KConfigGroup&cfg );
    virtual void writeConfig( KConfigGroup &cfg );
    virtual QString type() const { return "Table"; }

    void scrollUp();
    void scrollDown();

  public Q_SLOTS:
    virtual void reconstructListView();

  protected Q_SLOTS:
    /**
      Called whenever the user selects an addressee in the list view.
    */
    void addresseeSelected();

    /**
      Called whenever the user executes an addressee. In terms of the
      list view, this is probably a double click
    */
    void addresseeExecuted( Q3ListViewItem* );

    /**
      RBM menu called.
     */
    void rmbClicked( K3ListView*, Q3ListViewItem*, const QPoint& );

    /**
     * Called to update the presence of a single item
     */
    void updatePresence( const QString &uid );

  private:
    QVBoxLayout *mMainLayout;
    ContactListView *mListView;
    KIMProxy *mIMProxy;
};

#endif
