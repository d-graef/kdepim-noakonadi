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

#include <QCheckBox>
#include <QTimer>
#include <QApplication>
//Added by qt3to4:
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <kseparator.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>

#include "scheduler.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "knnntpaccount.h"
#include "kngroupbrowser.h"
#include <QLabel>
#include <QPushButton>


KNGroupBrowser::KNGroupBrowser(QWidget *parent, const QString &caption, KNNntpAccount *a,
                               ButtonCodes buttons, bool newCBact, const QString &user1, const QString &user2) :
  KDialog( parent ),
  incrementalFilter(false), a_ccount(a)
{
  setCaption( caption );
  setButtons( buttons | Help | Ok | Cancel );
  setButtonGuiItem( User1, KGuiItem(user1) );
  setButtonGuiItem( User2, KGuiItem(user2) );
  refilterTimer = new QTimer();
  refilterTimer->setSingleShot( true );

  allList=new QList<KNGroupInfo>;
  matchList=new QList<KNGroupInfo>;

  //create Widgets
  page=new QWidget(this);
  setMainWidget(page);

  filterEdit=new KLineEdit(page);
  QLabel *l=new QLabel(i18n("S&earch:"),page);
  l->setBuddy(filterEdit);
  filterEdit->setClearButtonShown( true );
  noTreeCB=new QCheckBox(i18n("Disable &tree view"), page);
  noTreeCB->setChecked(false);
  subCB=new QCheckBox(i18n("&Subscribed only"), page);
  subCB->setChecked(false);
  newCB=new QCheckBox(i18n("&New only"), page);
  if (!newCBact)
    newCB->hide();
  newCB->setChecked(false);
  KSeparator *sep = new KSeparator( Qt::Horizontal, page );

  QFont fnt=font();
  fnt.setBold(true);
  leftLabel=new QLabel(i18n("Loading groups..."),page);
  rightLabel=new QLabel(page);
  leftLabel->setFont(fnt);
  rightLabel->setFont(fnt);

  pmGroup=knGlobals.configManager()->appearance()->icon(KNode::Appearance::group);
  pmNew=knGlobals.configManager()->appearance()->icon(KNode::Appearance::redBall);
  pmRight=KIcon( QApplication::isRightToLeft()? "go-previous": "go-next");
  pmLeft=KIcon(  QApplication::isRightToLeft() ? "go-next" : "go-previous");

  arrowBtn1=new QPushButton(page);
  arrowBtn1->setEnabled(false);
  arrowBtn2=new QPushButton(page);
  arrowBtn2->setEnabled(false);
  arrowBtn1->setIcon(pmRight);
  arrowBtn2->setIcon(pmLeft);
  arrowBtn1->setFixedSize(35,30);
  arrowBtn2->setFixedSize(35,30);

  groupView4=new QTreeWidget(page);
  groupView4->setColumnCount(2);
  QStringList group_view_columns;
  group_view_columns << "Name" << "Description";
  groupView4->setHeaderLabels(group_view_columns);


  //layout
  QGridLayout *topL=new QGridLayout(page);
  topL->setSpacing(5);
  topL->setMargin(0);
  QHBoxLayout *filterL=new QHBoxLayout();
  filterL->setSpacing(10);
  QVBoxLayout *arrL=new QVBoxLayout();
  arrL->setSpacing(10);
  listL=new QGridLayout();
  listL->setSpacing(5);

  topL->addLayout(filterL, 0,0);
  topL->addWidget(sep,1,0);
  topL->addLayout(listL, 2,0);

  filterL->addWidget(l);
  filterL->addWidget(filterEdit, 1);
  filterL->addWidget(noTreeCB);
  filterL->addWidget(subCB);
  if (newCBact)
    filterL->addWidget(newCB);

  listL->addWidget(leftLabel, 0,0);
  listL->addWidget(rightLabel, 0,2);
  listL->addWidget(groupView4, 1,0);
  listL->addLayout(arrL, 1,1);
  listL->setRowStretch(1,1);
  listL->setColumnStretch(0,5);
  listL->setColumnStretch(2,2);

  arrL->addWidget( arrowBtn1, Qt::AlignCenter );
  arrL->addWidget( arrowBtn2, Qt::AlignCenter );

  //connect
  connect(filterEdit, SIGNAL(textChanged(const QString&)),
          SLOT(slotFilterTextChanged(const QString&)));
  connect(groupView4, SIGNAL(itemExpanded(QTreeWidgetItem*)),
          SLOT(slotItemExpand(QTreeWidgetItem*)));


  connect(refilterTimer, SIGNAL(timeout()), SLOT(slotRefilter()));
  connect(noTreeCB, SIGNAL(clicked()), SLOT(slotTreeCBToggled()));
  connect(subCB, SIGNAL(clicked()), SLOT(slotSubCBToggled()));
  connect(newCB, SIGNAL(clicked()), SLOT(slotNewCBToggled()));

  enableButton(User1,false);
  enableButton(User2,false);

  filterEdit->setFocus();

  QTimer::singleShot(2, this, SLOT(slotLoadList()));
}


KNGroupBrowser::~KNGroupBrowser()
{

  knGlobals.scheduler()->cancelJobs( KNJobData::JTLoadGroups );
  knGlobals.scheduler()->cancelJobs( KNJobData::JTFetchGroups );

  delete matchList;
  delete allList;
  delete refilterTimer;
}


void KNGroupBrowser::slotReceiveList(KNGroupListData* d)
{
  enableButton(User1,true);
  enableButton(User2,true);

  if (d) {  // d==0 if something has gone wrong...
    delete allList;
    allList = d->extractList();
    incrementalFilter=false;
    slotRefilter();
  }
}


void KNGroupBrowser::changeItemState(const KNGroupInfo &gi, bool s)
{
  QTreeWidgetItemIterator it(groupView4, QTreeWidgetItemIterator::Checked);

   Qt::CheckState state;
   if (s) {
     state = Qt::Checked;
   } else {
     state = Qt::Unchecked;
   }

   while (*it) {
       if ( static_cast<CheckItem4*>(*it)->info == gi ) {
         static_cast<CheckItem4*>(*it)->setCheckState(0, state);
       }
     ++it;
   }
}


bool KNGroupBrowser::itemInListView(QTreeWidget *view, const KNGroupInfo &gi)
{
  if(!view) return false;

  QTreeWidgetItemIterator it(view);
  while (*it) {
    if ( static_cast<GroupItem4*>(*it)->info==gi ) {
       return true;
    }
    ++it;
  }
  return false;
}


void KNGroupBrowser::createListItems(QTreeWidgetItem *parent)
{
  QString prefix, tlgn, compare;
  QStringList tlgn_list;
  QTreeWidgetItem *it;
  CheckItem4 *cit;

  int colon;
  bool expandit=false;

  if(parent) {
    QTreeWidgetItem *p=parent;
    while(p) {
      prefix.prepend(p->text(0));
      p=p->parent();
    }
  }

  qSort(*matchList);
  Q_FOREACH(const KNGroupInfo& gn, *matchList) {

    if(!prefix.isEmpty() && !gn.name.startsWith(prefix)) {
      if(!compare.isNull())
        break;
      else
        continue;
    }

    compare=gn.name.mid(prefix.length());

    if(!expandit || !compare.startsWith(tlgn)) {
     if( ( colon = compare.indexOf('.') ) != -1 ) {
        colon++;
        expandit=true;
      } else {
        colon=compare.length();
        expandit=false;
      }

      tlgn = compare.left(colon);
      tlgn_list.clear();
      tlgn_list.append(tlgn);

      if(expandit) {
        if(parent) {
          it=new QTreeWidgetItem(parent, tlgn_list);
        }
        else {
          it=new QTreeWidgetItem(groupView4, tlgn_list);
        }
          it->setFlags(Qt::NoItemFlags);
          it->setFlags(Qt::ItemIsEnabled);
          it->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
      }
      else {
        if(parent) {
          cit=new CheckItem4(parent, gn, this);
          cit->setFlags(Qt::NoItemFlags);
          cit->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        }
        else {
          cit=new CheckItem4(groupView4, gn, this);
          cit->setFlags(Qt::NoItemFlags);
          cit->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
        }
        updateItemState(cit);
      }
    }
  }
}


void KNGroupBrowser::removeListItem(QTreeWidget *view, const KNGroupInfo &gi)
{
  if(!view) return;

  QTreeWidgetItemIterator it(view);
  while (*it) {
    if ( static_cast<GroupItem4*>(*it)->info==gi ) {
      delete *it;
      break;
    }
    ++it;
  }
}


void KNGroupBrowser::slotLoadList()
{
  emit(loadList(a_ccount));
}


void KNGroupBrowser::slotItemExpand(QTreeWidgetItem *it)
{
  if(!it) return;

  if(it->childCount()) {
    kDebug(5003) <<"KNGroupBrowser::slotItemExpand() : has already been expanded, returning";
    return;
  }

  createListItems(it);
/*
  // center the item - smart scrolling
  delayedCenter = -1;
  int y = groupView4->itemPos(it);
  int h = it->height();

  if ( (y+h*4+5) >= (groupView4->contentsY()+groupView->visibleHeight()) )
  {
    groupView4->ensureVisible(groupView4->contentsX(), y+h/2, 0, h/2);
    delayedCenter = y+h/2;
    QTimer::singleShot(300, this, SLOT(slotCenterDelayed()));
  }
*/
}

/*
void KNGroupBrowser::slotCenterDelayed()
{
  if (delayedCenter != -1)
    groupView4->ensureVisible(groupView4->contentsX(), delayedCenter, 0, groupView4->visibleHeight()/2);
}
*/

#define MIN_FOR_TREE 200
void KNGroupBrowser::slotFilter(const QString &txt)
{
  QString filtertxt = txt.toLower();
  QRegExp reg(filtertxt, Qt::CaseInsensitive, QRegExp::RegExp);
  CheckItem4 *cit=0;

  bool notCheckSub = !subCB->isChecked();
  bool notCheckNew = !newCB->isChecked();
  bool notCheckStr = (filtertxt.isEmpty());

  bool isRegexp = filtertxt.contains(QRegExp("[^a-z0-9\\-\\+.]"));

  bool doIncrementalUpdate = (!isRegexp && incrementalFilter && (filtertxt.left(lastFilter.length())==lastFilter));

    kDebug() << "Populating view, incremental is " << doIncrementalUpdate;
  if (doIncrementalUpdate) {
    QList<KNGroupInfo> *tempList = new QList<KNGroupInfo>();

    Q_FOREACH(const KNGroupInfo& g, *matchList) {
      if ((notCheckSub||g.subscribed)&&
          (notCheckNew||g.newGroup)&&
          ( notCheckStr || ( g.name.indexOf(filtertxt) != -1 ) ) )
      tempList->append(g);
    }

    delete matchList;
    matchList=tempList;
  } else {
    matchList->clear();

    Q_FOREACH(const KNGroupInfo& g, *allList) {
      if ((notCheckSub||g.subscribed)&&
          (notCheckNew||g.newGroup)&&
          (notCheckStr||(isRegexp? (reg.indexIn(g.name,0) != -1) : ( g.name.indexOf( filtertxt ) != -1 ) )))
        matchList->append(g);
    }
  }

  groupView4->clear();

  if((matchList->count() < MIN_FOR_TREE) || noTreeCB->isChecked()) {
    Q_FOREACH(const KNGroupInfo& g, *matchList) {
      cit=new CheckItem4(groupView4, g, this);
      cit->setFlags(Qt::NoItemFlags);
      cit->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
      updateItemState(cit);
    }
  groupView4->sortItems(0, Qt::AscendingOrder); 
  } else {
    createListItems();
  }

  lastFilter = filtertxt;
  incrementalFilter = !isRegexp;

  leftLabel->setText(i18n("Groups on %1: (%2 displayed)", a_ccount->name(), matchList->count()));

  arrowBtn1->setEnabled(false);
  arrowBtn2->setEnabled(false);
  groupView4->resizeColumnToContents(0);
}


void KNGroupBrowser::slotTreeCBToggled()
{
  incrementalFilter=false;
  slotRefilter();
}


void KNGroupBrowser::slotSubCBToggled()
{
  incrementalFilter=subCB->isChecked();
  slotRefilter();
}


void KNGroupBrowser::slotNewCBToggled()
{
  incrementalFilter=newCB->isChecked();
  slotRefilter();
}


void KNGroupBrowser::slotFilterTextChanged(const QString &)
{
  if (subCB->isChecked() || newCB->isChecked())
    slotRefilter();
  else
    refilterTimer->start(200);
}


void KNGroupBrowser::slotRefilter()
{
  refilterTimer->stop();
  slotFilter(filterEdit->text());
}


//=======================================================================================


KNGroupBrowser::CheckItem4::CheckItem4(QTreeWidget *v, const KNGroupInfo &gi, KNGroupBrowser *b) :
  QTreeWidgetItem(v), info(gi), browser(b)
{
  setText(0, gi.name);      // set group name (column 0)
  QString des(gi.description);
  if (gi.status == KNGroup::moderated) {
    setText(0, gi.name+" (m)");
    if (!des.toUpper().contains(i18n("moderated").toUpper()))
      des+=i18n(" (moderated)");
  }
  setText(1, des);    // description of newsgroup
}


KNGroupBrowser::CheckItem4::CheckItem4(QTreeWidgetItem *i, const KNGroupInfo &gi, KNGroupBrowser *b) :
  QTreeWidgetItem(i), info(gi), browser(b)
{
  setText(0, gi.name);      // set group name (column 0)
  QString des(gi.description);
  if (gi.status == KNGroup::moderated) {
    setText(0, gi.name+" (m)");
    if (!des.toUpper().contains(i18n("moderated").toUpper()))
      des+=i18n(" (moderated)");
  }
  setText(1, des);     // description of newsgroup
}


KNGroupBrowser::CheckItem4::~CheckItem4()
{
}


void KNGroupBrowser::CheckItem4::setChecked(bool c)
{
  KNGroupBrowser *b=browser;
  browser=0;
  Qt::CheckState state(Qt::Unchecked);

  if (c) state=Qt::Checked;

  QTreeWidgetItem::setCheckState(0, state);
  browser=b;
}


void KNGroupBrowser::CheckItem4::stateChange(bool s)
{

// In Qt3 this function is called when checkbox state has changed: http://doc.qt.io/qt-4.8/q3checklistitem.html#stateChange
// No direct substitution in Qt4
// Possible solution:
// Reimplement setData() as described here:
// http://stackoverflow.com/questions/9686648/is-it-possible-to-create-a-signal-for-when-a-qtreewidgetitem-checkbox-is-toggled
// SIGNAL itemChanged() (QTreeWidget) is emitted for any change, including the checkbox. But: how to distinguish?

  if(browser) {
    kDebug(5003) <<"KNGroupBrowser::CheckItem::stateChange()";
    browser->itemChangedState(this, s);
  }
}


//=======================================================================================

KNGroupBrowser::GroupItem4::GroupItem4(QTreeWidget *v, const KNGroupInfo &gi)
  : QTreeWidgetItem(v), info(gi)
{
  setText(0, gi.name);
  if (gi.status == KNGroup::moderated)
    setText(0, gi.name+" (m)");
}

KNGroupBrowser::GroupItem4::GroupItem4(QTreeWidgetItem *i, const KNGroupInfo &gi)
  : QTreeWidgetItem(i), info(gi)
{
  setText(0, gi.name);
}

KNGroupBrowser::GroupItem4::~GroupItem4()
{
}

//-----------------------------------------

#include "kngroupbrowser.moc"
