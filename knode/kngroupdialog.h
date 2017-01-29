/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNGROUPDIALOG_H
#define KNGROUPDIALOG_H

#include "kngroupbrowser.h"
class KDatePicker;

/** New group subscription dialog. */
class KNGroupDialog : public KNGroupBrowser {

  Q_OBJECT

  public:
    KNGroupDialog(QWidget *parent, KNNntpAccount *a);
    ~KNGroupDialog();

    void toSubscribe(QList<KNGroupInfo> *l);
    void toUnsubscribe(QStringList *l);

  protected:
    enum arrowDirection { right, left };
    enum arrowButton { btn1, btn2 };
    void updateItemState(CheckItem4 *it);
    void itemChangedState(CheckItem4 *it, bool s);
    void setButtonDirection(arrowButton b, arrowDirection d);
    QPushButton *newListBtn;
    QTreeWidget *subView4, *unsubView4;
    arrowDirection dir1, dir2;
    KDatePicker *dateSel;
  protected slots:
    void slotItemSelected4();
    /** deactivates the button when a root item is selected */
    void slotSelectionChanged();
    void slotArrowBtn1();
    void slotArrowBtn2();
    /** new list */
    void slotUser1();
    /** new groups */
  void slotUser2();
  void slotDatePickerEnabled( bool );

  signals:
    void fetchList(KNNntpAccount *a);
    void checkNew(KNNntpAccount *a,QDate date);
};

#endif