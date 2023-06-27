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

#ifndef _H_SERIAL_CONTROL_
#define _H_SERIAL_CONTROL_

#include "gtkmm.h"

class MainWindow;

class CSerialFrame : public Gtk::Frame
{
public:
	CSerialFrame();
	virtual ~CSerialFrame();

	void SetParentWindow(MainWindow* window);
protected:
	void OnClick_ConnectButton();
	void OnToggle_PowerButton();
	void OnClick_ReloadButton();
	void OnClick_LoggerButton();

private:
	Gtk::Box m_box;
	Gtk::CheckButton m_button_power;
	Gtk::Button m_button_conn; // Serial connect button
	Gtk::Button m_button_reload; // Reload config file button
	Gtk::Button m_button_logdump; // Dump logged values to file
	MainWindow* m_parentWindow;
};

#endif