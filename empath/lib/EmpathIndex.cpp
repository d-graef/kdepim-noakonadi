/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

// Qt includes
#include <qdatastream.h>

// Local includes
#include "EmpathIndexRecord.h"
#include "EmpathIndex.h"
#include "EmpathFolder.h"
#include "Empath.h"

EmpathIndex::EmpathIndex()
    :   blockSize_(1024)
{
    empathDebug("");
    // Empty.
}

EmpathIndex::~EmpathIndex()
{
    empathDebug("");
    _close();
}

    void
EmpathIndex::setFilename(const QString & filename)
{
    empathDebug("");
    filename_ = filename;
    _open();
}

    void
EmpathIndex::setFolder(const EmpathURL & folder)
{
    empathDebug("");
    folder_ = folder;
}

    EmpathIndexRecord *
EmpathIndex::record(const QCString & key)
{
    if (!dbf_) return 0;
    
    datum k;
    k.dptr  = const_cast<char *>(key.data());
    k.dsize = key.length();
    
    datum out = gdbm_fetch(dbf_, k);

    if (out.dptr == NULL)
        return 0;
    
    QByteArray a;
    a.setRawData(out.dptr, out.dsize);
    
    EmpathIndexRecord * rec = new EmpathIndexRecord;

    QDataStream s(a, IO_ReadOnly);
    s >> *rec;
    
    a.resetRawData(out.dptr, out.dsize);
    
    return rec;
}

    Q_UINT32
EmpathIndex::countUnread()
{
    empathDebug("");
    if (!dbf_) return 0;
    Q_UINT32 count;

    datum key = gdbm_firstkey(dbf_);
    
    QByteArray a;
    
    while (key.dptr) {

        datum nextkey = gdbm_nextkey(dbf_, key);
        
#if 0
        a.setRawData(key.dptr, key.dsize);

        EmpathIndexRecord rec;
    
        QDataStream s(a, IO_ReadOnly);
        
        s >> rec;
   
        if (rec.status() & RMM::Read)
#endif
            ++count;
        
        key = nextkey;
    }
    
    empathDebug("done");
    return count;
}

    Q_UINT32
EmpathIndex::count()
{
    empathDebug("");
    if (!dbf_) return 0;
    Q_UINT32 count;
    
    datum key = gdbm_firstkey(dbf_);
    
    QByteArray a;
    
    while (key.dptr) {

        datum nextkey = gdbm_nextkey(dbf_, key);
        ++count;
        key = nextkey;
    }
    
    return count;

}


    void
EmpathIndex::sync()
{
    empathDebug("");
    if (!dbf_) return;
    EmpathFolder * f = empath->folder(folder_);
    
    if (!f) {
        empathDebug("Can't access my folder :(");
        return;
    }
    
    f->update();
}

    void
EmpathIndex::_close()
{
    empathDebug("");
    if (!dbf_) return;
    
    gdbm_close(dbf_);
}

    void
EmpathIndex::_open()
{
    empathDebug(filename_);
    
    dbf_ = gdbm_open(
        filename_.local8Bit().data(), blockSize_, GDBM_WRCREAT, 0600, NULL);
    
    if (!dbf_) {
        empathDebug(gdbm_strerror(gdbm_errno));
    }
}

    QStrList
EmpathIndex::allKeys()
{
    empathDebug("");
    
    QStrList l;   
    if (!dbf_) return l;
 
    datum key = gdbm_firstkey(dbf_);

    while (key.dptr) {
        
        datum nextkey = gdbm_nextkey(dbf_, key);
        
        QCString s(key.dptr,key.dsize);
        
        l.append(s);
        
        key = nextkey;
    }
  
    return l;
}

    bool
EmpathIndex::insert(const QCString & key, EmpathIndexRecord & rec)
{
    if (!dbf_) return false;

    datum k;
    k.dptr  = key.data();
    k.dsize = key.length();
    
    QByteArray a;
    QDataStream s(a, IO_WriteOnly);
    s << rec;
    
    datum d;
    d.dptr  = a.data();
    d.dsize = a.size();
    
    int retval = gdbm_store(dbf_, k, d, GDBM_REPLACE);
    
    return (retval != -1);

}

    bool
EmpathIndex::remove(const QCString & key)
{  
    empathDebug("");
    if (!dbf_) return false;

    datum k;
    
    k.dptr  = const_cast<char *>(key.data());
    k.dsize = key.length();
    
    return (gdbm_delete(dbf_, k) == 0);
}

    void
EmpathIndex::clear()
{
    empathDebug("");
    if (!dbf_) return;
    
    datum key = gdbm_firstkey(dbf_);

    while (key.dptr) {
        
        datum nextkey = gdbm_nextkey(dbf_, key);
        gdbm_delete(dbf_, key);
        key = nextkey;
    }

}

// vim:ts=4:sw=4:tw=78
