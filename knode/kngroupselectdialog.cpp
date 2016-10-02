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

#include <QCheckBox>
#include <klocale.h>
#include <kmessagebox.h>

#include "utilities.h"
#include "kngroupselectdialog.h"
#include <QPushButton>


KNGroupSelectDialog::KNGroupSelectDialog(QWidget *parent, KNNntpAccount *a, const QString &act) :
  KNGroupBrowser(parent, i18n("Select Destinations"), a)
{
  selView4=new QTreeWidget(page);
  listL->addWidget(selView4, 1,2);
  rightLabel->setText(i18n("Groups for this article:"));
  subCB->setChecked(true);
  QStringList group_view_columns;
  group_view_columns << "Name";
  selView4->setHeaderLabels(group_view_columns);


  KNGroupInfo info;
  QStringList actGroups = act.split(',',QString::SkipEmptyParts);
  for ( QStringList::Iterator it = actGroups.begin(); it != actGroups.end(); ++it ) {
    info.name = *it;
    new GroupItem4(selView4, info);
  }

  connect(selView4, SIGNAL(itemSelectionChanged()),
    this, SLOT(slotItemSelected()));
  connect(groupView4, SIGNAL(itemSelectionChanged()),
    this, SLOT(slotItemSelected()));
  connect(groupView4, SIGNAL(itemSelectionChanged()),
    this, SLOT(slotSelectionChanged()));


  connect(arrowBtn1, SIGNAL(clicked()), this, SLOT(slotArrowBtn1()));
  connect(arrowBtn2, SIGNAL(clicked()), this, SLOT(slotArrowBtn2()));

  KNHelper::restoreWindowSize("groupSelDlg", this, QSize(659,364));  // optimized for 800x600
}


KNGroupSelectDialog::~KNGroupSelectDialog()
{
  KNHelper::saveWindowSize("groupSelDlg", this->size());
}


void KNGroupSelectDialog::itemChangedState(CheckItem4 *it, bool s)
{
  if(s)
    new GroupItem4(selView4, it->info);
  else
    removeListItem(selView4, it->info);
  arrowBtn1->setEnabled(!s);
}


void KNGroupSelectDialog::updateItemState(CheckItem4 *it)
{
  it->setChecked(itemInListView(selView4, it->info));

//  it->setPixmap(0, pmGroup);    */
}


QString KNGroupSelectDialog::selectedGroups()const
{
  QString ret;
  QTreeWidgetItemIterator it(selView4);

  bool moderated=false;
  int count=0;
  bool isFirst=true;

  while (*it) {
    if (!isFirst)
      ret+=',';
    ret+=(static_cast<GroupItem4*>(*it))->info.name;
    isFirst=false;
    count++;
    if ((static_cast<GroupItem4*>(*it))->info.status == KNGroup::moderated)
      moderated=true;
     it++;
  }

  if (moderated && (count>=2))   // warn the user
     KMessageBox::information(parentWidget(),i18n("You are crossposting to a moderated newsgroup.\nPlease be aware that your article will not appear in any group\nuntil it has been approved by the moderators of the moderated group."),
                              QString(),"crosspostModeratedWarning");
  return ret;
}


void KNGroupSelectDialog::slotItemSelected()
{
  const QObject *s=sender();

  QTreeWidgetItem *it = 0;
  QList<QTreeWidgetItem*> sel_it_list_gv=groupView4->selectedItems();
  QList<QTreeWidgetItem*> sel_it_list_s=selView4->selectedItems();

  if ( s == groupView4 && (sel_it_list_gv.count() == 0)) return;
  if ( s == selView4 && (sel_it_list_s.count() == 0)) return;

  if(s==groupView4) {
     it=sel_it_list_gv.at(0);
   } else if (s==selView4) {
     it=sel_it_list_s.at(0);
   }

  if(s==groupView4) {
    selView4->clearSelection();
    arrowBtn2->setEnabled(false);
    if(it) {
      Qt::CheckState state=it->checkState(0);
      arrowBtn1->setEnabled(!state);
    }
    else {
      arrowBtn1->setEnabled(false);
    }
  }
  else {
    groupView4->clearSelection();
    arrowBtn1->setEnabled(false);
    arrowBtn2->setEnabled((it!=0));
  }
}


void KNGroupSelectDialog::slotSelectionChanged()
{
  QList<QTreeWidgetItem*> sel_it_list_gv=groupView4->selectedItems();

  if (sel_it_list_gv.count() == 0)
    arrowBtn1->setEnabled(false);
}



void KNGroupSelectDialog::slotArrowBtn1()
{
  QList<QTreeWidgetItem*> sel_it_list_gv=groupView4->selectedItems();
  GroupItem4 *i=static_cast<GroupItem4*>(sel_it_list_gv.at(0));

  if(i) {
    new GroupItem4(selView4, i->info);
    arrowBtn1->setEnabled(false);
    i->setCheckState(0, Qt::Checked);
  }
}


void KNGroupSelectDialog::slotArrowBtn2()
{
  QList<QTreeWidgetItem*> sel_it_list_sv=selView4->selectedItems();
  GroupItem4 *i=static_cast<GroupItem4*>(sel_it_list_sv.at(0));

  if(i) {
    changeItemState(i->info, false);
    delete i;
    arrowBtn2->setEnabled(false);
  }
}

// -----------------------------------------------------------------------------

#include "kngroupselectdialog.moc"

