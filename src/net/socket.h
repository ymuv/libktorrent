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
#ifndef NETSOCKET_H
#define NETSOCKET_H

#include <ktorrent_export.h>
#include <QSharedPointer>
#include <net/socketdevice.h>
#include "address.h"

namespace net
{
	const int SEND_FAILURE = 0;
	const int SEND_WOULD_BLOCK = -1;
	
	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class KTORRENT_EXPORT Socket : public SocketDevice
	{
	public:
		explicit Socket(int fd,int ip_version);
		explicit Socket(bool tcp,int ip_version);
		~Socket() override;
		
		void setBlocking(bool on) override;
		bool connectTo(const Address & addr) override;
		bool connectSuccesFull() override;
		void close() override;
		Uint32 bytesAvailable() const override;
		int send(const bt::Uint8* buf,int len) override;
		int recv(bt::Uint8* buf,int max_len) override;
		bool ok() const override {return m_fd >= 0;}
		int fd() const override {return m_fd;}
		bool setTOS(unsigned char type_of_service) override;
		const Address & getPeerName() const override {return addr;}
		Address getSockName() const override;
		
		void reset() override;
		void prepare(Poll* p,Poll::Mode mode) override;
		bool ready(const Poll* p,Poll::Mode mode) const override;
		
		bool bind(const QString & ip,Uint16 port,bool also_listen);
		bool bind(const Address & addr,bool also_listen);
		int accept(Address & a);
		
		int sendTo(const bt::Uint8* buf,int size,const Address & addr);
		int recvFrom(bt::Uint8* buf,int max_size,Address & addr);
		
		bool isIPv4() const {return m_ip_version == 4;}
		bool isIPv6() const {return m_ip_version == 6;}

		/// Take the filedescriptor from the socket
		int take();
		
		typedef QSharedPointer<Socket> Ptr;
		
	private:
		void cacheAddress();
		
	private:
		int m_fd;
		int m_ip_version;
		int r_poll_index;
		int w_poll_index;
	};

}

#endif
