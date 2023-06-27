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

#include "app.h"
#include <iostream>

MainWindow::MainWindow() :
m_grid(),
m_subbox(),
m_dataframe_temp("Temperatura"),
m_dataframe_led("LED"),
m_dataframe_humid("Humidade"),
m_controlframe(),
m_serialframe()
{
	set_title("Estufa -- Supervisorio");
	set_default_size(800, 600);
	
	m_serialmanager = std::make_shared<CSerialManager>();
	m_serialmanager->SetMainWindow(this);

	m_grid.set_margin(10);
	m_grid.attach(m_controlframe, 0, 0);
	m_grid.attach(m_subbox, 1, 0);
	m_grid.attach(m_serialframe, 0, 1, 2, 1);
	m_grid.set_expand(true);

	m_subbox.set_margin(5);
	m_subbox.set_spacing(3);
	m_subbox.set_vexpand(true);
	m_subbox.set_valign(Gtk::Align::FILL);
	m_subbox.set_orientation(Gtk::Orientation::VERTICAL);

	m_subbox.append(m_dataframe_temp);
	m_dataframe_temp.set_expand(true);
	m_subbox.append(m_dataframe_led);
	m_dataframe_led.set_expand(true);
	m_subbox.append(m_dataframe_humid);
	m_dataframe_humid.set_expand(true);

	m_serialframe.set_expand(true);

	m_controlframe.SetParentWindow(this);
	m_serialframe.SetParentWindow(this);

	set_child(m_grid);

	sigc::slot<bool()> slot_update = sigc::bind(sigc::mem_fun(*this, &MainWindow::OnTimer_Update));
	m_updatetimer = Glib::signal_timeout().connect(slot_update, 500); // Call serial manager update every 500 ms
}

MainWindow::~MainWindow()
{
	m_updatetimer.disconnect();
}

bool MainWindow::OnTimer_Update()
{
	m_serialmanager->Update();
	return true;
}

void MainWindow::OnReceiveSerialCommand(CSerialCommand *command)
{
	switch (command->GetType())
	{
	case SETPOINT_TEMPERATURE:
		m_dataframe_temp.SetSetpoint(command->GetSetpointData());
		m_dataframe_temp.SetSensor(command->GetSensorData());
		m_dataframe_temp.SetPWM(command->GetPWMData());
		break;
	case SETPOINT_LED:
		m_dataframe_led.SetSetpoint(command->GetSetpointData());
		m_dataframe_led.SetSensor(command->GetSensorData());
		m_dataframe_led.SetPWM(command->GetPWMData());
		break;
	case SETPOINT_HUMIDITY:
		m_dataframe_humid.SetSetpoint(command->GetSetpointData());
		m_dataframe_humid.SetSensor(command->GetSensorData());
		m_dataframe_humid.SetPWM(command->GetPWMData());
		break;
	default:
		break;
	}
}