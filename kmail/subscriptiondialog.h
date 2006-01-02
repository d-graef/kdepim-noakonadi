/*  -*- c++ -*-
    subscriptiondialog.h

    This file is part of KMail, the KDE mail client.
    Copyright (C) 2002 Carsten Burghardt <burghardt@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __SUBSCRIPTIONDIALOG
#define __SUBSCRIPTIONDIALOG

#include <q3dict.h>
#include <ksubscription.h>
#include "imapaccountbase.h"

class KMMessage;
class FolderStorage;

namespace KMail {

  class SubscriptionDialog : public KSubscription
  {
    Q_OBJECT

    public:
      SubscriptionDialog( QWidget *parent, const QString &caption, KAccount* acct,
         QString startPath = QString() );

      void show();

    protected:
      /**
       * Find the parent item 
       */ 
      void findParentItem ( QString &name, QString &path, QString &compare,
                       GroupItem **parent, GroupItem **oldItem );

      /**
       * Process the next prefix in mPrefixList
       */ 
      void processNext();

      /**
       * Fill mPrefixList
       */ 
      void initPrefixList();

    public slots:
      /**
       * get the listing from the imap-server
       */ 
      void slotListDirectory(const QStringList&, const QStringList&,
          const QStringList&, const QStringList&, const ImapAccountBase::jobData &);

      /** 
       * called by Ok-button, saves the changes
       */ 
      void slotSave();

      /**
       * Called from the account when a connection was established
       */
      void slotConnectionResult( int errorCode, const QString& errorMsg );      

    protected slots:
      /**
       * Loads the folders
       */ 
      void slotLoadFolders();

      /**
       * Create or update the listitems
       */ 
      void createItems();

    private:
      QString mDelimiter;
      QStringList mFolderNames, mFolderPaths, 
                  mFolderMimeTypes, mFolderAttributes;
      ImapAccountBase::jobData mJobData;
      uint mCount;
      Q3Dict<GroupItem> mItemDict;
      QString mStartPath;
      bool mSubscribed, mForceSubscriptionEnable;
      QStringList mPrefixList;
      QString mCurrentNamespace;
  };

} // namespace KMail

#endif
