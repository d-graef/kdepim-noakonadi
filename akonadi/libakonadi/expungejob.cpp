/*
    Copyright (c) 2006 Volker Krause <volker.krause@rwth-aachen.de>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "expungejob.h"

using namespace PIM;

PIM::ExpungeJob::ExpungeJob(QObject * parent) :
    Job( parent )
{
}

PIM::ExpungeJob::~ ExpungeJob()
{
}

void PIM::ExpungeJob::doStart()
{
  writeData( newTag() + " EXPUNGE" );
}

void PIM::ExpungeJob::doHandleResponse(const QByteArray & tag, const QByteArray & data)
{
  Q_UNUSED( tag );
  Q_UNUSED( data );
}

#include "expungejob.moc"
