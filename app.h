/*
	Greenhouse SCADA - A simple GUI SCADA software for a small greenhouse project
	Copyright (C) 2023  caxanga334

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef _H_APP_
#define _H_APP_

#include <gtkmm.h>
#include <string>
#include <memory>

#include "dataframe.h"
#include "controlframe.h"
#include "serialcontrol.h"
#include "serialmanager.h"

class MainWindow : public Gtk::Window
{
public:
	MainWindow();
	virtual ~MainWindow();

	CSerialManager* GetSerialManager();
	void OnReceiveSerialCommand(CSerialCommand* command);
protected:
	bool OnTimer_Update();

private:
	Gtk::Grid m_grid;
	Gtk::Box m_subbox;
	CDataFrame m_dataframe_temp;
	CDataFrame m_dataframe_led;
	CDataFrame m_dataframe_humid;
	CControlFrame m_controlframe;
	CSerialFrame m_serialframe;
	std::shared_ptr<CSerialManager> m_serialmanager;
	sigc::connection m_updatetimer;
};

inline CSerialManager* MainWindow::GetSerialManager()
{
	return m_serialmanager.get();
}

#endif