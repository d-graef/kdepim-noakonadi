#ifndef MK_MAILDIR_PROTO_H
#define MK_MAILDIR_PROTO_H

#include "kio_proto.h"

/*
 * Protocol for (postfix?) maildir
 * Only tested with a copy of a maildir forder
 */
class Maildir_Protocol : public KIO_Protocol
{
public:
	Maildir_Protocol() {}
	virtual ~Maildir_Protocol() {}

	virtual KIO_Protocol * clone() const { return new Maildir_Protocol; }

	virtual QString protocol() const { return "file"; }
	virtual QString configName() const { return "maildir"; }

	virtual bool canReadSubjects() const { return true; }
	virtual bool canDeleteMail() const { return true; }
	virtual bool canReadMail() const { return true; }
	virtual bool fullMessage() const { return true; }

	virtual bool hasPort() const { return false; }
	virtual bool hasUsername() const { return false; }
	virtual bool hasPassword() const { return false; }
	
	virtual QString serverName() const { return i18n( "Path:" ); }

	virtual void recheckKURL( KURL &kurl, KIO::MetaData & )
	                               { kurl.setPath( kurl.host() + "/." + kurl.path().replace( '/' , '.' ) + "/new" ); kurl.setHost( "" ); }
	virtual void readSubjectKURL( KURL &, KIO::MetaData & ) { }
	virtual void deleteMailKURL( KURL &, KIO::MetaData & )  { }
	virtual void readMailKURL( KURL &, KIO::MetaData & )    { }
};

#endif
