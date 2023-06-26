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

#include "dataframe.h"

/**
 * Data frame displays data from a group, such as temperature
*/

CDataFrame::CDataFrame(Glib::ustring str) :
m_box(),
m_frame_sensor("Sensor"),
m_frame_setpoint("Setpoint"),
m_frame_pwm("PWM"),
m_label_sensor("--"),
m_label_setpoint("--"),
m_label_pwm("--")
{
	m_box.set_margin(10);
	m_box.set_halign(Gtk::Align::FILL);

	m_frame_pwm.set_child(m_label_pwm);
	m_frame_sensor.set_child(m_label_sensor);
	m_frame_setpoint.set_child(m_label_setpoint);

	m_box.append(m_frame_setpoint);
	m_frame_setpoint.set_expand(true);
	m_box.append(m_frame_sensor);
	m_frame_sensor.set_expand(true);
	m_box.append(m_frame_pwm);
	m_frame_pwm.set_expand(true);

	set_label(str);
	set_child(m_box);
}

CDataFrame::~CDataFrame()
{
}

void CDataFrame::SetSetpoint(Glib::ustring str)
{
	m_label_setpoint.set_text(str);
}

void CDataFrame::SetSensor(Glib::ustring str)
{
	m_label_sensor.set_text(str);
}

void CDataFrame::SetPWM(Glib::ustring str)
{
	m_label_pwm.set_text(str);
}
