/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include "vcardviewer.h"
#include "kmaddrbook.h"

#include <addresseeview.h>
using KPIM::AddresseeView;

#include <kabc/vcardconverter.h>
#include <kabc/addressee.h>
using KABC::VCardConverter;
using KABC::Addressee;

#include <klocale.h>

#include <qstring.h>

KMail::VCardViewer::VCardViewer(QWidget *parent, const QString& vCard, const char* name)
  : KDialogBase( parent, name, true, i18n("VCard viewer"), User1|Close, Close,
		 true, i18n("&Import"))
{
  mAddresseeView = new AddresseeView(this);
  mAddresseeView->setVScrollBarMode(QScrollView::Auto);
  setMainWidget(mAddresseeView);

  VCardConverter vcc;
  Addressee a;

  // FIXME This is not really a good solution. Would be fine
  // if the VCardConverter could handle this internally.
  bool ok = vcc.vCardToAddressee(vCard, a, VCardConverter::v3_0);
  if (!ok)
    ok = vcc.vCardToAddressee(vCard, a, VCardConverter::v2_1);

  if (ok)
    mAddresseeView->setAddressee(a);
  else {
    mAddresseeView->setText(i18n("Failed to parse vCard!"));
    showButton(User1, false);
  }

  resize(300,400);
}

KMail::VCardViewer::~VCardViewer()
{
}

void KMail::VCardViewer::slotUser1()
{
  if (KMAddrBookExternal::addVCard(mAddresseeView->addressee(), this))
    showButton(User1, false);
}

#include "vcardviewer.moc"
