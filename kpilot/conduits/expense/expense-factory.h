#ifndef _KPILOT_EXPENSE_FACTORY_H
#define _KPILOT_EXPENSE_FACTORY_H
/* expense-factory.h                       KPilot
**
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the factory for the expense-conduit plugin.
** It also defines the class for the behavior of the setup dialog.
*/
 
/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/
 
/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <klibloader.h>

#include "plugin.h"

class KInstance;
class KAboutData;

class ExpenseCSVPage;
class ExpenseDBPage;

class ExpenseWidgetSetup : public ConduitConfig
{
Q_OBJECT
public:
	ExpenseWidgetSetup(QWidget *,const char *,const QStringList &);
	virtual ~ExpenseWidgetSetup();

	virtual void readSettings();

protected:
	virtual void commitChanges();

private:
	ExpenseCSVPage *fCSVPage;
	ExpenseDBPage *fDBPage;
} ;

class ExpenseConduitFactory : public KLibFactory
{
Q_OBJECT

public:
	ExpenseConduitFactory(QObject * = 0L,const char * = 0L);
	virtual ~ExpenseConduitFactory();

	static KAboutData *about() { return fAbout; } ;

protected:
	virtual QObject* createObject( QObject* parent = 0, 
		const char* name = 0, 
		const char* classname = "QObject", 
		const QStringList &args = QStringList() );
private:
	KInstance *fInstance;
	static KAboutData *fAbout;
} ;

extern "C"
{

void *init_libexpenseconduit();

} ;

// $Log$

#endif
