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

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <QDir>
#include <kcmdlineargs.h>

#include "ktcli.h"

#include <version.h>
#include <util/error.h>
#include <util/functions.h>
#include <peer/authenticationmonitor.h>


using namespace bt;

KTCLI::KTCLI() : tc(new TorrentControl())
{
	qsrand(time(0));
	bt::SetClientInfo("ktcli",bt::MAJOR,bt::MINOR,bt::RELEASE,bt::NORMAL,"KT");
	bt::InitLog("ktcli.log",false,true,true);
	connect(tc.get(),SIGNAL(finished(bt::TorrentInterface*)),this,SLOT(finished(bt::TorrentInterface*)));
}

KTCLI::~KTCLI()
{
}

bool KTCLI::start()
{
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	bool ok = false;
	quint16 port = args->getOption("port").toInt(&ok);
	if (!ok)
		port = qrand();
	
	if (!bt::Globals::instance().initTCPServer(port))
	{
		Out(SYS_GEN|LOG_NOTICE) << "Failed to listen on port " << port << endl;
		return false;
	}
	
	return load(KCmdLineArgs::parsedArgs()->url(args->count() - 1));
}


bool KTCLI::load(const KUrl & url)
{
	QDir dir(url.toLocalFile());
	if (dir.exists() && dir.exists("torrent") && dir.exists("stats"))
	{
		// Load existing torrent
		if (loadFromDir(dir.absolutePath()))
		{
			tc->start();
			connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
			timer.start(250);
			return true;
		}
	}
	else if (url.isLocalFile())
	{
		QString path = url.toLocalFile();
		if (loadFromFile(path))
		{
			tc->start();
			connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
			timer.start(250);
			return true;
		}
	}
	else
	{
		Out(SYS_GEN|LOG_IMPORTANT) << "Non local files not supported" << endl;
	}
	
	return false;
}

QString KTCLI::tempDir()
{
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	QDir tmpdir = QDir(args->getOption("tmpdir"));
	int i = 0;
	while (tmpdir.exists(QString("tor%1").arg(i)))
		i++;
	
	QString sd = QString("tor%1").arg(i);
	if (!tmpdir.mkdir(sd))
		throw bt::Error(QString("Failed to create temporary directory %1").arg(sd));
	
	tmpdir.cd(sd);
	return tmpdir.absolutePath();
}

bool KTCLI::loadFromFile(const QString & path)
{
	try
	{
		tc->init(this, path, tempDir(), QDir::currentPath());
		tc->setLoadUrl(KUrl(path));
		tc->createFiles();
		return true;
	}
	catch (bt::Error & err)
	{
		Out(SYS_GEN|LOG_IMPORTANT) << err.toString() << endl;
		return false;
	}
}

bool KTCLI::loadFromDir(const QString& path)
{
	try
	{
		tc->init(this, path + "/torrent", path, QString::null);
		tc->createFiles();
		return true;
	}
	catch (bt::Error & err)
	{
		Out(SYS_GEN|LOG_IMPORTANT) << err.toString() << endl;
		return false;
	}
}


bool KTCLI::notify(QObject* obj, QEvent* ev)
{
	try
	{
		return QCoreApplication::notify(obj,ev);
	}
	catch (bt::Error & err)
	{
		Out(SYS_GEN|LOG_DEBUG) << "Error: " << err.toString() << endl;
	}
	catch (std::exception & err)
	{
		Out(SYS_GEN|LOG_DEBUG) << "Error: " << err.what() << endl;
	}
	
	return true;
}

bool KTCLI::alreadyLoaded(const bt::SHA1Hash& ih) const 
{
	Q_UNUSED(ih); 
	return false;
}

void KTCLI::mergeAnnounceList(const bt::SHA1Hash& ih, const bt::TrackerTier* trk)
{
	Q_UNUSED(ih);
	Q_UNUSED(trk);
}


void KTCLI::update()
{
	bt::UpdateCurrentTime();
	AuthenticationMonitor::instance().update();
	tc->update();
}

void KTCLI::finished(bt::TorrentInterface* tor)
{
	Q_UNUSED(tor);
	Out(SYS_GEN|LOG_NOTICE) << "Torrent fully downloaded" << endl;
	quit();
}