/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2006 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNGROUPBROWSER_H
#define KNGROUPBROWSER_H


#include <QTreeWidget>
//Added by qt3to4:
#include <QIcon>
#include <QPixmap>
#include <QLabel>
#include <QGridLayout>

#include <kdialog.h>

#include "kngroupmanager.h"

class KLineEdit;
class QCheckBox;
class QLabel;
class QGridLayout;

class KNNntpAccount;


/** Base class for group selection dialogs. */
class KNGroupBrowser : public KDialog {

  Q_OBJECT

  public:
    /** Checkable list view item with special handling for displaying moderated groups. */

    class CheckItem4 : public QTreeWidgetItem {

      public:
        CheckItem4(QTreeWidget *v, const KNGroupInfo &gi, KNGroupBrowser *b);
        CheckItem4(QTreeWidgetItem *i, const KNGroupInfo &gi, KNGroupBrowser *b);
        ~CheckItem4();
        void setChecked(bool c);
        void stateChange(QTreeWidgetItem *item, int i);

        KNGroupInfo info;

      protected:
        void stateChange(bool s);
        KNGroupBrowser *browser;

    };

    /** List view item with special handling for displaying moderated groups. */

    class GroupItem4 : public QTreeWidgetItem {

      public:
        GroupItem4(QTreeWidget *v, const KNGroupInfo &gi);
        GroupItem4(QTreeWidgetItem *i, const KNGroupInfo &gi);
        ~GroupItem4();

        KNGroupInfo info;
    };

    KNGroupBrowser( QWidget *parent, const QString &caption, KNNntpAccount *a, ButtonCodes buttons = 0,
                    bool newCBact = false, const QString &user1 = QString(), const QString &user2 = QString() );
    ~KNGroupBrowser();

    KNNntpAccount* account()const      { return a_ccount; }
    virtual void itemChangedState(CheckItem4 *it, bool s)=0;

  public slots:
    void slotReceiveList(KNGroupListData* d);

  signals:
    void loadList(KNNntpAccount *a);

  protected:
    virtual void updateItemState(CheckItem4 *it)=0;
    void changeItemState(const KNGroupInfo &gi, bool s);
    bool itemInListView(QTreeWidget *view, const KNGroupInfo &gi);
    void removeListItem(QTreeWidget *view, const KNGroupInfo &gi);
    void createListItems(QTreeWidgetItem *parent=0);

    QWidget *page;
    QTreeWidget *groupView4;
    int delayedCenter;
    KLineEdit *filterEdit;
    QCheckBox *noTreeCB, *subCB, *newCB;
    QPushButton  *arrowBtn1, *arrowBtn2;
    QPixmap pmGroup, pmNew;
    QIcon pmRight, pmLeft;
    QGridLayout *listL;
    QLabel *leftLabel, *rightLabel;
    QTimer *refilterTimer;
    QString lastFilter;
    bool incrementalFilter;

    KNNntpAccount *a_ccount;
    QList<KNGroupInfo> *allList, *matchList;

  protected slots:
    void slotLoadList();
    void slotItemExpand(QTreeWidgetItem *it);
//    void slotCenterDelayed();
    /** double click checks/unchecks (opens/closes) item */
    void slotFilter(const QString &txt);
    void slotTreeCBToggled();
    void slotSubCBToggled();
    void slotNewCBToggled();
    void slotFilterTextChanged(const QString &txt);
    void slotRefilter();

};

#endif

