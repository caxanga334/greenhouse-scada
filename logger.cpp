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

#include "logger.h"
#include <fstream>
#include <iostream>

CDataWriter::CDataWriter(std::string filename) :
m_mutex(),
m_filename(filename),
m_done(false)
{
}

CDataWriter::~CDataWriter()
{
}

void CDataWriter::Write(CDataLogger* logger, std::vector<std::string>* timestamp, std::vector<std::string> *setpoint, std::vector<std::string> *sensor, std::vector<std::string> *pwm)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	std::string filename = "log_" + m_filename + ".log";
	std::string line;
	std::fstream filestream;

	std::cout << "[THREADED] Logging data to file " << filename << std::endl;
	filestream.open(filename, std::fstream::out | std::fstream::app);
	for (std::size_t i = 0; i < setpoint->size(); i++)
	{
		line = timestamp->at(i) + " Setpoint: " + setpoint->at(i) + " Sensor: " + sensor->at(i) + " PWM: " + pwm->at(i) + " \n";
		filestream.write(line.c_str(), line.size());
	}
	
	filestream.close();
	m_done = true;
	logger->Notify();
}

bool CDataWriter::Done()
{
	std::lock_guard<std::mutex> lock(m_mutex);
    return m_done;
}

CDataLogger::CDataLogger(std::string filename) :
m_filename(filename),
m_timestamp_vector(new std::vector<std::string>),
m_setpoint_vector(new std::vector<std::string>()),
m_sensor_vector(new std::vector<std::string>()),
m_pwm_vector(new std::vector<std::string>()),
m_dispatcher(),
m_writer(filename),
m_thread(nullptr)
{
	m_dispatcher.connect(sigc::mem_fun(*this, &CDataLogger::OnSignal_WriterDone));
}

CDataLogger::~CDataLogger()
{
	if (m_thread != nullptr)
	{
		if (m_thread->joinable())
			m_thread->join();

		delete m_thread;
		m_thread = nullptr;
	}
}

void CDataLogger::Log(std::string setpoint, std::string sensor, std::string pwm)
{
	if (m_thread != nullptr) // Don't log new data while the writer thread is working
		return;

	std::time_t time = std::time(nullptr);
	std::unique_ptr<char[]> timebuffer(new char[128]);
	std::strftime(timebuffer.get(), 128, "%Y-%m-%dT%H:%M:%SZ", std::localtime(&time));
	
	m_timestamp_vector.get()->emplace_back(timebuffer.get());
	m_setpoint_vector.get()->emplace_back(setpoint);
	m_sensor_vector.get()->emplace_back(sensor);
	m_pwm_vector.get()->emplace_back(pwm);
}

void CDataLogger::Notify()
{
	m_dispatcher.emit();
}

void CDataLogger::WriteToFile()
{
	if (m_setpoint_vector.get()->size() == 0)
		return;

	if (m_thread == nullptr)
	{
		m_thread = new std::thread(
			[this]
			{
				m_writer.Write(this, m_timestamp_vector.get(), m_setpoint_vector.get(), m_sensor_vector.get(), m_pwm_vector.get());
			});
	}
}

void CDataLogger::OnSignal_WriterDone()
{
	m_timestamp_vector.get()->clear();
	m_setpoint_vector.get()->clear();
	m_sensor_vector.get()->clear();
	m_pwm_vector.get()->clear();

	if (m_thread != nullptr && m_writer.Done())
	{
		if (m_thread->joinable())
			m_thread->join();

		delete m_thread;
		m_thread = nullptr;
	}
}
