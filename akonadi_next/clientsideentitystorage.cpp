
#include "clientsideentitystorage.h"

#include <QTimer>
#include <QListIterator>

#include <akonadi/collectionfetchjob.h>
#include <akonadi/monitor.h>

#include <kdebug.h>

using namespace Akonadi;

// static ClientSideEntityStorage *sClientSideEntityStorage = 0;

namespace Akonadi{

class ClientSideEntityStoragePrivate
{
public:
  ClientSideEntityStoragePrivate( ClientSideEntityStorage *storage );

  Q_DECLARE_PUBLIC( ClientSideEntityStorage )
  ClientSideEntityStorage *q_ptr;

  QHash< Collection::Id, Collection > m_collections;
  QHash< qint64, Item > m_items;
  QHash< Collection::Id, QList< qint64 > > m_childEntities;

  Monitor *m_monitor;
  int m_entitiesToFetch;
  EntityUpdateAdapter *m_entityUpdateAdapter;
  Collection m_rootCollection;
  QStringList m_mimeTypeFilter;

  void startFirstListJob();

  bool passesFilter( const QStringList &mimetypes );
  bool mimetypeMatches( const QStringList &mimetypes, const QStringList &other );
  Collection getParentCollection( qint64 id );

  void collectionsReceived( const Akonadi::Collection::List& );
  void itemsReceived( const Akonadi::Item::List&, Collection::Id );

  void monitoredCollectionAdded( const Akonadi::Collection&, const Akonadi::Collection& );
  void monitoredCollectionRemoved( const Akonadi::Collection& );
  void monitoredCollectionChanged( const Akonadi::Collection& );
  void monitoredCollectionStatisticsChanged( Akonadi::Collection::Id, const Akonadi::CollectionStatistics &);
  void monitoredCollectionMoved( const Akonadi::Collection&, const Akonadi::Collection&, const Akonadi::Collection&);
  void monitoredItemAdded( const Akonadi::Item&, const Akonadi::Collection& );
  void monitoredItemRemoved( const Akonadi::Item& );
  void monitoredItemChanged( const Akonadi::Item&, const QSet<QByteArray>& );
  void monitoredItemMoved( const Akonadi::Item&, const Akonadi::Collection&, const Akonadi::Collection& );

};

}

ClientSideEntityStoragePrivate::ClientSideEntityStoragePrivate( ClientSideEntityStorage *parent ) : q_ptr( parent )
{
}

void ClientSideEntityStoragePrivate::startFirstListJob()
{
  m_collections.insert( m_rootCollection.id(), m_rootCollection );

  // Includes recursive trees. Lower levels are fetched in the onRowsInserted slot if
  // necessary.
  if (m_entitiesToFetch &
        ( ClientSideEntityStorage::FetchFirstLevelChildCollections | ClientSideEntityStorage::FetchCollectionsRecursive )
     )
  {
    m_entityUpdateAdapter->fetchCollections( m_rootCollection, CollectionFetchJob::FirstLevel );
  }
  if (m_entitiesToFetch & ClientSideEntityStorage::FetchItems)
  {
    m_entityUpdateAdapter->fetchItems( m_rootCollection );
  }
}

ClientSideEntityStorage::ClientSideEntityStorage( Monitor *monitor, EntityUpdateAdapter *entityUpdateAdapter,
QStringList mimetypes,
      Collection rootCollection,
      QObject *parent,
              int entitiesToFetch )
              : QObject( parent ), d_ptr( new ClientSideEntityStoragePrivate( this ) )
{
  Q_D( ClientSideEntityStorage );
  d->m_monitor = monitor;
  d->m_entitiesToFetch = entitiesToFetch;
  d->m_entityUpdateAdapter = entityUpdateAdapter;
  d->m_rootCollection = rootCollection;
  d->m_mimeTypeFilter = mimetypes;
//   sClientSideEntityStorage = this;


  // Signals for collections we retrieve through entityUpdateAdapter.
  connect( entityUpdateAdapter, SIGNAL( collectionsReceived( Akonadi::Collection::List ) ),
           SLOT( collectionsReceived( Akonadi::Collection::List ) ) );
  connect( entityUpdateAdapter, SIGNAL( itemsReceived( Akonadi::Item::List, Collection::Id ) ),
           SLOT( itemsReceived( Akonadi::Item::List, Collection::Id ) ) );

  // monitor collection changes
  connect( monitor, SIGNAL( collectionChanged( const Akonadi::Collection& ) ),
           SLOT( monitoredCollectionChanged( const Akonadi::Collection& ) ) );
  connect( monitor, SIGNAL( collectionAdded( const Akonadi::Collection &, const Akonadi::Collection & ) ),
           SLOT( monitoredCollectionAdded( const Akonadi::Collection &, const Akonadi::Collection & ) ) );
  connect( monitor, SIGNAL( collectionRemoved( const Akonadi::Collection & ) ),
           SLOT( monitoredCollectionRemoved( const Akonadi::Collection &) ) );
  connect( monitor,
            SIGNAL( collectionMoved( const Akonadi::Collection &, const Akonadi::Collection &, const Akonadi::Collection & ) ),
           SLOT( monitoredCollectionMoved( const Akonadi::Collection &, const Akonadi::Collection &, const Akonadi::Collection & ) ) );

  // Monitor item changes.
  connect( monitor, SIGNAL( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ),
           SLOT( monitoredItemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ) );
  connect( monitor, SIGNAL( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ),
           SLOT( monitoredItemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ) );
  connect( monitor, SIGNAL( itemRemoved( const Akonadi::Item& ) ),
           SLOT( monitoredItemRemoved( const Akonadi::Item& ) ) );
  connect( monitor, SIGNAL( itemMoved( const Akonadi::Item, const Akonadi::Collection, const Akonadi::Collection ) ),
           SLOT( monitoredItemMoved( const Akonadi::Item, const Akonadi::Collection, const Akonadi::Collection ) ) );

  connect( monitor, SIGNAL( collectionStatisticsChanged( Akonadi::Collection::Id, const Akonadi::CollectionStatistics &) ),
           SLOT(monitoredCollectionStatisticsChanged( Akonadi::Collection::Id, const Akonadi::CollectionStatistics &) ) );


// TODO:
//   q->connect( monitor, SIGNAL( itemLinked(const Akonadi::Item &item, const Akonadi::Collection &collection)),
//             q, SLOT(itemLinked(const Akonadi::Item &item, const Akonadi::Collection &collection)));
//   q->connect( monitor, SIGNAL( itemUnlinked(const Akonadi::Item &item, const Akonadi::Collection &collection)),
//             q, SLOT(itemUnlinked(const Akonadi::Item &item, const Akonadi::Collection &collection)));


  // Start retrieving collections and items at the next event loop.
  QTimer::singleShot( 0, this, SLOT(startFirstListJob()) );

}

Collection ClientSideEntityStorage::rootCollection()
{
  Q_D( ClientSideEntityStorage );
  return d->m_rootCollection;
}

ClientSideEntityStorage::~ClientSideEntityStorage()
{
  Q_D( ClientSideEntityStorage );
  d->m_collections.clear();
  d->m_items.clear();
  d->m_childEntities.clear();
}

Collection ClientSideEntityStorage::getCollection( Collection::Id id )
{
  Q_D( ClientSideEntityStorage );
  return d->m_collections.value(id);
}

Item ClientSideEntityStorage::getItem( qint64 id )
{
  Q_D( ClientSideEntityStorage );
  if (id > 0) id *= -1;
  return d->m_items.value(id);
}

qint64 ClientSideEntityStorage::internalEntityIdentifier( Item item )
{
  return item.id() * -1;
}

qint64 ClientSideEntityStorage::internalEntityIdentifier( Collection col )
{
  return col.id();
}

int ClientSideEntityStorage::childEntitiesCount( Collection::Id id )
{
  Q_D( ClientSideEntityStorage );
  return d->m_childEntities.value( id ).size();
}

int ClientSideEntityStorage::childItemsCount( Collection::Id id )
{
  Q_D( ClientSideEntityStorage );

  int itemCount = 0;
  QListIterator<qint64> i( d->m_childEntities.value( id ) );
  while (i.hasNext())
  {
    if (i.next() < 0)
    {
      itemCount++;
    }
  }
  return itemCount;
}

int ClientSideEntityStorage::childCollectionsCount( Collection::Id id )
{
  Q_D( ClientSideEntityStorage );

  int colCount = 0;
  QListIterator<qint64> i( d->m_childEntities.value( id ) );
  while (i.hasNext())
  {
    if (i.next() > 0)
    {
      colCount++;
    }
  }
  return colCount;
}

qint64 ClientSideEntityStorage::childAt( Collection::Id id, int internalIndex )
{
  Q_D( ClientSideEntityStorage );
  QList<qint64> list = d->m_childEntities.value( id );
  if (list.size() <= internalIndex )
    return -1;
  return list.at( internalIndex );
}

int ClientSideEntityStorage::indexOf( Collection::Id parent, Collection::Id col )
{
//   if ( col == m_rootCollection.id() )
//     return 0;
  Q_D( ClientSideEntityStorage );

  return d->m_childEntities.value(parent).indexOf(col);
}

int ClientSideEntityStorage::entityTypeForInternalIdentifier( qint64 t )
{
  // TODO: Check that the item exists in the QHash first and return UnknownType if it doesn't exist.
  return (t > 0) ? CollectionType : ItemType ;
}

Collection ClientSideEntityStorage::getParentCollection( Collection col )
{
  Q_D( ClientSideEntityStorage );
  return d->m_collections.value( col.parent() );
}

Collection ClientSideEntityStorage::getParentCollection( Item item )
{
  Q_D( ClientSideEntityStorage );
  return d->getParentCollection( item.id() * -1 );
}

Collection ClientSideEntityStoragePrivate::getParentCollection( qint64 id )
{
  QHashIterator< Collection::Id, QList<qint64> > iter( m_childEntities );
  while ( iter.hasNext() ) {
    iter.next();
    if ( iter.value().contains( id ) ) {
      return m_collections.value( iter.key() );
    }
  }
  return Collection( -1 );
}

void ClientSideEntityStoragePrivate::collectionsReceived( const Akonadi::Collection::List& cols )
{
  Q_Q( ClientSideEntityStorage );
  QHash<Collection::Id, Collection> newCollections;
  QHash< Collection::Id, QList< Collection::Id > > newChildCollections;
  foreach( Collection col, cols ) {
    if ( m_collections.contains( col.id() ) ) {
      // If the collection is already known to the model, we simply update it...
      // Notify akonadi about this? Do I need a col stats job? Or something in entityUpdateAdapter?
//       col.setStatistics( m_collections.value( col.id() ).statistics() );
//       m_collections[ col.id()] = col;
      // Surely this is useless. startIndex and endIndex are the same...
      // Maybe I can check for contiguous indexes and emit in groups.
//       QModelIndex startIndex = indexForId( col.id() );
//       QModelIndex endIndex = indexForId( col.id(), q->columnCount( q->parent( startIndex ) ) - 1 );
//       emit q->dataChanged( startIndex, endIndex );
      continue;
    }
    // ... otherwise we add it to the set of collections we need to handle.
    if ( passesFilter( col.contentMimeTypes() ) ) {
      newChildCollections[ col.parent()].append( col.id() );
      newCollections.insert( col.id(), col );
    }
  }

  // Insert new child collections into model.
  QHashIterator< Collection::Id, QList< Collection::Id > > i( newChildCollections );
  while ( i.hasNext() ) {
    i.next();

    // the key is the parent of new collections.
    Collection::Id parentId = i.key();

    QList< Collection::Id > newChildCols = i.value();
    int newChildCount = newChildCols.size();

    if ( m_collections.contains( parentId ) ) {
      int startRow = 0; // Prepend collections.

//       TODO: account for ordering.
    q->beginInsertEntities(parentId, startRow, startRow + newChildCount - 1 );
      foreach( Collection::Id id, newChildCols ) {
        Collection c = newCollections.value( id );
        kDebug() << id << c.name();
        m_collections.insert( id, c );
        m_childEntities[ parentId ].prepend( id );
      }
    q->endInsertEntities();

      foreach( Collection::Id id, newChildCols ) {
        Collection col = newCollections.value( id );
        if (m_entitiesToFetch & ClientSideEntityStorage::FetchCollectionsRecursive)
        {
          // Fetch the next level of collections if neccessary.
          m_entityUpdateAdapter->fetchCollections( col, CollectionFetchJob::FirstLevel );
        }
        if (m_entitiesToFetch & ClientSideEntityStorage::FetchItems)
        {
          // Fetch items if neccessary.
          m_entityUpdateAdapter->fetchItems( col );
        }
      }


    }
    // TODO: Fetch parent again so that its entities get ordered properly. Or start a modify job?
    // Should I do this for all other cases as well instead of using transactions?
    // Could be a way around the fact that monitor could notify me of things out of order. a parent could
    // be 'changed' for its entities to be reordered before one of its entities is in the model yet.
  }
}

void ClientSideEntityStoragePrivate::itemsReceived( const Akonadi::Item::List& list, Collection::Id colId )
{
  Q_Q( ClientSideEntityStorage );
  Item::List itemsToInsert;
  Item::List itemsToUpdate;

  foreach( Item item, list ) {
    if ( m_items.contains( item.id() * -1 ) ) {
      itemsToUpdate << item;
    } else {
      if ( passesFilter( QStringList() << item.mimeType() ) ) {
        itemsToInsert << item;
      }
    }
  }
  if ( itemsToInsert.size() > 0 ) {
    int startRow = m_childEntities.value( colId ).size();
    q->beginInsertEntities(colId, startRow, startRow + itemsToInsert.size() - 1 );
    foreach( Item item, itemsToInsert ) {
      qint64 itemId = item.id() * -1;
      m_items.insert( itemId, item );
      m_childEntities[ colId ].append( itemId );
    }
    q->endInsertEntities();
  }
  if ( itemsToUpdate.size() > 0 ) {
// TODO: Implement
// ... Or remove. This slot is only for new collections and items fetched from akonadi.
  }
}

void ClientSideEntityStoragePrivate::monitoredCollectionAdded( const Akonadi::Collection& collection, const Akonadi::Collection& parent )
{
  Q_Q( ClientSideEntityStorage );
  if ( !passesFilter( collection.contentMimeTypes() ) )
    return;

  // TODO: Use order attribute of parent if available
  // Otherwise prepend collections and append items. Currently this prepends all collections.

  // Ah! Actually I can prepend and append for single signals, then 'change' the parent.

//   QList<qint64> childCols = m_childEntities.value( parent.id() );
//   int row = childCols.size();
//   int numChildCols = childCollections.value(parent.id()).size();
  int row = 0;

  q->beginInsertEntities( parent.id(), row, row );
  m_collections.insert( collection.id(), collection );
  m_childEntities[ parent.id()].prepend( collection.id() );
  q->endInsertEntities();

}

void ClientSideEntityStoragePrivate::monitoredCollectionRemoved( const Akonadi::Collection& collection )
{
  // TODO: can I be sure child indexes are already gone from the model? I don't think I can be.
  // However, I don't think it matters. Maybe beginRemoveRows takes that into account.
  // Other wise I'll have to persist pending deletes somewhere.
  kDebug() << collection.name() << collection.id();
  Q_Q( ClientSideEntityStorage );

  int row = q->indexOf( collection.parent(), collection.id() );

  q->beginRemoveEntities(collection.parent(), row, row);

  m_collections.remove( collection.id() );                 // Remove deleted collection.
  m_childEntities.remove( collection.id() );               // Remove children of deleted collection.
  m_childEntities[ collection.parent() ].removeAt( row );  // Remove deleted collection from its parent.

  q->endRemoveEntities();
}

void ClientSideEntityStoragePrivate::monitoredCollectionMoved( const Akonadi::Collection& col, const Akonadi::Collection& src, const Akonadi::Collection& dest)
{
  // This should possibly tell the model layoutAboutToBeChanged instead, but
  // Then I think I'd also have to update persistent model indexes, which is troublesome.
  // Instead I remove and add.
  // This has the benefit of making the api of this storage class simpler (no need for move
  // signals, nothing model-specific in the api ) but may make actual models slower, especially when
  // moving collections containing many items.
  // Stephen Kelly, 11 Jan 2009.

  Q_Q( ClientSideEntityStorage );
  int srcRow = q->indexOf( src.id(), col.id() );

  q->beginRemoveEntities( src.id(), srcRow, srcRow );
  Collection c = m_collections.take( col.id() );
  m_childEntities[ src.id()].removeOne( col.id() );
  q->endRemoveEntities();

  int dstRow = 0; // Prepend collections

  q->beginInsertEntities( dest.id(), dstRow, dstRow );
  m_collections.insert( col.id(), col );
  m_childEntities[ dest.id() ].prepend( col.id() );
  q->endInsertEntities();

}

void ClientSideEntityStoragePrivate::monitoredCollectionChanged( const Akonadi::Collection &col)
{
  Q_Q( ClientSideEntityStorage );
  m_collections[ col.id() ] == col;
  q->collectionChanged(col);
}

void ClientSideEntityStoragePrivate::monitoredCollectionStatisticsChanged( Akonadi::Collection::Id collection, const Akonadi::CollectionStatistics &statistics )
{
kDebug();
}

void ClientSideEntityStoragePrivate::monitoredItemAdded( const Akonadi::Item& item, const Akonadi::Collection& col)
{
  Q_Q( ClientSideEntityStorage );

  if ( !passesFilter( QStringList() << item.mimeType() ) )
    return;

  int row = q->childEntitiesCount( col.id() );

  q->beginInsertEntities( col.id(), row, row );
  m_items.insert( item.id() * -1, item );
  m_childEntities[ col.id()].append( item.id() * -1 );
  q->endInsertEntities();

}

void ClientSideEntityStoragePrivate::monitoredItemRemoved( const Akonadi::Item &item )
{
  Q_Q( ClientSideEntityStorage );

  Collection col = q->getParentCollection( item );

  int row = q->indexOf(col.id(), item.id() * -1);

  q->beginRemoveEntities(col.id(), row, row);
  m_items.remove( item.id() * -1 );
  m_childEntities[ col.id() ].removeAt( row );
  q->endRemoveEntities();
}

void ClientSideEntityStoragePrivate::monitoredItemChanged( const Akonadi::Item &item, const QSet<QByteArray> &parts )
{
  Q_Q( ClientSideEntityStorage );
  m_items[ item.id() ] == item;
  q->itemChanged(item, parts);
}

void ClientSideEntityStoragePrivate::monitoredItemMoved( const Akonadi::Item& item,
                  const Akonadi::Collection& src, const Akonadi::Collection& dest )
{
  Q_Q( ClientSideEntityStorage );
  int srcRow = q->indexOf( src.id(), item.id() * -1 );

  q->beginRemoveEntities( src.id(), srcRow, srcRow );
  Item i = m_items.take( item.id() * -1 );
  m_childEntities[ src.id()].removeOne( item.id() * -1 );
  q->endRemoveEntities();

  int dstRow = q->childEntitiesCount( dest.id() );

  q->beginInsertEntities( dest.id(), dstRow, dstRow );
  m_items.insert( item.id() * -1, i );
  m_childEntities[ dest.id()].append( item.id() * -1 );
  q->endInsertEntities();
}

bool ClientSideEntityStoragePrivate::passesFilter( const QStringList &mimetypes )
{
  if (m_mimeTypeFilter.size() == 0)
    return true;
  return mimetypeMatches( m_mimeTypeFilter, mimetypes );
}

bool ClientSideEntityStoragePrivate::mimetypeMatches( const QStringList &mimetypes, const QStringList &other )
{
  QStringList::const_iterator constIterator;
  bool found = false;

  for ( constIterator = mimetypes.constBegin(); constIterator != mimetypes.constEnd(); ++constIterator ) {
    if ( other.contains( ( *constIterator ) ) ) {
      found = true;
      break;
    }
  }
  return found;
}


#include "clientsideentitystorage.moc"
