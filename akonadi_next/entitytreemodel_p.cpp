/*
    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#include "entitytreemodel_p.h"
#include "entitytreemodel.h"

#include <KDE/KIconLoader>
#include <KDE/KUrl>

#include <akonadi/agentmanager.h>
#include <akonadi/agenttype.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionstatistics.h>
#include <akonadi/collectionstatisticsjob.h>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/monitor.h>
#include <akonadi/session.h>

#include "collectionchildorderattribute.h"

#include <kdebug.h>

using namespace Akonadi;

EntityTreeModelPrivate::EntityTreeModelPrivate( EntityTreeModel *parent )
    : q_ptr( parent ),
      m_collectionFetchStrategy( EntityTreeModel::FetchCollectionsRecursive ),
      m_itemPopulation( EntityTreeModel::ImmediatePopulation ),
      m_includeUnsubscribed( true ),
      m_includeStatistics( false ),
      m_showRootCollection( false )
{
}


int EntityTreeModelPrivate::indexOf( const QList<Node*> &nodes, Entity::Id id ) const
{
  int i = 0;
  foreach ( const Node *node, nodes ) {
    if ( node->id == id )
      return i;
    i++;
  }

  return -1;
}

void EntityTreeModelPrivate::fetchItems( const Collection &parent )
{
  Q_Q( EntityTreeModel );
//   kDebug() << parent.remoteId();
  Akonadi::ItemFetchJob *itemJob = new Akonadi::ItemFetchJob( parent, m_session );
  itemJob->setFetchScope( m_monitor->itemFetchScope() );

  // ### HACK: itemsReceivedFromJob needs to know which collection items were added to.
  // That is not provided by akonadi, so we attach it in a property.
  itemJob->setProperty( ItemFetchCollectionId(), QVariant( parent.id() ) );

  q->connect( itemJob, SIGNAL( itemsReceived( const Akonadi::Item::List& ) ),
              q, SLOT( itemsFetched( const Akonadi::Item::List& ) ) );
  q->connect( itemJob, SIGNAL( result( KJob* ) ),
              q, SLOT( fetchJobDone( KJob* ) ) );
}

void EntityTreeModelPrivate::fetchCollections( const Collection &collection, CollectionFetchJob::Type type )
{
  Q_Q( EntityTreeModel );
  CollectionFetchJob *job = new CollectionFetchJob( collection, type, m_session );
  job->includeUnsubscribed( m_includeUnsubscribed );
  job->includeStatistics( m_includeStatistics );
  q->connect( job, SIGNAL( collectionsReceived( const Akonadi::Collection::List& ) ),
              q, SLOT( collectionsFetched( const Akonadi::Collection::List& ) ) );
  q->connect( job, SIGNAL( result( KJob* ) ),
              q, SLOT( fetchJobDone( KJob* ) ) );
}

void EntityTreeModelPrivate::collectionsFetched( const Akonadi::Collection::List& collections )
{
  // TODO: refactor this stuff into separate methods for listing resources in Collection::root, and listing collections within resources.
  Q_Q( EntityTreeModel );
  QHash<Collection::Id, Collection> newCollections;
  QHash<Collection::Id, QList<Collection::Id> > newChildCollections;

  Akonadi::AgentManager *agentManager = Akonadi::AgentManager::self();

  foreach ( const Collection collection, collections ) {
    if ( m_collections.contains( collection.id() ) ) {
      // If we already know about the collection, there is nothing left to do
      continue;
    }
    // ... otherwise we add it to the set of collections we need to handle.
    if ( collection.parent() == Collection::root().id() ) {
      Akonadi::AgentInstance agentInstance = agentManager->instance( collection.resource() );

      if ( ( !m_mimeChecker.isWantedCollection( collection ) ) &&
           ( !m_monitor->resourcesMonitored().contains( collection.resource().toUtf8() ) ) &&
             !m_monitor->isAllMonitored() )
        continue;
    }

    newChildCollections[ collection.parent() ].append( collection.id() );
    newCollections.insert( collection.id(), collection );
  }

  // Insert new child collections into model.
  QHashIterator<Collection::Id, QList<Collection::Id> > it( newChildCollections );
  while ( it.hasNext() ) {
    it.next();

    // the key is the parent of new collections.
    const Collection::Id parentId = it.key();

    const QList<Collection::Id> newChildCollections = it.value();
    const int newChildCount = newChildCollections.size();

    if ( m_collections.contains( parentId ) ) {
      int startRow = 0; // Prepend collections.

      // TODO: account for ordering.
      const QModelIndex parentIndex = q->indexForCollection( m_collections.value( parentId ) );

      q->beginInsertRows( parentIndex, startRow, startRow + newChildCount - 1 );
      foreach ( const Collection::Id id, newChildCollections ) {
        const Collection collection = newCollections.value( id );
        m_collections.insert( id, collection );

        Node *node = new Node;
        node->id = id;
        node->parent = parentId;
        node->type = Node::Collection;
        m_childEntities[ parentId ].prepend( node );
      }
      q->endInsertRows();

      foreach ( const Collection::Id id, newChildCollections ) {
        const Collection collection = newCollections.value( id );

        // Fetch the next level of collections if necessary.
        if ( m_collectionFetchStrategy == EntityTreeModel::FetchCollectionsRecursive )
          fetchCollections( collection, CollectionFetchJob::FirstLevel );

        // Fetch items if necessary. If we don't fetch them now, we'll wait for an application
        // to request them through EntityTreeModel::fetchMore
        if ( m_itemPopulation == EntityTreeModel::ImmediatePopulation )
          fetchItems( collection );
      }
    }
    // TODO: Fetch parent again so that its entities get ordered properly. Or start a modify job?
    // Should I do this for all other cases as well instead of using transactions?
    // Could be a way around the fact that monitor could notify me of things out of order. a parent could
    // be 'changed' for its entities to be reordered before one of its entities is in the model yet.
  }
}

void EntityTreeModelPrivate::itemsFetched( const Akonadi::Item::List& items )
{
  Q_Q( EntityTreeModel );
  QObject *job = q->sender();
  if ( job ) {
    const Collection::Id collectionId = job->property( ItemFetchCollectionId() ).value<Collection::Id>();
    Item::List itemsToInsert;
    Item::List itemsToUpdate;

    const Collection collection = m_collections.value( collectionId );

    const QList<Node*> collectionEntities = m_childEntities.value( collectionId );
    foreach ( const Item &item, items ) {
      if ( indexOf( collectionEntities, item.id() ) != -1 ) {
        itemsToUpdate << item;
      } else {
        if ( m_mimeChecker.isWantedItem( item ) ) {
          itemsToInsert << item;
        }
      }
    }
    if ( itemsToInsert.size() > 0 ) {
      const int startRow = m_childEntities.value( collectionId ).size();

      const QModelIndex parentIndex = q->indexForCollection( m_collections.value( collectionId ) );
      q->beginInsertRows( parentIndex, startRow, startRow + items.size() - 1 );
      foreach ( const Item &item, items ) {
        Item::Id itemId = item.id();
        m_items.insert( itemId, item );

        Node *node = new Node;
        node->id = itemId;
        node->parent = collectionId;
        node->type = Node::Item;

        m_childEntities[ collectionId ].append( node );
      }
      q->endInsertRows();
    }
  }
}

void EntityTreeModelPrivate::monitoredMimeTypeChanged( const QString & mimeType, bool monitored )
{
  if ( monitored )
    m_mimeChecker.addWantedMimeType( mimeType );
  else
    m_mimeChecker.removeWantedMimeType( mimeType );
}

void EntityTreeModelPrivate::monitoredCollectionAdded( const Akonadi::Collection& collection, const Akonadi::Collection& parent )
{
  Q_Q( EntityTreeModel );
//   if ( !passesFilter( collection.contentMimeTypes() ) )
//     return;

  // TODO: Use order attribute of parent if available
  // Otherwise prepend collections and append items. Currently this prepends all collections.

  // Or I can prepend and append for single signals, then 'change' the parent.

//   QList<qint64> childCols = m_childEntities.value( parent.id() );
//   int row = childCols.size();
//   int numChildCols = childCollections.value(parent.id()).size();

  const int row = 0;

  const QModelIndex parentIndex = q->indexForCollection( parent );
  q->beginInsertRows( parentIndex, row, row );
  m_collections.insert( collection.id(), collection );

  Node *node = new Node;
  node->id = collection.id();
  node->parent = parent.id();
  node->type = Node::Collection;
  m_childEntities[ parent.id() ].prepend( node );
  q->endInsertRows();
}

void EntityTreeModelPrivate::monitoredCollectionRemoved( const Akonadi::Collection& collection )
{
  Q_Q( EntityTreeModel );

  const int row = indexOf( m_childEntities.value( collection.parent() ), collection.id() );

  if ( row < 0 )
    return;

//   int row = m_childEntities.value(collection.parent()).indexOf(collection.id());
  Q_ASSERT( row >= 0 );
  const QModelIndex parentIndex = q->indexForCollection( m_collections.value( collection.parent() ) );

  q->beginRemoveRows( parentIndex, row, row );

  // TODO: Also need to handle all descendant collections and items here.

  // Remove deleted collection.
  m_collections.remove( collection.id() );

  // Remove children of deleted collection.
  m_childEntities.remove( collection.id() );

  // Remove deleted collection from its parent.
  m_childEntities[ collection.parent() ].removeAt( row );

  q->endRemoveRows();
}

void EntityTreeModelPrivate::monitoredCollectionMoved( const Akonadi::Collection& collection,
                                                       const Akonadi::Collection& sourceCollection,
                                                       const Akonadi::Collection& destCollection )
{
  Q_Q( EntityTreeModel );

  const int srcRow = indexOf( m_childEntities.value( sourceCollection.id() ), collection.id() );

  const QModelIndex srcParentIndex = q->indexForCollection( sourceCollection );
  const QModelIndex destParentIndex = q->indexForCollection( destCollection );

  const int destRow = 0; // Prepend collections

  q->beginMoveRows( srcParentIndex, srcRow, srcRow, destParentIndex, destRow );
  Node *node = m_childEntities[ sourceCollection.id() ].takeAt( srcRow );
  m_childEntities[ destCollection.id() ].prepend( node );
  q->endMoveRows();
}

void EntityTreeModelPrivate::monitoredCollectionChanged( const Akonadi::Collection &collection )
{
  Q_Q( EntityTreeModel );

  if ( m_collections.contains( collection.id() ) )
    m_collections[ collection.id() ] = collection;

  const QModelIndex index = q->indexForCollection( collection );
  q->dataChanged( index, index );
}

void EntityTreeModelPrivate::monitoredCollectionStatisticsChanged( Akonadi::Collection::Id id,
                                                                   const Akonadi::CollectionStatistics &statistics )
{
  Q_Q( EntityTreeModel );

  if ( !m_collections.contains( id ) ) {
    kWarning( 5250 ) << "Got statistics response for non-existing collection:" << id;
  } else {
    m_collections[ id ].setStatistics( statistics );

    const QModelIndex index = q->indexForCollection( m_collections[ id ] );
    q->dataChanged( index, index );
  }
}

void EntityTreeModelPrivate::monitoredItemAdded( const Akonadi::Item& item, const Akonadi::Collection& collection )
{
  Q_Q( EntityTreeModel );

  if ( !m_mimeChecker.isWantedItem( item ) )
    return;

  const int row = m_childEntities.value( collection.id() ).size();

  const QModelIndex parentIndex = q->indexForCollection( m_collections.value( collection.id() ) );

  q->beginInsertRows( parentIndex, row, row );
  m_items.insert( item.id(), item );
  Node *node = new Node;
  node->id = item.id();
  node->parent = collection.id();
  node->type = Node::Item;
  m_childEntities[ collection.id() ].append( node );
  q->endInsertRows();
}

void EntityTreeModelPrivate::monitoredItemRemoved( const Akonadi::Item &item )
{
  Q_Q( EntityTreeModel );

  const Collection collection = getParentCollections( item ).at( 0 );

  const int row = indexOf( m_childEntities.value( collection.id() ), item.id() );

  const QModelIndex parentIndex = q->indexForCollection( m_collections.value( collection.id() ) );

  q->beginInsertRows( parentIndex, row, row );
  m_items.remove( item.id() );
  m_childEntities[ collection.id() ].removeAt( row );
  q->endInsertRows();
}

void EntityTreeModelPrivate::monitoredItemChanged( const Akonadi::Item &item, const QSet<QByteArray>& )
{
  Q_Q( EntityTreeModel );
  m_items[ item.id() ] = item;

  const QModelIndexList indexes = q->indexesForItem( item );
  foreach ( const QModelIndex &index, indexes )
    q->dataChanged( index, index );
}

void EntityTreeModelPrivate::monitoredItemMoved( const Akonadi::Item& item,
                                                 const Akonadi::Collection& sourceItem,
                                                 const Akonadi::Collection& destItem )
{
  Q_Q( EntityTreeModel );

  const Item::Id itemId = item.id();

  const int srcRow = indexOf( m_childEntities.value( sourceItem.id() ), itemId );

  const QModelIndex srcIndex = q->indexForCollection( sourceItem );
  const QModelIndex destIndex = q->indexForCollection( destItem );

  // Where should it go? Always append items and prepend collections and reorganize them with separate reactions to Attributes?

  const int destRow = q->rowCount( destIndex );

  q->beginMoveRows( srcIndex, srcRow, srcRow, destIndex, destRow );
  Node *node = m_childEntities[ sourceItem.id() ].takeAt( srcRow );
  m_childEntities[ destItem.id() ].append( node );
  q->endMoveRows();
}

void EntityTreeModelPrivate::monitoredItemLinked( const Akonadi::Item& item, const Akonadi::Collection& collection )
{
  kDebug() << item.remoteId() << collection.id();
  Q_Q( EntityTreeModel );

  if ( !m_mimeChecker.isWantedItem( item ) )
    return;

  const int row = m_childEntities.value( collection.id() ).size();

  const QModelIndex parentIndex = q->indexForCollection( m_collections.value( collection.id() ) );

  q->beginInsertRows( parentIndex, row, row );
  Node *node = new Node;
  node->id = item.id();
  node->parent = collection.id();
  node->type = Node::Item;
  m_childEntities[ collection.id()].append( node );
  q->endInsertRows();
}

void EntityTreeModelPrivate::monitoredItemUnlinked( const Akonadi::Item& item, const Akonadi::Collection& collection )
{
  Q_Q( EntityTreeModel );

  const int row = indexOf( m_childEntities.value( collection.id() ), item.id() );

  const QModelIndex parentIndex = q->indexForCollection( m_collections.value( collection.id() ) );

  q->beginInsertRows( parentIndex, row, row );
  m_childEntities[ collection.id() ].removeAt( row );
  q->endInsertRows();
}

void EntityTreeModelPrivate::fetchJobDone( KJob *job )
{
  if ( job->error() ) {
    kWarning( 5250 ) << "Job error: " << job->errorString() << endl;
  }
}

void EntityTreeModelPrivate::copyJobDone( KJob *job )
{
  if ( job->error() ) {
    kWarning( 5250 ) << "Job error: " << job->errorString() << endl;
  }
}

void EntityTreeModelPrivate::moveJobDone( KJob *job )
{
  if ( job->error() ) {
    kWarning( 5250 ) << "Job error: " << job->errorString() << endl;
  }
}

void EntityTreeModelPrivate::updateJobDone( KJob *job )
{
  if ( job->error() ) {
    // TODO: handle job errors
    kWarning( 5250 ) << "Job error:" << job->errorString();
  } else {
    // TODO: Is this trying to do the job of collectionstatisticschanged?
//     CollectionStatisticsJob *csjob = static_cast<CollectionStatisticsJob*>( job );
//     Collection result = csjob->collection();
//     collectionStatisticsChanged( result.id(), csjob->statistics() );
  }
}

void EntityTreeModelPrivate::startFirstListJob()
{
  Q_Q(EntityTreeModel);

  kDebug() << m_collections.size();

  if (m_collections.size() > 0)
    return;

  Collection rootCollection;
  // Even if the root collection is the invalid collection, we still need to start
  // the first list job with Collection::root.
  if ( m_showRootCollection ) {
    rootCollection = Collection::root();
    // Notify the outside that we're putting collection::root into the model.
//     kDebug() << "begin";
    q->beginInsertRows( QModelIndex(), 0, 0 );
    m_collections.insert( rootCollection.id(), rootCollection );
    m_rootNode = new Node;
    m_rootNode->id = rootCollection.id();
    m_rootNode->parent = -1;
    m_rootNode->type = Node::Collection;
    m_childEntities[ -1 ].append( m_rootNode );
//     kDebug() << "why";
    q->endInsertRows();
  } else {
    // Otherwise store it silently because it's not part of the usable model.
    rootCollection = m_rootCollection;
    m_rootNode = new Node;
    m_rootNode->id = rootCollection.id();
    m_rootNode->parent = -1;
    m_rootNode->type = Node::Collection;
    m_collections.insert( rootCollection.id(), rootCollection );
  }

//   kDebug() << "inserting" << rootCollection.id();

  // Includes recursive trees. Lower levels are fetched in the onRowsInserted slot if
  // necessary.
  if ( ( m_collectionFetchStrategy == EntityTreeModel::FetchFirstLevelChildCollections)
    || ( m_collectionFetchStrategy == EntityTreeModel::FetchCollectionsRecursive ) ) {
    fetchCollections( rootCollection, CollectionFetchJob::FirstLevel );
  }
  // If the root collection is not collection::root, then it could have items, and they will need to be
  // retrieved now.

  if ( m_itemPopulation != EntityTreeModel::NoItemPopulation ) {
//     kDebug() << (rootCollection == Collection::root());
    if (rootCollection != Collection::root())
      fetchItems( rootCollection );
  }
}

Collection EntityTreeModelPrivate::getParentCollection( Entity::Id id ) const
{
  QHashIterator<Collection::Id, QList<Node*> > iter( m_childEntities );
  while ( iter.hasNext() ) {
    iter.next();
    if ( indexOf( iter.value(), id ) != -1 ) {
      return m_collections.value( iter.key() );
    }
  }

  return Collection();
}

Collection::List EntityTreeModelPrivate::getParentCollections( const Item &item ) const
{
  Collection::List list;
  QHashIterator<Collection::Id, QList<Node*> > iter( m_childEntities );
  while ( iter.hasNext() ) {
    iter.next();
    if ( indexOf( iter.value(), item.id() ) != -1 ) {
      list << m_collections.value( iter.key() );
    }
  }

  return list;
}

Collection EntityTreeModelPrivate::getParentCollection( const Collection &collection ) const
{
  return m_collections.value( collection.parent() );
}

Entity::Id EntityTreeModelPrivate::childAt( Collection::Id id, int position, bool *ok ) const
{
  const QList<Node*> list = m_childEntities.value( id );
  if ( list.size() <= position ) {
    *ok = false;
    return 0;
  }

  *ok = true;
  return list.at( position )->id;
}


int EntityTreeModelPrivate::indexOf( Collection::Id parent, Collection::Id collectionId ) const
{
  return indexOf( m_childEntities.value( parent ), collectionId );
}

Item EntityTreeModelPrivate::getItem( Item::Id id) const
{
  if ( id > 0 )
    id *= -1;

  return m_items.value( id );
}
