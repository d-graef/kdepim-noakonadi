/* calendarconduit.cc			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "calendarconduit.h"

#include <akonadi/collection.h>
#include <kcal/todo.h>

#include "idmapping.h"
#include "options.h"
#include "pilotTodoEntry.h"
#include "calendarakonadiproxy.h"
#include "calendarakonadirecord.h"
#include "calendarhhrecord.h"
#include "calendarhhdataproxy.h"
#include "calendarsettings.h"

class CalendarConduit::Private
{
public:
	Private()
	{
		fCollectionId = -1;
	}
	
	Akonadi::Collection::Id fCollectionId;
};

CalendarConduit::CalendarConduit( KPilotLink *o, const QVariantList &a )
	: RecordConduit( o, a, CSL1( "DatebookDB" ), CSL1( "Calendar Conduit" ) )
	, d( new CalendarConduit::Private )
{
}

CalendarConduit::~CalendarConduit()
{
	KPILOT_DELETE( d );
}

void CalendarConduit::loadSettings()
{
	FUNCTIONSETUP;
	
	CalendarSettings::self()->readConfig();
	d->fCollectionId = CalendarSettings::akonadiCollection();
}

bool CalendarConduit::initDataProxies()
{
	FUNCTIONSETUP;
	
	if( !fDatabase )
	{
		addSyncLogEntry( i18n( "Error: Handheld database is not loaded." ) );
		return false;
	}
	
	if( d->fCollectionId < 0 )
	{
		addSyncLogEntry( i18n( "Error: No valid akonadi collection configured." ) );
		return false;
	}
	
	// At this point we should be able to read the backup and handheld database.
	// However, it might be that Akonadi is not started.
	CalendarAkonadiProxy* tadp = new CalendarAkonadiProxy( fMapping );
	tadp->setCollectionId( d->fCollectionId );
	 
	fPCDataProxy = tadp;
	fHHDataProxy = new CalendarHHDataProxy( fDatabase );
	fHHDataProxy->loadAllRecords();
	fBackupDataProxy = new CalendarHHDataProxy( fLocalDatabase );
	fBackupDataProxy->loadAllRecords();
	fPCDataProxy->loadAllRecords();
	
	return true;
}

bool CalendarConduit::equal( const Record *pcRec, const HHRecord *hhRec ) const
{
	FUNCTIONSETUP;
	// TODO: Implement
	return false;
	/*
	const CalendarAkonadiRecord* tar = static_cast<const CalendarAkonadiRecord*>( pcRec );
	const CalendarHHRecord* thr = static_cast<const CalendarHHRecord*>( hhRec );
	
	boost::shared_ptr<KCal::Todo> pcTodo
		 = boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>( tar->item().payload<IncidencePtr>() );
	PilotTodoEntry hhTodo = thr->todoEntry();
	
	bool descriptionEqual = pcTodo->summary() == hhTodo.getDescription();
	bool noteEqual = pcTodo->description() == hhTodo.getNote();
	bool categoriesEqual = pcTodo->categories().contains( thr->category() );
	bool completeEqual = pcTodo->isCompleted() == (hhTodo.getComplete() != 0 );
	
	bool dueDateEqual;
	if( pcTodo->hasDueDate() && !hhTodo.getIndefinite() )
	{
		DEBUGKPILOT << "Both have due date";
		dueDateEqual = pcTodo->dtDue().dateTime() == readTm( hhTodo.getDueDate() );
	}
	// This is a bit tricky when getIndefinite() returns true it means that no
	// due date is set.
	else if( pcTodo->hasDueDate() != !hhTodo.getIndefinite() )
	{
		DEBUGKPILOT << "On has and other doesn't have due date. PC[" 
			<< pcTodo->hasDueDate() << "], HH[" << !hhTodo.getIndefinite() << ']';
		dueDateEqual = false;
	}
	else
	{
		DEBUGKPILOT << "Both don't have duedate.";
		dueDateEqual = true;
	}
	
	
	// TODO: Do some mapping for the priority.
	// TODO: Do some mapping for the completed percentage.
	
	DEBUGKPILOT << "descriptionEqual: " << descriptionEqual;
	DEBUGKPILOT << "noteEqual: " << noteEqual;
	DEBUGKPILOT << "categoriesEqual: " << categoriesEqual;
	DEBUGKPILOT << "dueDateEqual: " << dueDateEqual;
	DEBUGKPILOT << "completeEqual: " << completeEqual;
	
	return descriptionEqual
		&& noteEqual
		&& categoriesEqual
		&& dueDateEqual
		&& completeEqual;
	*/
}

Record* CalendarConduit::createPCRecord( const HHRecord *hhRec )
{
	FUNCTIONSETUP;
	// TODO: Implement
/*
	Akonadi::Item item;
	item.setPayload<IncidencePtr>( IncidencePtr( new KCal::Todo() ) );
	item.setMimeType( "application/x-vnd.akonadi.calendar.event" );
		
	Record* rec = new CalendarAkonadiRecord( item, fMapping.lastSyncedDate() );
	copy( hhRec, rec );
	return rec;
	*/
	return 0L;
}

HHRecord* CalendarConduit::createHHRecord( const Record *pcRec )
{
	FUNCTIONSETUP;
	// TODO: Implement
	/*
	HHRecord* hhRec = new CalendarHHRecord( PilotTodoEntry().pack(), "Unfiled" );
	copy( pcRec, hhRec );
	return hhRec;
	*/
	return 0L;
}

void CalendarConduit::_copy( const Record *from, HHRecord *to )
{
	// TODO: Implement
	/*
	const CalendarAkonadiRecord* tar = static_cast<const CalendarAkonadiRecord*>( from );
	CalendarHHRecord* thr = static_cast<CalendarHHRecord*>( to );
	
	boost::shared_ptr<KCal::Todo> pcFrom
		 = boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>( tar->item().payload<IncidencePtr>() );
	
	PilotTodoEntry hhTo = thr->todoEntry();
	
	// set secrecy, start/end times, alarms, recurrence, exceptions, summary and description:
	if( pcFrom->secrecy() != KCal::Todo::SecrecyPublic )
	{
		hhTo.setSecret( true );
	}

	if( pcFrom->hasDueDate() )
	{
		struct tm t = writeTm( pcFrom->dtDue().dateTime() );
		hhTo.setDueDate( t );
		hhTo.setIndefinite( 0 );
	}
	else
	{
		hhTo.setIndefinite( 1 );
	}

	// TODO: Map priority of KCal::Todo to PilotTodoEntry
	// hhTo.setPriority( todo->priority() );

	hhTo.setComplete( pcFrom->isCompleted() );

	// what we call summary pilot calls description.
	hhTo.setDescription( pcFrom->summary() );

	// what we call description pilot puts as a separate note
	hhTo.setNote( pcFrom->description() );
	
	// NOTE: copyCategory( from, to ); is called before _copy( from, to ). Make
	// sure that the CalendarHHRecord::setTodoEntry() keeps the information in the
	// pilot record.

	thr->setTodoEntry( hhTo );
	*/
}

void CalendarConduit::_copy( const HHRecord *from, Record *to  )
{
	// TODO: Implement
	/*
	CalendarAkonadiRecord* tar = static_cast<CalendarAkonadiRecord*>( to );
	const CalendarHHRecord* thr = static_cast<const CalendarHHRecord*>( from );
	
	boost::shared_ptr<KCal::Todo> pcTo
		 = boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>( tar->item().payload<IncidencePtr>() );
		 
	PilotTodoEntry hhFrom = thr->todoEntry();
	
	pcTo->setSecrecy( hhFrom.isSecret() ? KCal::Todo::SecrecyPrivate : KCal::Todo::SecrecyPublic );

	if ( hhFrom.getIndefinite() )
	{
		pcTo->setHasDueDate( false );
	}
	else
	{
		pcTo->setDtDue(KDateTime(readTm(hhFrom.getDueDate()), KDateTime::Spec::LocalZone()));
		pcTo->setHasDueDate( true );
	}

	// PRIORITY //
	// TODO: e->setPriority(de->getPriority());
	
	if( hhFrom.getComplete() && !pcTo->hasCompletedDate() )
	{
		pcTo->setCompleted( KDateTime::currentLocalDateTime() );
	}
	else if( !hhFrom.getComplete() )
	{
		pcTo->setCompleted( false );
	}

	pcTo->setSummary( hhFrom.getDescription() );
	pcTo->setDescription( hhFrom.getNote() );
	
	// This is not needed as we modified the Todo using a pointer. Uncommenting
	// this give problems as there are now two IncidencePtr objects managing the
	// same raw pointer.
	// Akonadi::Item item( tar->item() );
	// item.setPayload<IncidencePtr>( IncidencePtr( pcTo ) );
	// tar->setItem( item );
	*/
}