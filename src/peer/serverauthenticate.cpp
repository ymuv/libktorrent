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
#include "serverauthenticate.h"
#include <mse/encryptedpacketsocket.h>
#include <util/sha1hash.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include "peermanager.h"
#include "peerid.h"
#include <torrent/torrent.h>
#include <peer/accessmanager.h>


namespace bt
{
	bool ServerAuthenticate::s_firewalled = true;


	ServerAuthenticate::ServerAuthenticate(mse::EncryptedPacketSocket::Ptr sock) : AuthenticateBase(sock)
	{
	}


	ServerAuthenticate::~ServerAuthenticate()
	{
	}
	
	
	void ServerAuthenticate::onFinish(bool succes)
	{		
		Out(SYS_CON|LOG_NOTICE) << "Authentication(S) to " << sock->getRemoteIPAddress() 
				<< " : " << (succes ? "ok" : "failure") << endl;
		finished = true;
		setFirewalled(false);
		
		if (!succes)
			sock.clear();
		
		timer.stop();
	}

	void ServerAuthenticate::handshakeReceived(bool full)
	{
		Uint8* hs = handshake;
		AccessManager & aman = AccessManager::instance();

		if (!aman.allowed(sock->getRemoteAddress()))
		{
			Out(SYS_GEN|LOG_NOTICE) << "The IP address " << sock->getRemoteIPAddress() << " is blocked" << endl;
			onFinish(false);
			return;
		}
		
		// try to find a PeerManager which has the right info hash
		SHA1Hash rh(hs+28);
		PeerManager* pman = ServerInterface::findPeerManager(rh);
		if (!pman)
		{
			onFinish(false);
			return;
		}

		if (full)
		{
			// check if we aren't connecting to ourself
			char tmp[21];
			tmp[20] = '\0';
			memcpy(tmp,hs+48,20);
			PeerID peer_id = PeerID(tmp);
			if (pman->getTorrent().getPeerID() == peer_id)
			{
				Out(SYS_CON|LOG_NOTICE) << "Lets not connect to our self" << endl;
				onFinish(false);
				return;
			}
			
			// check if we aren't already connected to the client
			if (pman->connectedTo(peer_id))
			{
				Out(SYS_CON|LOG_NOTICE) << "Already connected to " << peer_id.toString() << endl;
				onFinish(false);
				return;
			}
			
					
			// send handshake and finish off
			sendHandshake(rh,pman->getTorrent().getPeerID());
			onFinish(true);
			// hand over connection
			pman->newConnection(sock,peer_id,supportedExtensions());
		}
		else
		{
			// if the handshake wasn't fully received just send our handshake
			sendHandshake(rh,pman->getTorrent().getPeerID());
		}
	}
}

