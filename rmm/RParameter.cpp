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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <qstring.h>
#include <RMM_Parameter.h>
#include <RMM_Token.h>

RParameter::RParameter()
	:	RMessageComponent()
{
	rmmDebug("ctor");
}

RParameter::RParameter(const RParameter & p)
	:	RMessageComponent(p)
{
	rmmDebug("ctor");
}

RParameter::RParameter(const QCString & s)
	:	RMessageComponent(s)
{
	rmmDebug("ctor with \"" + s + "\"");
}

RParameter::~RParameter()
{
	rmmDebug("dtor");
}

	RParameter &
RParameter::operator = (const QCString & s)
{
	RMessageComponent::operator = (s);
}

	RParameter &
RParameter::operator = (const RParameter & p)
{
	rmmDebug("operator =");
    
	if (this == &p) return *this; // Don't do a = a.
	
	attribute_ = p.attribute_;
	value_ = p.value_;

	RMessageComponent::operator = (p);
	return *this;
}

	void
RParameter::parse()   
{
	if (parsed_) return;
	rmmDebug("parse() called");
	
	int split = strRep_.find('=');
	
	if (split == -1) {
		rmmDebug("Invalid parameter");
		return;
	}
	
	attribute_	= strRep_.left(split).stripWhiteSpace();
	value_		= strRep_.right(strRep_.length() - split - 1).stripWhiteSpace();
	
	parsed_		= true;
	assembled_	= false;
}

	void
RParameter::assemble()
{
	parse();
	if (assembled_) return;
	rmmDebug("assemble() called");
	strRep_ = attribute_ + "=" + value_;
	assembled_ = true;
}

	void
RParameter::createDefault()
{
	attribute_ = value_ = "";
}

	QCString
RParameter::attribute()
{
	parse();
	return attribute_;
}

	QCString
RParameter::value()
{
	parse();
	return value_;
}

	void
RParameter::setAttribute(const QCString & attribute)
{
	attribute_ = attribute;
	assembled_ = false;
}
	void
RParameter::setValue(const QCString & value)	
{
	value_ = value;
	assembled_ = false;
}
	
