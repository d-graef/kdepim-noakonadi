/*
	libvcard - vCard parsing library for vCard version 3.0
	
	Copyright (C) 1999 Rik Hemsley rik@kde.org
	
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

#ifndef  PARAM_H
#define  PARAM_H

#include <qcstring.h>
#include <qstrlist.h>

#include <VCardEntity.h>

namespace VCARD
{
	
class SubParam
{
	public:
		
		SubParam();
		SubParam(const QCString &, const QCString &);
		
		void setName(const QCString &);
		void setValue(const QCString &);
		
		QCString name();
		QCString value();
	
	private:
		
		QCString name_;
		QCString value_;
};

typedef QList<SubParam> SubParamList;
typedef QListIterator<SubParam> SubParamListIterator;


class Param : public Entity
{

#include "Param-generated.h"
	
	void parseToList();
	
	protected:
		
		SubParamList subParamList_;
};

typedef QList<Param> ParamList;
typedef QListIterator<Param> ParamListIterator;

}

#endif
