/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef BTLOGSYSTEMMANAGER_H
#define BTLOGSYSTEMMANAGER_H

#include <QObject>
#include <QMap>
#include <QScopedPointer>
#include <util/constants.h>
#include <ktorrent_export.h>

namespace bt
{

	/**
		Keeps track of all logging system ID's
	*/
	class KTORRENT_EXPORT LogSystemManager : public QObject
	{
		Q_OBJECT
		
		
		LogSystemManager();
	public:
		~LogSystemManager() override;

		/// Register a system
		void registerSystem(const QString & name,Uint32 id);
		
		/// Unregister a system
		void unregisterSystem(const QString & name);
		
		typedef QMap<QString,Uint32>::iterator iterator;
		typedef QMap<QString,Uint32>::const_iterator const_iterator;
		
		iterator begin() {return systems.begin();}
		iterator end() {return systems.end();}

		const_iterator begin() const {return systems.cbegin();}
		const_iterator end() const {return systems.cend();}
		
		static LogSystemManager & instance(); 
		
		/// Get the ID of a system
		Uint32 systemID(const QString & name);
		
	Q_SIGNALS:
		void registered(const QString & name);
		void unregisted(const QString & name);
		
	private:
		QMap<QString,Uint32> systems;
		static QScopedPointer<LogSystemManager> self;
	};

}

#endif
