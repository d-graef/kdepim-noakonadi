/*
 *  dateedit.cpp  -  date entry widget
 *  Program:  kalarm
 *  Copyright © 2002-2008 by David Jarvie <djarvie@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QMouseEvent>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "dateedit.moc"


DateEdit::DateEdit(QWidget* parent)
	: KDateEdit(parent)
{
	connect(this, SIGNAL(dateEntered(const QDate&)), SLOT(newDateEntered(const QDate&)));
}

void DateEdit::setMinDate(const QDate& d, const QString& errorDate)
{
	mMinDate = d;
	if (mMinDate.isValid()  &&  date().isValid()  &&  date() < mMinDate)
		setDate(mMinDate);
	mMinDateErrString = errorDate;
}

void DateEdit::setMaxDate(const QDate& d, const QString& errorDate)
{
	mMaxDate = d;
	if (mMaxDate.isValid()  &&  date().isValid()  &&  date() > mMaxDate)
		setDate(mMaxDate);
	mMaxDateErrString = errorDate;
}

void DateEdit::setInvalid()
{
	setDate(QDate());
}

// Check a new date against any minimum or maximum date.
void DateEdit::newDateEntered(const QDate& newDate)
{
	if (newDate.isValid())
	{
		if (mMinDate.isValid()  &&  newDate < mMinDate)
		{
			pastLimitMessage(mMinDate, mMinDateErrString,
					 ki18nc("@info", "Date cannot be earlier than %1"));
			setDate(mMinDate);
		}
		else if (mMaxDate.isValid()  &&  newDate > mMaxDate)
		{
			pastLimitMessage(mMaxDate, mMaxDateErrString,
					 ki18nc("@info", "Date cannot be later than %1"));
			setDate(mMaxDate);
		}
	}
}

void DateEdit::pastLimitMessage(const QDate& limit, const QString& error, const KLocalizedString& defaultError)
{
	QString errString = error;
	if (errString.isNull())
	{
		if (limit == QDate::currentDate())
			errString = i18nc("@info/plain", "today");
		else
			errString = KGlobal::locale()->formatDate(limit, KLocale::ShortDate);
		errString = defaultError.subs(errString).toString();
	}
	KMessageBox::sorry(this, errString);
}

void DateEdit::mousePressEvent(QMouseEvent *e)
{
	if (isReadOnly())
	{
		// Swallow up the event if it's the left button
		if (e->button() == Qt::LeftButton)
			return;
	}
	KDateEdit::mousePressEvent(e);
}

void DateEdit::mouseReleaseEvent(QMouseEvent* e)
{
	if (!isReadOnly())
		KDateEdit::mouseReleaseEvent(e);
}

void DateEdit::mouseMoveEvent(QMouseEvent* e)
{
	if (!isReadOnly())
		KDateEdit::mouseMoveEvent(e);
}

void DateEdit::keyPressEvent(QKeyEvent* e)
{
	if (!isReadOnly())
		KDateEdit::keyPressEvent(e);
}

void DateEdit::keyReleaseEvent(QKeyEvent* e)
{
	if (!isReadOnly())
		KDateEdit::keyReleaseEvent(e);
}
