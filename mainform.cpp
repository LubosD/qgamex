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

#include "mainform.h"
#include "qgamex.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QMap>

QMap<QString,int> g_mapAATypes;
QMap<QString,QList<int> > g_mapGPUCaps;

MainForm::MainForm()
{
	setupUi(this);
	
	connect(checkKeymap, SIGNAL(stateChanged(int)), this, SLOT(updateByCheckbox(int)));
	connect(checkXConf, SIGNAL(stateChanged(int)), this, SLOT(updateByCheckbox(int)));
	connect(checkScreen, SIGNAL(stateChanged(int)), this, SLOT(updateByCheckbox(int)));
	connect(checkXinit, SIGNAL(stateChanged(int)), this, SLOT(updateByCheckbox(int)));
	connect(checkSync, SIGNAL(stateChanged(int)), this, SLOT(updateByCheckbox(int)));
	
	connect(radioRunType1, SIGNAL(clicked()), this, SLOT(switchedRunType()));
	connect(radioRunType2, SIGNAL(clicked()), this, SLOT(switchedRunType()));
	
	connect(comboGpuType, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(handleGpuChange(const QString&)));
	
	connect(toolXConf, SIGNAL(clicked()), this, SLOT(xconfPressed()));
	connect(pushLoad, SIGNAL(clicked()), this, SLOT(loadProfile()));
	connect(pushNew, SIGNAL(clicked()), this, SLOT(newProfile()));
	connect(pushSave, SIGNAL(clicked()), this, SLOT(saveProfile()));
	connect(pushDelete, SIGNAL(clicked()), this, SLOT(deleteProfile()));
	
	connect(listProfiles, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(loadProfile(QListWidgetItem*)));
	connect(pushAbout, SIGNAL(clicked()), this, SLOT(showAbout()));
	
	QStringList list;
	list << "GeForce, GeForce2, Quadro, Quadro2 Pro";
	list << "GeForce4 MX, GeForce4 4xx Go, Quadro4 380,550,580 XGL, Quadro4 NVS";
	list << "GeForce3, Quadro DCC, GeForce4 Ti, GeForce4 4200 Go, Quadro4 700,750,780,900,980 XGL";
	list << "GeForce FX, GeForce 6xxx, GeForce 7xxx, Quadro FX";
	list << "GeForce 8xxx, G8xGL";
	
	g_mapGPUCaps[list[0]] = QList<int>() << -1 << 0 << 3 << 4;
	g_mapGPUCaps[list[1]] = QList<int>() << -1 << 0 << 1 << 2 << 4;
	g_mapGPUCaps[list[2]] = QList<int>() << -1 << 0 << 1 << 2 << 4 << 5 << 6;
	g_mapGPUCaps[list[3]] = QList<int>() << -1 << 0 << 1 << 2 << 4 << 5 << 6 << 7 << 8;
	g_mapGPUCaps[list[4]] = QList<int>() << -1 << 0 << 1 << 4 << 7 << 9 << 10 << 11 << 12 << 13;
	
	comboGpuType->addItems(list);
	
	g_mapAATypes["<default>"] = -1;
	g_mapAATypes["Disabled"] = 0;
	g_mapAATypes["2x Bilinear Multisampling"] = 1;
	g_mapAATypes["2x Quincunx Multisampling"] = 2;
	g_mapAATypes["1.5 x 1.5 Supersampling"] = 3;
	g_mapAATypes["2 x 2 Supersampling / 4x Bilinear Multisampling"] = 4;
	g_mapAATypes["4x Gaussian Multisampling"] = 5;
	g_mapAATypes["2x Bilinear Multisampling by 4x Supersampling"] = 6;
	g_mapAATypes["4x Bilinear Multisampling by 4x Supersampling"] = 7;
	g_mapAATypes["4x Bilinear Multisampling by 2x Supersampling"] = 8;
	g_mapAATypes["8x Bilinear Multisampling"] = 9;
	g_mapAATypes["8x"] = 10;
	g_mapAATypes["16x"] = 11;
	g_mapAATypes["16xQ"] = 12;
	g_mapAATypes["8x Bilinear Multisampling by 4x Supersampling"] = 13;
	
	handleGpuChange(list[0]);
	list.clear();
	
	list << "<default>" << "Disabled" << "Enabled";
	
	comboVblank->addItems(list);
	comboDoom3->addItems(list);
	
	listProfiles->addItems(getProfileList());
	radioRunType1->setChecked(true);
	updateByCheckbox(0);
}

void MainForm::handleGpuChange(const QString& type)
{
	QString prev = comboFSAA->currentText();
	QList<int> caps = g_mapGPUCaps[type];
	int gindex = comboGpuType->currentIndex();
	int newindex = 0;
	
	comboFSAA->clear();
	
	for(int i=0;i<caps.size();i++)
	{
		QString name = g_mapAATypes.key(caps[i]);
		comboFSAA->addItem(name);
		
		if(name == prev)
			newindex = i;
	}
	
	comboFSAA->setCurrentIndex(newindex);
	
	newindex = comboAniso->currentIndex();
	comboAniso->clear();
	
	QStringList list;
	
	list << "<default>" << "Disabled" << "2x anisotropic filtering";
	
	if(gindex >= 2)
	{
		list << "4x anisotropic filtering" << "8x anisotropic filtering";
		if(gindex >= 3)
			list << "16x anisotropic filtering";
	}
	
	if(newindex > list.size()-1)
		newindex = list.size()-1;
	else if(newindex < 0)
		newindex = 0;
	
	comboAniso->addItems(list);
	comboAniso->setCurrentIndex(newindex);
}

void MainForm::updateByCheckbox(int)
{
	bool newX = radioRunType1->isChecked();
	
	lineKeymap->setEnabled(checkKeymap->checkState() == Qt::Checked);
	
	checkXConf->setEnabled(newX);
	lineXConf->setEnabled(checkXConf->checkState() == Qt::Checked && newX);
	toolXConf->setEnabled(checkXConf->checkState() == Qt::Checked && newX);
	
	spinScreen->setEnabled(checkScreen->checkState() == Qt::Checked);
	textXinit->setEnabled(checkXinit->checkState() == Qt::Checked);
	lineSync->setEnabled(checkSync->checkState() == Qt::Checked);
}

void MainForm::xconfPressed()
{
	QString xconf = QFileDialog::getOpenFileName(this, "Choose file", "/etc/X11", "X11 config files (*.conf)");
	
	if(!xconf.isNull())
	{
		if(!xconf.startsWith("/etc/X11/"))
			QMessageBox::warning(this, "Error", "The X config file must reside in the /etc/X11 directory!");
		else
			lineXConf->setText(xconf.mid(9));
	}
}

void MainForm::newProfile()
{
	comboGpuType->setCurrentIndex(0);
	comboFSAA->setCurrentIndex(0);
	comboAniso->setCurrentIndex(0);
	comboVblank->setCurrentIndex(0);
	comboDoom3->setCurrentIndex(0);
	
	radioRunType1->setChecked(true);
	
	checkXmodmap->setCheckState(Qt::Checked);
	checkKeymap->setCheckState(Qt::Unchecked);
	checkXConf->setCheckState(Qt::Unchecked);
	checkScreen->setCheckState(Qt::Unchecked);
	checkXinit->setCheckState(Qt::Unchecked);
	checkSync->setCheckState(Qt::Unchecked);
	
	lineKeymap->clear();
	lineSync->clear();
	lineKeymap->setText("xorg.conf");
	lineKeymap->setText("us");
	textXinit->clear();
	
	updateByCheckbox(0);
}

void MainForm::loadProfile()
{
	QMap<QString,QString> params;
	QListWidgetItem* item;
	
	item = listProfiles->currentItem();
	
	if(!item) return;
	
	if(!::loadProfile(item->text(),params))
		QMessageBox::critical(0, "Fatal error", "Unable to load the specified profile!");
	else
	{
		newProfile();
		checkXmodmap->setCheckState((params.value("xmodmap","1").toInt()) ? Qt::Checked : Qt::Unchecked);
		lineName->setText(item->text());
		
		((params.value("runtype","0").toInt()) ? radioRunType2 : radioRunType1)->setChecked(true);
		
		if(params.value("keymap","0").toInt())
		{
			checkKeymap->setCheckState(Qt::Checked);
			lineKeymap->setText(params.value("keymap_val","us"));
		}
		
		if(params.value("xconf","0").toInt())
		{
			checkXConf->setCheckState(Qt::Checked);
			lineXConf->setText(params.value("xconf_val","xorg.conf"));
		}
		
		if(params.value("screen","0").toInt())
		{
			checkScreen->setCheckState(Qt::Checked);
			spinScreen->setValue(params.value("screen_val","0").toInt());
		}
		
		if(params.value("xinit","0").toInt())
		{
			checkXinit->setCheckState(Qt::Checked);
			textXinit->setPlainText(params.value("xinit_val",""));
		}
		
		if(params.value("nv_sync","0").toInt())
		{
			checkSync->setCheckState(Qt::Checked);
			lineSync->setText(params.value("nv_sync_val",""));
		}
		
		comboGpuType->setCurrentIndex(params.value("gputype").toInt());
		handleGpuChange(comboGpuType->currentText());
		
		int fsaanum = params.value("__GL_FSAA_MODE","-1").toInt();
		int index = g_mapGPUCaps[comboGpuType->currentText()].indexOf(fsaanum);
		
		comboFSAA->setCurrentIndex((index<0) ? 0 : index);
		
		comboAniso->setCurrentIndex(params.value("__GL_LOG_MAX_ANISO","-1").toInt()+1);
		comboVblank->setCurrentIndex(params.value("__GL_SYNC_TO_VBLANK","-1").toInt()+1);
		comboDoom3->setCurrentIndex(params.value("__GL_DOOM3","-1").toInt()+1);
		
		lineName->setText(item->text());
	}
	
	updateByCheckbox(0);
}

void MainForm::saveProfile()
{
	QMap<QString,QString> params;
	QString profileName = lineName->text();
	
	if(profileName.isEmpty() || profileName.contains('/'))
	{
		QMessageBox::warning(this,"Error","Invalid profile name!");
	}
	else
	{
		params["xmodmap"] = QString::number((int) checkXmodmap->checkState() == Qt::Checked);
		
		params["runtype"] = QString::number( (radioRunType1->isChecked()) ? 0 : 1 );
		
		if(checkKeymap->checkState() == Qt::Checked)
		{
			params["keymap"] = "1";
			params["keymap_val"] = lineKeymap->text();
		}
		
		if(checkXConf->checkState() == Qt::Checked)
		{
			params["xconf"] = "1";
			params["xconf_val"] = lineXConf->text();
		}
		
		if(checkScreen->checkState() == Qt::Checked)
		{
			params["screen"] = "1";
			params["screen_val"] = QString::number(spinScreen->value());
		}
		
		if(checkXinit->checkState() == Qt::Checked)
		{
			params["xinit"] = "1";
			params["xinit_val"] = textXinit->toPlainText();
		}
		
		if(checkSync->checkState() == Qt::Checked)
		{
			params["nv_sync"] = "1";
			params["nv_sync_val"] = lineSync->text().replace('"',"\\\"");
		}
		
		params["gputype"] = QString::number(comboGpuType->currentIndex());
		params["__GL_FSAA_MODE"] = QString::number( g_mapAATypes[comboFSAA->currentText()] );
		params["__GL_LOG_MAX_ANISO"] = QString::number(comboAniso->currentIndex()-1);
		params["__GL_SYNC_TO_VBLANK"] = QString::number(comboVblank->currentIndex()-1);
		params["__GL_DOOM3"] = QString::number(comboDoom3->currentIndex()-1);
		
		if(::saveProfile(profileName,params))
			reloadProfiles();
		else
			QMessageBox::critical(this, "Fatal error", "Unable to save the current profile!");
	}
}

void MainForm::reloadProfiles()
{
	QString profileName = lineName->text();
	QStringList profiles = getProfileList();
	int index;
	
	listProfiles->clear();
	listProfiles->addItems(profiles);
	
	index = profiles.indexOf(profileName);
	if(index != -1)
	{
		listProfiles->setCurrentRow(index);
		loadProfile();
	}
	else
		newProfile();
}

void MainForm::deleteProfile()
{
	QListWidgetItem* item;
	
	item = listProfiles->currentItem();
	
	if(!item) return;
	
	if(!::deleteProfile(item->text()))
		QMessageBox::critical(0, "Fatal error", "Unable to delete the current profile!");
	reloadProfiles();
}

void MainForm::showAbout()
{
	QMessageBox::information(this, "About QGameX",
		QString::fromUtf8("QGameX 1.1.1\n\nCopyright © 2005-07 Luboš Doležel <lubos@dolezel.info>\n"
				 "Licensed under GNU GPL v2, based on the Trolltech Qt library\n\n"
				 "http://www.dolezel.info"));
}
