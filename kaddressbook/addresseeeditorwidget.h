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

#ifndef ADDRESSEEEDITORWIDGET_H
#define ADDRESSEEEDITORWIDGET_H

#include <QtCore/QHash>

#include <kabc/addressee.h>
#include <kdialog.h>

#include "addresseeeditorbase.h"
#include "contacteditorwidgetmanager.h"
#include "extensionwidget.h"
#include "ui_addresseditgeneraltab.h"
#include "ui_addresseditdetailstab.h"

class KLineEdit;
class KSqueezedTextLabel;
class KTabWidget;

class AddressEditWidget;
class EmailEditWidget;
class IMEditWidget;
class PhoneEditWidget;
class SecrecyWidget;


namespace KPIM {
class AddresseeLineEdit;
class CategorySelectDialog;
class CategoryEditDialog;
class KDateEdit;
}

namespace KABC { class AddressBook; }

class AddresseeEditorWidget : public AddresseeEditorBase
{
  Q_OBJECT

  public:
    AddresseeEditorWidget( QWidget *parent );
    ~AddresseeEditorWidget();

    void setAddressee( const KABC::Addressee& );
    const KABC::Addressee &addressee();

    void load();
    void save();

    bool dirty();

    void setInitialFocus();

    bool readyToClose();

  protected Q_SLOTS:
    void textChanged( const QString& );

    /**
      Emits the modified signal and sets the dirty flag. Any slot
      that modifies data should use this method instead of calling emit
      modified() directly.
     */
    void emitModified();

    void dateChanged( const QDate& );
    void invalidDate();
    void nameTextChanged( const QString& );
    void organizationTextChanged( const QString& );
    void nameBoxChanged();
    void nameButtonClicked();
    void selectCategories();

    /**
      Called whenever the categories change in the categories dialog.
     */
    void categoriesSelected( const QStringList& );

    /**
      Edits which categories are available in the CategorySelectDialog.
     */
    void editCategories();

  private:
    void initGUI();
    void setupTab1();
    void setupTab2();
    void setupAdditionalTabs();
    void setupCustomFieldsTabs();

    void setReadOnly( bool );

    KABC::Addressee mAddressee;
    int mFormattedNameType;
    bool mDirty;
    bool mBlockSignals;
    bool mReadOnly;

    // GUI
    KPIM::CategorySelectDialog *mCategorySelectDialog;
    KPIM::CategoryEditDialog *mCategoryEditDialog;
    KTabWidget *mTabWidget;

    // Tab1
    Ui::tab1 tab1;
    IMEditWidget *mIMWidget;
    KSqueezedTextLabel *mNameLabel;

    // Tab2
    Ui::tab2 tab2;

    QHash<QString, ContactEditorTabPage*> mTabPages;
};

#endif
