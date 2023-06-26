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

#ifndef _H_DATAFRAME_
#define _H_DATAFRAME_

#include <gtkmm.h>

class CDataFrame : public Gtk::Frame
{
public:
	CDataFrame(Glib::ustring str);
	virtual ~CDataFrame();

	void SetSetpoint(Glib::ustring str);
	void SetSensor(Glib::ustring str);
	void SetPWM(Glib::ustring str);

private:
	Gtk::Box m_box;
	Gtk::Frame m_frame_sensor, m_frame_setpoint, m_frame_pwm;
	Gtk::Label m_label_sensor, m_label_setpoint, m_label_pwm;
};

#endif