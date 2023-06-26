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

#include "controlframe.h"
#include "app.h"
#include <iostream>

CControlPanel::CControlPanel(Glib::ustring name, double value, double lower, double upper, double step_inc, double page_inc, SetpointType type) :
m_hbox(),
m_button("Alterar Setpoint"),
m_adjustment( Gtk::Adjustment::create(value, lower, upper, step_inc, page_inc) ),
m_spin(m_adjustment)
{
	set_label(name);
	m_hbox.set_expand(true);
	m_button.set_expand(true);
	m_button.signal_clicked().connect(sigc::mem_fun(*this, &CControlPanel::OnButtonClicked));
	m_spin.set_expand(true);
	m_spin.set_digits(2);
	m_spin.set_numeric(true);
	m_hbox.append(m_button);
	m_hbox.append(m_spin);
	set_child(m_hbox);
	m_parentframe = nullptr;
	m_type = type;
}

CControlPanel::~CControlPanel()
{
	m_parentframe = nullptr;
}

void CControlPanel::SetControlFrame(CControlFrame *frame)
{
	m_parentframe = frame;
}

CSerialManager *CControlPanel::GetSerialManager()
{
	return m_parentframe->GetSerialManager();
}

void CControlPanel::OnButtonClicked()
{
	// std::cout << get_label() << " -- Clicked! -- " << m_spin.get_value() << std::endl;
	GetSerialManager()->SendCommand(SERIAL_CMD_SETPOINT, m_type, static_cast<float>(m_spin.get_value()));
}

CControlFrame::CControlFrame() :
m_grid(),
m_temp_cpanel("Temperatura", 24.0, 15.0, 30.0, 0.5, 5.0, SETPOINT_TEMPERATURE),
m_led_cpanel("LED", 50.0, 0.0, 100.0, 1.0, 10.0, SETPOINT_LED),
m_humid_cpanel("Humidade", 50.0, 0.0, 100.0, 1.0, 10.0, SETPOINT_HUMIDITY)
{
	m_grid.attach(m_temp_cpanel, 0, 0);
	m_grid.attach(m_led_cpanel, 0, 1);
	m_grid.attach(m_humid_cpanel, 0, 2);
	m_grid.set_expand(true);

	m_temp_cpanel.SetControlFrame(this);
	m_led_cpanel.SetControlFrame(this);
	m_humid_cpanel.SetControlFrame(this);

	set_child(m_grid);
	m_parentwindow = nullptr;
}

CControlFrame::~CControlFrame()
{
	m_parentwindow = nullptr;
}

void CControlFrame::SetParentWindow(MainWindow* window)
{
	m_parentwindow = window;
}

CSerialManager *CControlFrame::GetSerialManager()
{
	return m_parentwindow->GetSerialManager();
}
