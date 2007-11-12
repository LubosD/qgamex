/***************************************************************************
 *   Copyright (C) 2005-07 by Luboš Doležel                                *
 *   lubos at dolezel info                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2        *
 *   as published by the Free Software Foundation                          *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "qgamex.h"
#include "mainform.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QTemporaryFile>
#include <QProcess>
#include <QtDebug>
#include <iostream>
#include <cstdio>

#define ROOTCOLOR "xsetroot -solid black\n"

int main(int argc, char** argv)
{
	if(argc == 1)
	{
		QApplication app(argc, argv);
		MainForm wnd;
		wnd.show();
	
		return app.exec();
	}
	else
	{
		QMap<QString,QString> params;
		const char* profile = getenv("PROFILE");
		
		if(profile && !loadProfile(profile,params))
		{
			if(getenv("DISPLAY") != 0)
			{
				QApplication app(argc, argv);
				QMessageBox::critical(0, "Fatal error", "Unable to load specified profile!");
			}
			else
				std::cerr << "Fatal error: Unable to load specified profile!\n";
			return 1;
		}
		else
		{
			dumpProfile(params);
			runGame(argc,argv,params);
			return 0;
		}
	}
}

void runGame(int argc,char** argv,const QMap<QString,QString>& params)
{
	const char* nvargs[] = { "__GL_FSAA_MODE", "__GL_LOG_MAX_ANISO", "__GL_SYNC_TO_VBLANK", "__GL_DOOM3" };
	int display;
	QTemporaryFile file;
	QStringList args;
	QString cmd,filename;
	
	file.open();
	filename = file.fileName();
	
	bool startX, bKeymap;
	
	if(startX = (params.value("runtype","0").toInt() == 0))
	{
		file.write(ROOTCOLOR,sizeof(ROOTCOLOR)-1);
		display = getFreeDisplay();
	}
	else
	{
		display = getCurrentDisplay();
	}
	
	if(params.value("xmodmap","1").toInt())
	{
		QString xm = getXmodmap();
		
		if(!xm.isNull())
		{
			cmd = QString("xmodmap %1\n").arg(xm);
			file.write(cmd.toAscii(),cmd.length());
		}
	}
	
	if(params.value("xinit","0").toInt())
	{
		cmd = params.value("xinit_val");
		cmd += "\n";
		file.write(cmd.toAscii(),cmd.length());
	}
	
	if(params.value("screen","0").toInt())
	{
		cmd = QString("export DISPLAY=%1:%2\n").arg(display).arg(params.value("screen_val",0));
		file.write(cmd.toAscii(),cmd.length());
	}
	
	if(bKeymap = (params.value("keymap","0").toInt()) != 0)
	{
		cmd = "for str in $(setxkbmap -v 7); do [[ \"$prev\" == \"layout:\" ]] && prevmap=$str; prev=$str; done\n";
		cmd += QString("setxkbmap %1\n").arg(params.value("keymap_val"));
		file.write(cmd.toAscii(),cmd.length());
	}
	
	for(size_t i=0;i<sizeof(nvargs)/sizeof(nvargs[0]);i++)
	{
		int val;
		if((val = params.value(nvargs[i],"-1").toInt()) != -1)
		{
			cmd = QString("export %1=%2\n").arg(nvargs[i]).arg(val);
			file.write(cmd.toAscii(),cmd.length());
		}
	}
	
	if(params.value("nv_sync","0").toInt())
	{
		if(params.value("__GL_SYNC_TO_VBLANK").toInt() != 0) // -1 for default, 0 for no, 1 for yes
		{
			cmd = QString("export __GL_SYNC_DISPLAY_DEVICE=\"%1\"\n").arg(params.value("nv_sync_val"));
			file.write(cmd.toAscii(),cmd.length());
		}
	}
	
	cmd.clear();
	for(int i=1;i<argc;i++)
	{
		QString arg = argv[i];
		arg.replace('"', "\\\"");
		
		if(arg.contains(' '))
			cmd += QString("\"%1\" ").arg(arg);
		else
			cmd += QString("%1 ").arg(arg);
	}
	
	cmd += '\n';
	
	if(bKeymap)
		cmd += "echo $prevmap;setxkbmap $prevmap\n";
	file.write(cmd.toAscii(),cmd.length());
	
	file.close();
	
	QProcess process;
	if(startX)
	{
		QStringList args;
		
		args << filename << "--" << QString(":%1").arg(display);
		
		qDebug() << "Starting new X session on display" << display;
		
		if(params.value("xconf","0").toInt())
			args << "-config" << params.value("xconf_val","xorg.conf");
		
		qDebug() << "Args:" << args;
		
		process.setProcessChannelMode(QProcess::ForwardedChannels);
		process.start("startx", args, QIODevice::ReadOnly);
	}
	else
	{
		process.setProcessChannelMode(QProcess::ForwardedChannels);
		process.start("/bin/sh", QStringList(filename), QIODevice::ReadOnly);
	}
	
	if(!process.waitForStarted(-1))
		QMessageBox::critical(0, "Fatal error", "Unable to run a new process, something went wrong.");
	else
		process.waitForFinished(-1);
}

int getFreeDisplay()
{
	for(int i=0;i<12;i++)
	{	
		if(!QFile::exists(QString("/tmp/.X%1-lock").arg(i)))
			return i;
	}
	return 0;
}

int getCurrentDisplay()
{
	const char* display = getenv("DISPLAY");
	
	if(!display)
	{
		std::cerr << "The environment variable \"DISPLAY\" is not set.\n";
		exit(1);
	}
	else
	{
		display = strchr(display, ':');
		if(!display)
		{
			std::cerr << "The environment variable \"DISPLAY\" is invalid.\n";
			exit(1);
		}
		else
			return atoi(display+1);
	}
}

QString getXmodmap()
{
	QString path = QDir::home().filePath(".Xmodmap");
	if(QFile::exists(path))
		return path;
	
	// try the system wide
	if(QFile::exists("/etc/X11/Xmodmap"))
		return "/etc/X11/Xmodmap";
	
	return QString();
}

QStringList getProfileList()
{
	QStringList result,filter;
	QDir home = QDir::home();
	home.mkdir(".qgamex");
	
	if(!home.cd(".qgamex"))
	{
		QMessageBox::critical(0, "Fatal error", "Cannot open the $HOME/.qgamex directory!");
		return QStringList();
	}
	else
	{
		filter << "*.prf";
		result = home.entryList(filter, QDir::Files);
		
		for(int i=0;i<result.size();i++)
			result[i] = result[i].left(result[i].lastIndexOf('.'));
		
		return result;
	}
}

QString profileToFile(QString name)
{
	QDir home = QDir::home();
	
	if(!home.cd(".qgamex"))
		return QString();
	else
		return home.filePath(name + ".prf");
}

bool loadProfile(QString name, QMap<QString,QString>& data)
{
	QString fileName;
	QFile file;
	QDomDocument doc;
	
	fileName = profileToFile(name);
	if(fileName.isNull())
		return false;
	else
		file.setFileName(fileName);
	
	if(!file.open(QIODevice::ReadOnly))
		return false;
	else
	{
		if(!doc.setContent(&file))
			return false;
		
		data.clear();
		
		QDomNode n = doc.documentElement().firstChild();
		while(!n.isNull())
		{
			QDomElement e = n.toElement();
			if(!e.isNull() && e.tagName() == "param")
			{
				if(!e.hasAttribute("name"))
					continue;
				else
				{
					QString name,value;
					name = e.attribute("name");
					value = e.text();
					
					data[name] = value;
				}
			}
			n = n.nextSibling();
 		}
		
		return true;
	}
}

bool saveProfile(QString name, const QMap<QString,QString>& data)
{
	QMapIterator<QString,QString> it(data);
	QDomDocument doc;
	QFile file;
	QString fileName;
	QDomElement root;
	
	fileName = profileToFile(name);
	if(fileName.isNull())
		return false;
	else
		file.setFileName(fileName);
	
	root = doc.createElement("qgamex");
	doc.appendChild(root);
	
	while(it.hasNext())
	{
		QDomText text;
		QDomElement elem = doc.createElement("param");
		it.next();
		
		text = doc.createTextNode(it.value());
		elem.setAttribute("name", it.key());
		elem.appendChild(text);
		root.appendChild(elem);
	}
	
	if(!file.open(QIODevice::WriteOnly))
		return false;
	else
	{
		file.write(doc.toByteArray());
		file.close();
		return true;
	}
}

bool deleteProfile(QString name)
{
	QDir home = QDir::home();
	if(!home.cd(".qgamex"))
		return false;
	return home.remove(name + ".prf");
}

void dumpProfile(const QMap<QString,QString>& data)
{
	QMapIterator<QString,QString> it(data);
	while(it.hasNext())
	{
		it.next();
		qDebug() << it.key() << "=" << it.value();
	}
}

