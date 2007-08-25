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

#ifndef QGAMEX_H
#define QGAMEX_H
#include <QStringList>
#include <QMap>

QStringList getProfileList();
bool loadProfile(QString name, QMap<QString,QString>& data);
bool saveProfile(QString name, const QMap<QString,QString>& data);
bool deleteProfile(QString name);
QString profileToFile(QString name);
void dumpProfile(const QMap<QString,QString>& data);

void runGame(int argc,char** argv,const QMap<QString,QString>& params);
int getFreeDisplay();
int getCurrentDisplay();
QString getXmodmap();

#endif
