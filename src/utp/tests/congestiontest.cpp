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
#include <utp/connection.h>
#include <utp/utpsocket.h>
#include <utp/utpserver.h>
#include <util/functions.h>
#include <ctime>
#include <unistd.h>


#define PACKETS_TO_SEND 20
#define TEST_DATA "This is the packet loss test\n"

using namespace utp;
using namespace bt;

/**
	Server which simulates packet loss
*/
class CongestionTestServer : public UTPServer
{
	Q_OBJECT
public:
	CongestionTestServer(QObject* parent = 0) : UTPServer(parent),congestion_delay(200)
	{}
	
	virtual ~CongestionTestServer()
	{}
	
	virtual bool sendTo(const QByteArray& data, const net::Address& addr)
	{
	}
	
	virtual bool sendTo(const bt::Uint8* data, const bt::Uint32 size, const net::Address& addr)
	{
	}
	
	void setCongestionDelay(int cd)
	{
		congestion_delay = cd;
	}
	
public Q_SLOTS:
	void delayedSend()
	{
	}
	
private:
	int congestion_delay;
};

class SendThread : public QThread
{
	Q_OBJECT
public:
	
	SendThread(Connection::Ptr outgoing,QObject* parent = 0) : QThread(parent),outgoing(outgoing)
	{}
	
	virtual void run()
	{
		char test[] = TEST_DATA;
		int sent = 0;
		while (sent < PACKETS_TO_SEND)
		{
			int ret = outgoing->send((const bt::Uint8*)test,strlen(test));
			if (ret > 0)
			{
				sent++;
			}
			
			msleep(200);
		}
		
		while (!outgoing->allDataSent())
			sleep(1);
	
		Out(SYS_UTP|LOG_DEBUG) << "Transmitted " << sent << " packets " << endl;
		outgoing->dumpStats();
	}
	
	Connection::Ptr outgoing;
};

class CongestionTest : public QEventLoop
{
	Q_OBJECT
public:
	CongestionTest(QObject* parent = 0) : QEventLoop(parent)
	{
	}
	
public Q_SLOTS:
	void accepted()
	{
		incoming = srv.acceptedConnection().toStrongRef();
		exit();
	}
	
	void endEventLoop()
	{
		exit();
	}
	
private Q_SLOTS:
	void initTestCase()
	{
		bt::InitLog("congestiontest.log");
		
		incoming = outgoing = 0;
		port = 50000;
		while (port < 60000)
		{
			if (!srv.changePort(port))
				port++;
			else
				break;
		}
		
		srv.setCreateSockets(false);
		srv.start();
	}
	
	void cleanupTestCase()
	{
		srv.stop();
	}
	
	void testConnect()
	{
		net::Address addr("127.0.0.1",port);
		connect(&srv,SIGNAL(accepted()),this,SLOT(accepted()),Qt::QueuedConnection);
		outgoing = srv.connectTo(addr);
		QVERIFY(outgoing != 0);
		QTimer::singleShot(5000,this,SLOT(endEventLoop())); // use a 5 second timeout
		exec();
		QVERIFY(incoming != 0);
	}
	
	void testCongestionTest()
	{
		bt::Out(SYS_UTP|LOG_DEBUG) << "testCongestionTest" << bt::endl;
		if (outgoing->connectionState() != CS_CONNECTED || incoming->connectionState() != CS_CONNECTED)
		{
			QSKIP("Not Connected",SkipAll);
			return;
		}
		/*
		srv.setCongestionTestSimulation(true,0.1); // Drop 10 % of all packets
		SendThread st(outgoing);
		st.start(); // The thread will start sending a whole bunch of data
		int received = 0;
		QString received_data;
		while (!st.isFinished())
		{
			bt::Uint32 ba = incoming->bytesAvailable();
			if (ba > 0)
			{
				QByteArray data(ba,0);
				int ret = incoming->recv((bt::Uint8*)data.data(),ba);
				if (ret > 0)
				{
					received_data.append(data);
					received += ret;
				}
			}
			else
			{
				usleep(50000);
			}
		}
		
		st.wait();
		Out(SYS_UTP|LOG_DEBUG) << "Received " << received << " bytes:" << endl;
		Out(SYS_UTP|LOG_DEBUG) << received_data << endl;
		incoming->dumpStats();
		QVERIFY(incoming->bytesAvailable() == 0);
		QVERIFY(received_data.count(TEST_DATA) == PACKETS_TO_SEND);
		QVERIFY(outgoing->allDataSent());
		*/
	}
	
private:
	
	
private:
	Connection::Ptr incoming;
	Connection::Ptr outgoing;
	CongestionTestServer srv;
	int port;
};

QTEST_MAIN(CongestionTest)

#include "congestiontest.moc"

