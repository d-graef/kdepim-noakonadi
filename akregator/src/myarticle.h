/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORMYARTICLE_H
#define AKREGATORMYARTICLE_H

#include <qdom.h>

#include "librss/article.h" /* <rss/article.h> */

class KURLLabel;
class QWidget;

using namespace RSS;

namespace Akregator
{
    /** A proxy class for RSS::Article with some additional methods to assist sorting. */
    class MyArticle
    {
        public:
            enum { Unread=0, Read, New };
            typedef QValueList<MyArticle> List;

            MyArticle();
            MyArticle(Article article);
            MyArticle(const MyArticle &other);
            MyArticle &operator=(const MyArticle &other);
            bool operator==(const MyArticle &other) const;
            bool operator!=(const MyArticle &other) const { return !operator==(other); }
            virtual ~MyArticle();

            int status();
            void setStatus(int s);
            
            /**
             * @return true if this article is the same as other, based on guid or link.
             */
            bool isTheSameAs(const MyArticle &other);

            QString title() const;
            const KURL &link() const;
            QString description() const;
            QString guid() const;
            bool guidIsPermaLink() const;
            const QDateTime &pubDate() const;
            KURLLabel *widget(QWidget *parent = 0, const char *name = 0) const;

            void dumpXmlData( QDomElement parent, QDomDocument doc ) const;
            
        private:
            struct Private;
            Private *d;
            QString buildTitle();
    };
};

#endif
