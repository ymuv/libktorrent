/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
#ifndef NETSPEED_H
#define NETSPEED_H

#include <QPair>
#include <QLinkedList>
#include <util/constants.h>
#include <deque>

namespace net
{

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
		
		Measures the download and upload speed.
	*/
	class Speed
	{
		QAtomicInt rate;
		bt::Uint32 bytes;
#ifndef DO_NOT_USE_DEQUE
		std::deque<QPair<bt::Uint32,bt::TimeStamp> > dlrate;
#else
		QLinkedList<QPair<bt::Uint32,bt::TimeStamp> > dlrate;
#endif
	public:
		Speed();
		virtual ~Speed();
		
		void onData(bt::Uint32 bytes,bt::TimeStamp ts);
		void update(bt::TimeStamp now);
		int getRate() const {return rate;}
	};

}

#endif
