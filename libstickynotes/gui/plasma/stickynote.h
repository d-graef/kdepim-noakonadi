/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. <gamaral@amaral.com.mx>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
 
#ifndef PLASMA_APPLET_STICKYNOTE_H
#define PLASMA_APPLET_STICKYNOTE_H

#include <Plasma/Applet>

#include <QAbstractSocket>

namespace Plasma { class Label; }

namespace StickyNotes {
	class BaseNoteItem;
	class RemoteNoteController;
	class RemoteNoteItem;
}

class QTcpSocket;
class QTimer;

class StickyNote : public Plasma::Applet
{
Q_OBJECT

public:
	StickyNote(QObject *_parent, const QVariantList &_args);
	virtual ~StickyNote(void);

	virtual void init(void);
	virtual void paintInterface(QPainter *_painter,
	    const QStyleOptionGraphicsItem *_option,
	    const QRect& _rect);

private:
	void clear(void);
	void reset(void);

private slots:
	void on_controller_createdItem(StickyNotes::RemoteNoteItem &_item);
	void on_socket_connected(void);
	void on_socket_error(QAbstractSocket::SocketError _error);
	void on_timer_timeout(void);

private:
	StickyNotes::RemoteNoteController *m_controller;
	QSet<StickyNotes::BaseNoteItem *> m_roots;
	QTcpSocket *m_socket;
	QTimer     *m_timer;
};
 
#endif // ! PLASMA_APPLET_STICKYNOTE_H

