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

#ifndef _H_LOGGER_
#define _H_LOGGER_

#include <gtkmm.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <mutex>

class CDataLogger;

// Data writer writes the stored data from a data logger class into a file
class CDataWriter
{
public:
	CDataWriter(std::string filename);
	virtual ~CDataWriter();

	// Writes data to file
	void Write(CDataLogger* logger, std::vector<std::string>* timestamp, std::vector<std::string>* setpoint, std::vector<std::string>* sensor, std::vector<std::string>* pwm);
	bool Done();
private:
	mutable std::mutex m_mutex;
	std::string m_filename;
	bool m_done;
};

// Data logger stores data received from the serial.
class CDataLogger
{
public:
	CDataLogger(std::string filename);
	virtual ~CDataLogger();

	// Store values
	void Log(std::string setpoint, std::string sensor, std::string pwm);
	void Notify();
	void WriteToFile();
private:
	void OnSignal_WriterDone();

	std::string m_filename;
	std::shared_ptr<std::vector<std::string>> m_timestamp_vector;
	std::shared_ptr<std::vector<std::string>> m_setpoint_vector;
	std::shared_ptr<std::vector<std::string>> m_sensor_vector;
	std::shared_ptr<std::vector<std::string>> m_pwm_vector;
	Glib::Dispatcher m_dispatcher;
	CDataWriter m_writer;
	std::thread* m_thread;
};

#endif