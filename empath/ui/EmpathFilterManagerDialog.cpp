/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// KDE includes
#include <klocale.h>
#include <kapp.h>
#include <kconfig.h>

// Local includes
#include "Empath.h"
#include "RikGroupBox.h"
#include "EmpathDefines.h"
#include "EmpathFilterManagerDialog.h"
#include "EmpathFilterEditDialog.h"
#include "EmpathFilterList.h"
#include "EmpathUtilities.h"
#include "EmpathFilter.h"

bool EmpathFilterManagerDialog::exists_ = false;

	void
EmpathFilterManagerDialog::create()
{
	if (exists_) return;
	exists_ = true;
	EmpathFilterManagerDialog * d = new EmpathFilterManagerDialog(0, 0);
	CHECK_PTR(d);
	d->show();
	kapp->processEvents();
}

EmpathFilterManagerDialog::EmpathFilterManagerDialog(
		QWidget * parent, const char * name)
	:	QDialog(parent, name, false)
{
	empathDebug("ctor");
	setCaption(i18n("Filter Settings - ") + kapp->getCaption());

	mainLayout_ = new QGridLayout(this, 2, 1, 10, 10);
	CHECK_PTR(mainLayout_);
	
	QPushButton	tempButton((QWidget *)0);
	Q_UINT32 h	= tempButton.sizeHint().height();

	rgb_filters_ = new RikGroupBox(QString::null, 8, this, "rgb_filters");
	CHECK_PTR(rgb_filters_);
	
	w_filters_ = new QWidget(rgb_filters_, "w_filters");
	CHECK_PTR(w_filters_);

	rgb_filters_->setWidget(w_filters_);
	
	l_about_ =
		new QLabel(i18n("The following filters will be applied, in order, to new mail."),
				w_filters_, "l_filtersFolder");
	CHECK_PTR(l_about_);

	l_about_->setFixedHeight(h);
	
	lv_filters_ = new QListView(w_filters_, "lv_matches");
	CHECK_PTR(lv_filters_);

	lv_filters_->addColumn(i18n("Filter descriptions"));
	lv_filters_->setFrameStyle(QFrame::Box | QFrame::Sunken);

	filtersButtonBox_ = new KButtonBox(w_filters_, KButtonBox::VERTICAL);
	CHECK_PTR(filtersButtonBox_);

	pb_addFilter_ = filtersButtonBox_->addButton(i18n("Add filter"));
	CHECK_PTR(pb_addFilter_);

	pb_editFilter_ = filtersButtonBox_->addButton(i18n("Edit filter"));
	CHECK_PTR(pb_editFilter_);
	
	pb_removeFilter_ = filtersButtonBox_->addButton(i18n("Delete filter"));
	CHECK_PTR(pb_removeFilter_);
	
	filtersButtonBox_->addStretch();
	
	pb_moveUp_ = filtersButtonBox_->addButton(i18n("Move up"));
	CHECK_PTR(pb_moveUp_);
	
	pb_moveDown_ = filtersButtonBox_->addButton(i18n("Move down"));
	CHECK_PTR(pb_moveDown_);
	
	filtersButtonBox_->addStretch();

	QObject::connect(pb_addFilter_, SIGNAL(clicked()),
			this, SLOT(s_addFilter()));
	
	QObject::connect(pb_editFilter_, SIGNAL(clicked()),
			this, SLOT(s_editFilter()));
	
	QObject::connect(pb_removeFilter_, SIGNAL(clicked()),
			this, SLOT(s_removeFilter()));
	
	QObject::connect(pb_moveUp_, SIGNAL(clicked()),
			this, SLOT(s_moveUp()));
	
	QObject::connect(pb_moveDown_, SIGNAL(clicked()),
			this, SLOT(s_moveDown()));
	
	filtersButtonBox_->layout();
	filtersButtonBox_->setFixedWidth(filtersButtonBox_->sizeHint().width());
	filtersButtonBox_->setMinimumHeight(filtersButtonBox_->sizeHint().height());

///////////////////////////////////////////////////////////////////////////////
// Button box

	buttonBox_	= new KButtonBox(this);
	CHECK_PTR(buttonBox_);

	buttonBox_->setFixedHeight(h);
	
	pb_help_	= buttonBox_->addButton(i18n("&Help"));	
	CHECK_PTR(pb_help_);
	
	buttonBox_->addStretch();
	
	pb_OK_		= buttonBox_->addButton(i18n("&OK"));
	CHECK_PTR(pb_OK_);
	
	pb_OK_->setDefault(true);
	
	pb_apply_	= buttonBox_->addButton(i18n("&Apply"));
	CHECK_PTR(pb_apply_);
	
	pb_cancel_	= buttonBox_->addButton(i18n("&Cancel"));
	CHECK_PTR(pb_cancel_);
	
	buttonBox_->layout();
	
	QObject::connect(pb_OK_,		SIGNAL(clicked()),	SLOT(s_OK()));
	QObject::connect(pb_apply_,		SIGNAL(clicked()),	SLOT(s_apply()));
	QObject::connect(pb_cancel_,	SIGNAL(clicked()),	SLOT(s_cancel()));
	QObject::connect(pb_help_,		SIGNAL(clicked()),	SLOT(s_help()));
/////////////////////////////////////////////////////////////////////////////

	filtersLayout_ = new QGridLayout(w_filters_, 2, 2, 10, 10);
	CHECK_PTR(filtersLayout_);

	filtersLayout_->addMultiCellWidget(l_about_,	0, 0, 0, 1);
	filtersLayout_->addWidget(lv_filters_,			1, 0);
	filtersLayout_->addWidget(filtersButtonBox_,	1, 1);
	
	filtersLayout_->activate();
	
	mainLayout_->addWidget(rgb_filters_,	0, 0);
	mainLayout_->addWidget(buttonBox_,		1, 0);
	mainLayout_->activate();

	setMinimumSize(minimumSizeHint());
	resize(minimumSizeHint());
	
	update();
}

	void
EmpathFilterManagerDialog::s_addFilter()
{
	empathDebug("s_addFilter() called");
	
	EmpathFilter * newFilter = new EmpathFilter(i18n("Unnamed"));
	
	EmpathFilterEditDialog filterEditDialog(newFilter,
			this, "filterEditDialog");
	
	if (filterEditDialog.exec() == Cancel) {
		empathDebug("Deleting unwanted filter");
		delete newFilter;
		return;
	}
	
	// 0 is the first- filter becomes the last when it's added.
	newFilter->setPriority(empath->filterList().count());
	empath->filterList().append(newFilter);
	update();
}

	void
EmpathFilterManagerDialog::s_editFilter()
{
	empathDebug("s_editFilter() called");
	
	EmpathFilter * editedFilter =
		((EmpathFilterListItem *)lv_filters_->currentItem())->filter();
	
	if (editedFilter == 0)
		return;
	
	EmpathFilterEditDialog filterEditDialog(editedFilter,
			this, "filterEditDialog");

	if (filterEditDialog.exec() == Cancel) return;
	
	update();
}

	void
EmpathFilterManagerDialog::s_removeFilter()
{
	empathDebug("s_removeFilter() called");
	
	EmpathFilter * editedFilter =
		((EmpathFilterListItem *)lv_filters_->currentItem())->filter();
	
	if (editedFilter == 0)
		return;
	
	empath->filterList().remove(editedFilter);
	
	update();
}

	void
EmpathFilterManagerDialog::s_moveUp()
{
	empathDebug("s_moveUp() called");
}

	void
EmpathFilterManagerDialog::s_moveDown()
{
	empathDebug("s_moveDown() called");
}

	void
EmpathFilterManagerDialog::update()
{
	lv_filters_->setUpdatesEnabled(false);

	lv_filters_->clear();

	EmpathFilterListIterator it(empath->filterList());

	for (; it.current(); ++it)
		new EmpathFilterListItem(lv_filters_, it.current());
	
	lv_filters_->setUpdatesEnabled(true);
	lv_filters_->triggerUpdate();
}

	void
EmpathFilterManagerDialog::saveData()
{
	empathDebug("saveData() called");
	empath->filterList().save();
}

	void
EmpathFilterManagerDialog::s_OK()
{
	s_apply();
	kapp->getConfig()->sync();
	delete this;
}

	void
EmpathFilterManagerDialog::s_help()
{
	empathInvokeHelp(QString::null, QString::null);
}

	void
EmpathFilterManagerDialog::s_apply()
{
	if (applied_) {
		pb_apply_->setText(i18n("&Apply"));
		kapp->getConfig()->rollback(true);
		kapp->getConfig()->reparseConfiguration();
		applied_ = false;
	} else {
		pb_apply_->setText(i18n("&Revert"));
		pb_cancel_->setText(i18n("&Close"));
		applied_ = true;
	}
	saveData();
}

	void
EmpathFilterManagerDialog::s_cancel()
{
	if (!applied_)
		kapp->getConfig()->rollback(true);
	delete this;
}

	void
EmpathFilterManagerDialog::closeEvent(QCloseEvent * e)
{
	e->accept();
	delete this;
}

