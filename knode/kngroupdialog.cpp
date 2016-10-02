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

#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
//Added by qt3to4:
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>

#include <kdebug.h>
#include <kglobal.h>
#include <kdatepicker.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klineedit.h>

#include "utilities.h"
#include "knnntpaccount.h"
#include "kngroupdialog.h"
#include "knglobals.h"
#include <QPushButton>


KNGroupDialog::KNGroupDialog(QWidget *parent, KNNntpAccount *a) :
  KNGroupBrowser(parent, i18n("Subscribe to Newsgroups"),a, User1 | User2, true, i18n("New &List"), i18n("New &Groups...") )
{
  rightLabel->setText(i18n("Current changes:"));
  subView4=new QTreeWidget(page);
  subView4->setHeaderLabel("subscribe to");
  unsubView4=new QTreeWidget(page);
  unsubView4->setHeaderLabel("unsubscribe from");

  QVBoxLayout *protL=new QVBoxLayout();
  protL->setSpacing(3);
  listL->addLayout(protL, 1,2);
  protL->addWidget(subView4);
  protL->addWidget(unsubView4);

  dir1=right;
  dir2=left;

  connect(groupView4, SIGNAL(itemSelectionChanged()),
    this, SLOT(slotItemSelected4()));
  connect(groupView4, SIGNAL(itemSelectionChanged()),
    this, SLOT(slotSelectionChanged()));

  connect(subView4, SIGNAL(itemSelectionChanged()),
    this, SLOT(slotItemSelected4()));
  connect(unsubView4, SIGNAL(itemSelectionChanged()),
    this, SLOT(slotItemSelected4()));

  connect(arrowBtn1, SIGNAL(clicked()), this, SLOT(slotArrowBtn1()));
  connect(arrowBtn2, SIGNAL(clicked()), this, SLOT(slotArrowBtn2()));

  connect( this, SIGNAL(user1Clicked()), SLOT(slotUser1()) );
  connect( this, SIGNAL(user2Clicked()), SLOT(slotUser2()) );

  KNHelper::restoreWindowSize("groupDlg", this, QSize(662,393));  // optimized for 800x600

  setHelp("anc-fetch-group-list");
}


KNGroupDialog::~KNGroupDialog()
{
  KNHelper::saveWindowSize("groupDlg", this->size());
}


void KNGroupDialog::itemChangedState(CheckItem4 *it, bool s)
{

  kDebug(5003) <<"KNGroupDialog::itemChangedState()";
  if(s){
    if(itemInListView(unsubView4, it->info)) {
      removeListItem(unsubView4, it->info);
      setButtonDirection(btn2, right);
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(true);
    }
    else {
      new GroupItem4(subView4, it->info);
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
  else {
    if(itemInListView(subView4, it->info)) {
      removeListItem(subView4, it->info);
      setButtonDirection(btn1, right);
      arrowBtn1->setEnabled(true);
      arrowBtn2->setEnabled(false);
    }
    else {
      new GroupItem4(unsubView4, it->info);
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
}


void KNGroupDialog::updateItemState(CheckItem4 *it)
{
  it->setChecked( (it->info.subscribed && !itemInListView(unsubView4, it->info)) ||
                  (!it->info.subscribed && itemInListView(subView4, it->info)) );

/*  if((it->info.subscribed || it->info.newGroup) && it->pixmap(0)==0)
    it->setPixmap(0, (it->info.newGroup)? pmNew:pmGroup);   */
}


void KNGroupDialog::toSubscribe(QList<KNGroupInfo> *l)
{
  l->clear();

  bool moderated=false;
  QTreeWidgetItemIterator it(subView4);

    while (*it)
     {
      const KNGroupInfo& info = ( (static_cast<GroupItem4*>(*it))->info );
      l->append(info);
      if (info.status==KNGroup::moderated)
          moderated=true;
       ++it;
     }

  if (moderated)   // warn the user
     KMessageBox::information( knGlobals.topWidget,
       i18n("You have subscribed to a moderated newsgroup.\nYour articles will not appear in the group immediately.\nThey have to go through a moderation process."),
       QString(), "subscribeModeratedWarning" );
}


void KNGroupDialog::toUnsubscribe(QStringList *l)
{
  l->clear();
  QTreeWidgetItemIterator it(unsubView4);

   while (*it)
   {
    l->append( ((static_cast<GroupItem4*>(*it))->info).name );
    ++it;
   }
}


void KNGroupDialog::setButtonDirection(arrowButton b, arrowDirection d)
{
  QPushButton *btn=0;
  if(b==btn1 && dir1!=d) {
    btn=arrowBtn1;
    dir1=d;
  }
  else if(b==btn2 && dir2!=d) {
    btn=arrowBtn2;
    dir2=d;
  }

  if(btn) {
    if(d==right)
      btn->setIcon(pmRight);
    else
      btn->setIcon(pmLeft);
  }
}


void KNGroupDialog::slotItemSelected4()
{

  const QObject *s=sender();

   bool cFlag = false;
   QTreeWidgetItem *it = 0;
   QList<QTreeWidgetItem*> sel_it_list_s=subView4->selectedItems();
   QList<QTreeWidgetItem*> sel_it_list_u=unsubView4->selectedItems();
   QList<QTreeWidgetItem*> sel_it_list_g=groupView4->selectedItems();

   if ( s==subView4 && (sel_it_list_s.count() == 0)) return;
   if ( s==unsubView4 && (sel_it_list_u.count() == 0)) return;
   if ( s==groupView4 && (sel_it_list_g.count() == 0)) return;

   if(s==subView4) {
     it=sel_it_list_s.at(0);
   }
   else if (s==unsubView4) {
     it=sel_it_list_u.at(0);
   }
   else {                                 // must be groupView
     it=sel_it_list_g.at(0);
   }

  if(s==subView4) {
    unsubView4->clearSelection();
    groupView4->clearSelection();
    arrowBtn2->setEnabled(false);
    arrowBtn1->setEnabled(true);
    setButtonDirection(btn1, left);
  }
  else if(s==unsubView4) {
    subView4->clearSelection();
    groupView4->clearSelection();
    arrowBtn1->setEnabled(false);
    arrowBtn2->setEnabled(true);
    setButtonDirection(btn2, left);
  }
  else {
    CheckItem4 *cit;
    subView4->clearSelection();
    unsubView4->clearSelection();
    cit=static_cast<CheckItem4*>(it);
    Qt::CheckState state=cit->checkState(0);
    if (state == Qt::Checked) cFlag = true;
    if(!cFlag && !itemInListView(subView4, cit->info) && !itemInListView(unsubView4, cit->info)) {
      arrowBtn1->setEnabled(true);
      arrowBtn2->setEnabled(false);
      setButtonDirection(btn1, right);
    }
    else if(cFlag && !itemInListView(unsubView4, cit->info) && !itemInListView(subView4, cit->info)) {
      arrowBtn2->setEnabled(true);
      arrowBtn1->setEnabled(false);
      setButtonDirection(btn2, right);
    }
    else {
      arrowBtn1->setEnabled(false);
      arrowBtn2->setEnabled(false);
    }
  }
}

void KNGroupDialog::slotSelectionChanged()
{
  QList<QTreeWidgetItem*> item_list;
  item_list = groupView4->selectedItems();

    if ( item_list.isEmpty() ) {
        arrowBtn1->setEnabled(false);
    } else {
        if (item_list.at(0)->checkState(0) != Qt::Checked) {
          arrowBtn1->setEnabled(true);
        }
    }
}


void KNGroupDialog::slotArrowBtn1()
{
  if(dir1==right) {
    QList<QTreeWidgetItem*> sel_it_list_gw=groupView4->selectedItems();

    if ( sel_it_list_gw.count() != 0 ) {
      GroupItem4 *pGI_gw4=static_cast<GroupItem4*>(sel_it_list_gw.at(0));
      if (pGI_gw4->childCount() == 0) {
      new GroupItem4(subView4, pGI_gw4->info);
      pGI_gw4->setCheckState(0, Qt::Checked);
    }
   }
 }
  else {
    QList<QTreeWidgetItem*> sel_it_list=subView4->selectedItems();
    GroupItem4 *pGI4=static_cast<GroupItem4*>(sel_it_list.at(0));

    if ( sel_it_list.count() != 0 ) {
       changeItemState(pGI4->info, false);
       delete sel_it_list.at(0);
    }
  }
  subView4->clearSelection();
  arrowBtn1->setEnabled(false);
}


void KNGroupDialog::slotArrowBtn2()
{
  if(dir2==right) {
    QList<QTreeWidgetItem*> sel_it_list_gw=groupView4->selectedItems();
    CheckItem4 *pCI4=static_cast<CheckItem4*>(sel_it_list_gw.at(0));

    if (pCI4) {
      new GroupItem4(unsubView4, pCI4->info);
      pCI4->setCheckState(0, Qt::Unchecked);
    }
  }
  else {
    QList<QTreeWidgetItem*> sel_it_list=unsubView4->selectedItems();
    GroupItem4 *pGI4=static_cast<GroupItem4*>(sel_it_list.at(0));
    if (pGI4) {
      changeItemState(pGI4->info, true);
      delete pGI4;
    }
  }
  arrowBtn2->setEnabled(false);
}


// new list
void KNGroupDialog::slotUser1()
{
  leftLabel->setText(i18n("Downloading groups..."));
  enableButton(User1,false);
  enableButton(User2,false);
  emit(fetchList(a_ccount));
}


// new groups
void KNGroupDialog::slotUser2()
{
  QDate lastDate = a_ccount->lastNewFetch();
  KDialog *dlg = new KDialog( this );
  dlg->setCaption( i18n("New Groups") );
  dlg->setButtons( Ok | Cancel );

  QGroupBox *btnGrp = new QGroupBox( i18n("Check for New Groups"), dlg );
  dlg->setMainWidget(btnGrp);
  QGridLayout *topL = new QGridLayout( btnGrp );

  QRadioButton *takeLast = new QRadioButton( i18n("Created since last check:"), btnGrp );
  topL->addWidget(takeLast, 0, 0, 1, 2 );

  QLabel *l = new QLabel(KGlobal::locale()->formatDate(lastDate, KLocale::LongDate),btnGrp);
  topL->addWidget(l, 1, 1, Qt::AlignLeft);

  connect(takeLast, SIGNAL(toggled(bool)), l, SLOT(setEnabled(bool)));

  QRadioButton *takeCustom = new QRadioButton( i18n("Created since this date:"), btnGrp );
  topL->addWidget(takeCustom, 2, 0, 1, 2 );

  dateSel = new KDatePicker( lastDate, btnGrp );
  dateSel->setMinimumSize(dateSel->sizeHint());
  topL->addWidget(dateSel, 3, 1, Qt::AlignLeft);

  connect(takeCustom, SIGNAL(toggled(bool)), this, SLOT(slotDatePickerEnabled(bool)));

  takeLast->setChecked(true);
  dateSel->setEnabled(false);

  topL->addItem( new QSpacerItem(30, 0 ), 0, 0 );

  if (dlg->exec()) {
    if (takeCustom->isChecked())
      lastDate = dateSel->date();
    a_ccount->setLastNewFetch(QDate::currentDate());
    leftLabel->setText(i18n("Checking for new groups..."));
    enableButton(User1,false);
    enableButton(User2,false);
    filterEdit->clear();
    subCB->setChecked(false);
    newCB->setChecked(true);
    emit(checkNew(a_ccount,lastDate));
    incrementalFilter=false;
    slotRefilter();
  }

  delete dlg;
}

void KNGroupDialog::slotDatePickerEnabled( bool b )
{
  dateSel->setEnabled( b );
}


//--------------------------------

#include "kngroupdialog.moc"
