#ifndef MK_NNTP_PROTO_H
#define MK_NNTP_PROTO_H

/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "kio_proto.h"

class Nntp_Protocol : public KIO_Protocol
{
public:
	Nntp_Protocol() { }
	virtual ~Nntp_Protocol() { }

	virtual KIO_Protocol * clone() const { return new Nntp_Protocol; }

	virtual bool connectionBased() const { return true; }

	virtual QString protocol() const { return "nntp"; }
	virtual QString configName() const { return "nntp"; }

	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return false; }
	virtual bool canReadMail() const { return true; }
	virtual bool fullMessage() const { return true; }

	virtual unsigned short defaultPort() const { return 119; }
};

#endif
