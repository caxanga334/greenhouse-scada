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

#ifndef _H_CONTROL_FRAME_
#define _H_CONTROL_FRAME_

#include <gtkmm.h>
#include "serialmanager.h"

class CControlFrame;
class MainWindow;

class CControlPanel : public Gtk::Frame
{
public:
	CControlPanel(Glib::ustring name, double value, double lower, double upper, double step_inc, double page_inc, SetpointType type);
	virtual ~CControlPanel();

	void SetControlFrame(CControlFrame* frame);
	CSerialManager* GetSerialManager();
protected:
	void OnButtonClicked();

private:
	CControlFrame* m_parentframe;
	Gtk::Box m_hbox;
	Gtk::Button m_button;
	Glib::RefPtr<Gtk::Adjustment> m_adjustment;
	Gtk::SpinButton m_spin;
	SetpointType m_type;
};

class CControlFrame : public Gtk::Frame
{
public:
	CControlFrame();
	virtual ~CControlFrame();

	void SetParentWindow(MainWindow* window);
	CSerialManager* GetSerialManager();
private:
	MainWindow* m_parentwindow;
	Gtk::Grid m_grid;
	CControlPanel m_temp_cpanel; // temperature control panel
	CControlPanel m_led_cpanel; // LED control panel
	CControlPanel m_humid_cpanel; // humidity control panel
};

#endif