/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#include <QtTest>
#include <QObject>
#include <util/log.h>
#include <utp/utpserver.h>
#include <utp/connection.h>

using namespace utp;

class ConnectionTest : public QEventLoop,public Transmitter
{
	Q_OBJECT
public:
	
	ConnectionTest(QObject* parent = 0) : QEventLoop(parent),remote("127.0.0.1",50000)
	{
	}
	
	bool sendTo(Connection::Ptr conn, const PacketBuffer & packet) override
	{
		sent_packets.append(packet);
		Q_UNUSED(conn);
		return true;
	}
	
    void stateChanged(Connection::Ptr conn, bool readable, bool writeable) override
	{
		Q_UNUSED(conn);
		Q_UNUSED(readable);
		Q_UNUSED(writeable);
	}
	
    void closed(Connection::Ptr conn) override
	{
		Q_UNUSED(conn);
	}
	
	bt::Buffer::Ptr buildPacket(bt::Uint32 type,bt::Uint32 recv_conn_id,bt::Uint32 send_conn_id,bt::Uint16 seq_nr,bt::Uint16 ack_nr)
	{
		TimeValue tv;
		bt::Buffer::Ptr packet = pool->get(Header::size());
		Header hdr;
		hdr.version = 1;
		hdr.type = type;
		hdr.extension = 0;
		hdr.connection_id = type == ST_SYN ? recv_conn_id : send_conn_id;
		hdr.timestamp_microseconds = tv.microseconds;
		hdr.timestamp_difference_microseconds = 0;
		hdr.wnd_size = 6666;
		hdr.seq_nr = seq_nr;
		hdr.ack_nr = ack_nr;
		hdr.write(packet->get());
		return packet;
	}
	
public Q_SLOTS:
	
	
private Q_SLOTS:
	void initTestCase()
	{
		bt::InitLog("connectiontest.log");
		pool = bt::BufferPool::Ptr(new bt::BufferPool());
		pool->setWeakPointer(pool.toWeakRef());
	}
	
	void cleanupTestCase()
	{
		pool.clear();
	}
	
	void init()
	{
		sent_packets.clear();
	}
	
	void testConnID()
	{
		bt::Uint32 conn_id = 666;
		Connection conn(conn_id,utp::Connection::INCOMING,remote,this);
		QVERIFY(conn.connectionStats().recv_connection_id == conn_id);
		QVERIFY(conn.connectionStats().send_connection_id == conn_id - 1);
		
		Connection conn2(conn_id,utp::Connection::OUTGOING,remote,this);
		QVERIFY(conn2.connectionStats().recv_connection_id == conn_id);
		QVERIFY(conn2.connectionStats().send_connection_id == conn_id + 1);
	}
	
	void testOutgoingConnectionSetup()
	{
		bt::Uint32 conn_id = 666;
		Connection conn(conn_id,utp::Connection::OUTGOING,remote,this);
		conn.startConnecting();
		const Connection::Stats & s = conn.connectionStats();
		QVERIFY(s.state == utp::CS_SYN_SENT);
		QVERIFY(s.seq_nr == 2);
		
		bt::Buffer::Ptr pkt = buildPacket(ST_STATE,conn_id,conn_id + 1,1,1);
		PacketParser pp(pkt->get(), pkt->size());
		QVERIFY(pp.parse());
		conn.handlePacket(pp,pkt);
		QVERIFY(s.state == CS_CONNECTED);
		QVERIFY(sent_packets.count() == 1);
	}
	
	void testIncomingConnectionSetup()
	{
		bt::Uint32 conn_id = 666;
		Connection conn(conn_id,utp::Connection::INCOMING,remote,this);
		const Connection::Stats & s = conn.connectionStats();
		
		bt::Buffer::Ptr pkt = buildPacket(ST_SYN,conn_id - 1,conn_id,1,1);
		PacketParser pp(pkt->get(), pkt->size());
		conn.handlePacket(pp,pkt);
		QVERIFY(s.state == CS_CONNECTED);
	}

	
private:
	net::Address remote;
	QList<PacketBuffer> sent_packets;
	bt::BufferPool::Ptr pool;
};

QTEST_MAIN(ConnectionTest)

#include "connectiontest.moc"

