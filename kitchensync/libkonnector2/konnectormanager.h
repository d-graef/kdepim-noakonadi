/* This file is part of the KDE libraries
    Copyright (C) 2002,2003 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KSYNC_KONNECTOR_MANAGER_H
#define KSYNC_KONNECTOR_MANAGER_H

#include <qobject.h>
#include <qstring.h>

#include <kstaticdeleter.h>

#include "kdevice.h"
#include "filter.h"
#include "error.h"
#include "progress.h"

namespace KSync {
    typedef QString UDI;
    class Kapabilities;
    class ConfigWidget;
    class KonnectorPlugin;
    class KonnectorInfo;

    class KonnectorManager : public QObject {
        Q_OBJECT
        friend class KStaticDeleter<KonnectorManager>;
    public:
        static KonnectorManager* self();
        Device::ValueList query( );
        UDI load( const Device& device );
        UDI load( const QString& deviceName );
        bool unload( const UDI& udi );

        Kapabilities capabilities( const UDI& udi )const;
        void setCapabilities( const UDI& udi,  const Kapabilities& cap );

        ConfigWidget* configWidget( const UDI& udi,
                                    QWidget* parent,
                                    const char* name );
        ConfigWidget* configWidget( const UDI& udi,
                                    const Kapabilities&,
                                    QWidget* parent, const char* name );

        /**
         * resource to be downloaded
         * on sync
         */
        void add( const UDI&, const QString& resource );
        void remove( const UDI&, const QString& res );
        QStringList resources( const UDI& )const;

        /**
         * download a file if possible and emits a signal
         */
        void download( const UDI&,  const QString& resource );

        bool isConnected( const UDI& )const;
        bool connectDevice( const UDI& );
        bool disconnectDevice( const UDI& );

        bool startSync(const UDI& );

        bool startBackup( const UDI&,  const QString& path );
        bool startRestore( const UDI&,  const QString& path );

        KonnectorInfo info( const UDI& )const;

        bool autoLoadFilter()const;

        /**
         * Set whether or not to load
         * Filters automatically
         */
        void setAutoLoadFilter( bool = true );

        /**
         * adds a custom filter
         * a custom filter overwrites
         * automatic loaded filters
         */
        void add( Filter* filter );
        void deleteFilter( Filter* filter );
        const Filter::PtrList filters();

    public slots:
        void write( const QString&, const Syncee::PtrList& );

    signals:
        void sync( const QString&,  Syncee::PtrList );
        void progress( const UDI&, const Progress& );
        void error( const UDI&,  const   Error& );
        void downloaded( const UDI&, Syncee::PtrList );

    private slots:
        void slotSync( const UDI&, Syncee::PtrList );
        void slotProgress( const UDI&, const Progress& );
        void slotError( const UDI&, const Error& );
        void slotDownloaded( const UDI&, Syncee::PtrList );

    private:
        void filter( Syncee::PtrList unknown, Syncee::PtrList& real );

        KonnectorManager();
        ~KonnectorManager();
        Device::ValueList allDevices();
        Device parseDevice( const QString& path );
        Device find( const QString& deviceName );
        KonnectorPlugin* pluginByUDI( const UDI& )const;
        UDI newUDI()const;
        Syncee::PtrList findUnknown( Syncee::PtrList& );
        static KonnectorManager* m_self;
        Filter::PtrList m_filter;
        Filter::PtrList m_filAdded;
        bool m_auto;

        class Private;
        Private *d;
    };

};


#endif
