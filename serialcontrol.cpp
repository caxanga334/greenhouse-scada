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

#include "serialcontrol.h"
#include "app.h"
#include <iostream>

CSerialFrame::CSerialFrame() :
m_box(Gtk::Orientation::HORIZONTAL, 5),
m_button_power("LIGADO"),
m_button_conn("Conectar"),
m_button_reload("Reconfigurar"),
m_button_logdump("Logger")
{
	set_label("Controle Serial");
	set_label_align(Gtk::Align::CENTER);

	m_box.append(m_button_power);
	m_box.append(m_button_conn);
	m_box.append(m_button_reload);
	m_box.append(m_button_logdump);

	m_button_power.signal_toggled().connect(sigc::mem_fun(*this, &CSerialFrame::OnToggle_PowerButton));
	m_button_conn.signal_clicked().connect(sigc::mem_fun(*this, &CSerialFrame::OnClick_ConnectButton));
	m_button_reload.signal_clicked().connect(sigc::mem_fun(*this, &CSerialFrame::OnClick_ReloadButton));
	m_button_logdump.signal_clicked().connect(sigc::mem_fun(*this, &CSerialFrame::OnClick_LoggerButton));

	m_button_power.set_expand(true);
	m_button_conn.set_expand(true);
	m_button_reload.set_expand(true);
	m_button_logdump.set_expand(true);
	m_box.set_expand(true);

	set_child(m_box);
	m_parentWindow = nullptr;
}

CSerialFrame::~CSerialFrame()
{
	m_parentWindow = nullptr;
}

void CSerialFrame::SetParentWindow(MainWindow *window)
{
	m_parentWindow = window;
}

void CSerialFrame::OnClick_ConnectButton()
{
	m_parentWindow->GetSerialManager()->OpenConnection();
}

void CSerialFrame::OnToggle_PowerButton()
{
	if (m_button_power.get_active())
	{
		m_parentWindow->GetSerialManager()->SendCommand(SERIAL_CMD_POWER_ON);
	}
	else
	{
		m_parentWindow->GetSerialManager()->SendCommand(SERIAL_CMD_POWER_OFF);
	}
}

void CSerialFrame::OnClick_ReloadButton()
{
	m_parentWindow->GetSerialManager()->ReloadConfig();
}

void CSerialFrame::OnClick_LoggerButton()
{
	m_parentWindow->GetSerialManager()->InvokeLogger();
}
