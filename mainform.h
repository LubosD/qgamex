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

#ifndef MAINFORM_H
#define MAINFORM_H
#include <QDialog>
#include "ui_mainform.h"

class MainForm : public QDialog, public Ui_MainForm
{
Q_OBJECT
public:
	MainForm();
private:
	void reloadProfiles();
public slots:
	void updateByCheckbox(int);
	void xconfPressed();
	void newProfile();
	void loadProfile();
	void saveProfile();
	void deleteProfile();
	void loadProfile(QListWidgetItem*) { loadProfile(); }
	void showAbout();
	
	void switchedRunType() { updateByCheckbox(0); }
	
	void handleGpuChange(const QString& type);
};

#endif
