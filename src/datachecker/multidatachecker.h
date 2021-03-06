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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTMULTIDATACHECKER_H
#define BTMULTIDATACHECKER_H

#include <QString>
#include <QMap>
#include <util/file.h>
#include "datachecker.h"

namespace bt
{

	/**
	@author Joris Guisson
	*/
	class KTORRENT_EXPORT MultiDataChecker : public DataChecker
	{
	public:
		MultiDataChecker(bt::Uint32 from, bt::Uint32 to);
		~MultiDataChecker() override;

		void check(const QString& path, const Torrent& tor,const QString & dnddir,const BitSet & current_status) override;
	private:
		bool loadChunk(Uint32 ci,Uint32 cs,const Torrent & to);
		File::Ptr open(const Torrent & tor,Uint32 idx);
		void closePastFiles(Uint32 min_idx);
		
	private:
		QString cache;
		QString dnd_dir;
		Uint8* buf;
		QMap<Uint32,File::Ptr> files;
	};

}

#endif
