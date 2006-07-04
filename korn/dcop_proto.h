/*
* Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef DBUS_PROTO_H
#define DBUS_PROTO_H

/**
 * @file
 *
 * This file only have one class: DBUS_Protocol
 */

#include "protocol.h"

class AccountInput;
class KConfigGroup;
class KIO_Protocol;
class KMailDrop;
class Protocol;

class QObject;
class QStringList;
class QString;
class QWidget;

template< class T> class QList;
template< class T> class QVector;
template< class T, class S> class QMap;

//#include <QString>

/**
 * This class implements a DBUS-protocol.
 * DBUS can be used to add messages to a box, or delete created dbus-messages.
 * This can be useful in scripts.
 */
class DBUS_Protocol : public Protocol
{
public:
	/**
	 * Constructor
	 */
	DBUS_Protocol() {}
	/**
	 * Destructor
	 */
	virtual ~DBUS_Protocol() {}

	/**
	 * This function returns a Protocol pointer given a configuration.
	 * This function always returns itself, as the configuration never uses another protocol.
	 */
	virtual const Protocol* getProtocol( KConfigGroup* ) const { return this; }
	/**
	 * This function creates the maildrop used to count dbus-messages.
	 * @param config The configuration
	 */
	virtual KMailDrop* createMaildrop( KConfigGroup* config ) const;
	/**
	 * The function converts the information of the configuration file into a mapping.
	 *
	 * @param config the configuration instance to be mapped
	 * @param password the password as obtained from the KWallet
	 * @return the keys and values of the configuration in a mapping
	 */
	virtual QMap< QString, QString > * createConfig( KConfigGroup* config, const QString& password ) const;
	/**
	 * This return the name of this protocol. It is always "dbus".
	 * @return The name of this protocol: "dbus"
	 */
	virtual QString configName() const;

	/**
	 * This function sets into the list the groupboxes.
	 *
	 * @param list A (empty) list, which is filled with the names of group-boxes.
	 */
	virtual void configFillGroupBoxes( QStringList* list ) const;
	/**
	 * This function filles the configuration field of this protocol.
	 * It is used to construct the configuration dialog.
	 *
	 * @param vector A vector with groupboxes.
	 * @param obj The pointer to the configDialog to connect signals to.
	 * @param result A list with AccountInput which is used to reconstruct the configuration.
	 */
	virtual void configFields( QVector< QWidget* >* vector, const QObject* obj, QList< AccountInput* >* result ) const;
	/**
	 * This function can edit some configuaration option before reading them.
	 */
	virtual void readEntries( QMap< QString, QString >* ) const;
	/**
	 * This function can edit some configuaration option before writing them.
	 */
	virtual void writeEntries( QMap< QString, QString >* ) const;

	//Functions that return a derived class.
	//This way, no explicit cast is needed
	/**
	 * This function returns a cast to a KIO_Protocol. Because this isn't a KIO_Protocol,
	 * it returns 0.
	 *
	 * @return 0
	 */
	virtual const KIO_Protocol* getKIOProtocol() const { return 0; }

	/**
	 * This function returns if the protocol can be rechecked after a certain interval.
	 *
	 * @return true if it is possible to recheck after a certain interval; false otherwise
	 */
	virtual bool isPollable() const { return false; }
};

#endif
