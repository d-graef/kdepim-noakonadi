// $Id$

#include <qlistview.h>
#include <qpushbutton.h>

#include "categoryselectdialog.h"

#include "koprefs.h"

/* 
 *  Constructs a CategorySelectDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CategorySelectDialog::CategorySelectDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : CategorySelectDialog_base( parent, name, modal, fl )
{
  setCategories();
  
  connect(mButtonEdit,SIGNAL(clicked()),SIGNAL(editCategories()));
}

void CategorySelectDialog::setCategories()
{
  mCategories->clear();

  QStringList::Iterator it;

  for (it = KOPrefs::instance()->mCustomCategories.begin();
       it != KOPrefs::instance()->mCustomCategories.end(); ++it ) {
    new QCheckListItem(mCategories,*it,QCheckListItem::CheckBox);
  }
}


/*  
 *  Destroys the object and frees any allocated resources
 */
CategorySelectDialog::~CategorySelectDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void CategorySelectDialog::setSelected(const QStringList &selList)
{
  clear();

  QStringList::ConstIterator it;
  for (it=selList.begin();it!=selList.end();++it) {
    QCheckListItem *item = (QCheckListItem *)mCategories->firstChild();
    while (item) {
      if (item->text() == *it) {
        item->setOn(true);
        break;
      }
      item = (QCheckListItem *)item->nextSibling();
    }
  }
}

void CategorySelectDialog::slotApply()
{
  QString catStr;
  QCheckListItem *item = (QCheckListItem *)mCategories->firstChild();
  while (item) {
    if (item->isOn()) {
      if (!catStr.isEmpty()) catStr.append(", ");
      catStr.append(item->text());
    }
    item = (QCheckListItem *)item->nextSibling();
  }

  emit categoriesSelected(catStr);
}

void CategorySelectDialog::slotOk()
{
  slotApply();
  accept();
}

void CategorySelectDialog::clear()
{
  QCheckListItem *item = (QCheckListItem *)mCategories->firstChild();
  while (item) {
    item->setOn(false);
    item = (QCheckListItem *)item->nextSibling();
  }  
}

void CategorySelectDialog::updateCategoryConfig()
{
  QStringList selected;
  QCheckListItem *item = (QCheckListItem *)mCategories->firstChild();
  while (item) {
    if (item->isOn()) {
      selected.append(item->text());
    }
    item = (QCheckListItem *)item->nextSibling();
  }

  setCategories();
  
  setSelected(selected);
}
