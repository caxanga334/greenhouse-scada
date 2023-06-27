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

#include "serialmanager.h"
#include "app.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <utility>

// Update is called every 500 ms

#define DELAY_BETWEEN_COMMANDS 5
#define DELAY_BETWEEN_READS 2
#define DELAY_NO_COMMANDS 1
#define SERIAL_READ_TIMEOUT_MS 2000 // timeout for serial read operations in milliseconds

CSerialCommand::CSerialCommand(std::string rawcommand)
{
	m_string = rawcommand;
	m_setpoint = "";
	m_sensor = "";
	m_pwm = "";
	m_type = SETPOINT_INVALID;
}

void CSerialCommand::Parse()
{
	// example of a command: sdt_24.00_19.83_255.00?
	// the string is pre-filtered by the serial reader
	auto strvector = Explode();

	if (strvector.size() < 4) // valid commands have at least 4 strings
	{
		return;
	}

	auto type = strvector[0];
	auto data1 = strvector[1];
	auto data2 = strvector[2];
	auto data3 = strvector[3];

	if (type == "sdt")
	{
		m_type = SETPOINT_TEMPERATURE;
	}
	else if (type == "sdl")
	{
		m_type = SETPOINT_LED;
	}
	else if (type == "sdh")
	{
		m_type = SETPOINT_HUMIDITY;
	}
	else
	{
		std::cout << "Warning: Unrecognized command \"" << m_string << "\"" << std::endl;
		return;
	}

	m_setpoint = data1;
	m_sensor = data2;
	m_pwm = data3;

	std::string charset = "UTF-8";
	std::string fromcharset = "ASCII";

	m_setpoint = Glib::convert(m_setpoint, charset, fromcharset);
	m_sensor = Glib::convert(m_sensor, charset, fromcharset);
	m_pwm = Glib::convert(m_pwm, charset, fromcharset);
}

// Splits the command string in a vector of strings
const std::vector<std::string> CSerialCommand::Explode()
{
	std::string str = m_string;
	const char delimiter = '_';

	std::vector<std::string> result;
	std::istringstream iss(str);

	for (std::string token; std::getline(iss, token, delimiter); )
	{
		result.push_back(std::move(token));
	}

	return result;
}

CSerialReceiver::CSerialReceiver() :
m_Mutex(),
m_done(false),
m_message()
{
}

void CSerialReceiver::Update(CSerialManager *caller, serialib *serialib)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	constexpr int size = 40;
	std::unique_ptr<char[]> buffer(new char[size]);
	serialib->readString(buffer.get(), '?', size, SERIAL_READ_TIMEOUT_MS);
	std::string serialbuffer = std::string(buffer.get());
	std::cout << "[THREADED] Received buffer from serial: " << serialbuffer << std::endl;
	m_message = serialbuffer;
	FormatCommand();
	m_done = true;
	caller->Notify_SerialReceiver();
}

void CSerialReceiver::FormatCommand()
{
	std::string command = m_message;

	command.erase(std::remove(command.begin(), command.end(), '\r'), command.cend());
	command.erase(std::remove(command.begin(), command.end(), '\n'), command.cend());
	// auto npos = command.find('\n');

	// while (npos != std::string::npos) // remove any new lines that get added by println
	// {
	// 	command.erase(npos);
	// 	npos = command.find('\n');
	// }
	
	auto startpos = command.find('s');
	auto endpos = command.find('?');

	if (startpos == std::string::npos || endpos == std::string::npos)
	{
		m_message = std::string("");
		return;
	}

	std::string subcommand = command.substr(startpos, endpos);
	m_message = subcommand;
}

void CSerialReceiver::GetCommand(std::string *command)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	
	if (command)
	{
		*command = m_message;
	}
}

bool CSerialReceiver::Done() const
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_done;
}

CSerialManager::CSerialManager() :
m_serialcfg(),
m_cmd_queue(),
m_last_cmd(""),
m_receiverdispatcher(),
m_receiverworker(),
m_receiverthread(nullptr)
{
	m_serialib = std::make_shared<serialib>();
	m_timer = DELAY_NO_COMMANDS;
	m_receiverdispatcher.connect(sigc::mem_fun(*this, &CSerialManager::OnSignal_ReceiveCommand));
	m_mainwindow = nullptr;
}

CSerialManager::~CSerialManager()
{
	m_mainwindow = nullptr;

	if (m_receiverthread != nullptr)
	{
		if (m_receiverthread->joinable())
			m_receiverthread->join();

		delete m_receiverthread;
		m_receiverthread = nullptr;
	}
}

bool CSerialManager::ReadConfigFile()
{
	if (m_serialcfg.configurated)
	{
		return true;
	}

	std::fstream filestream;
	const char* configfile = "serial.cfg";

	filestream.open(configfile, std::ios::in);

	if (!filestream.is_open())
	{
		std::cout << "Failed to read serial.cfg!" << std::endl;
		return false;
	}
	
	std::string line;

	while (std::getline(filestream, line))
	{
		if (line.find("//", 0, 2) != std::string::npos)
		{
			continue;
		}

		if (std::isspace(line[0]))
		{
			continue;
		}

		ReadConfigLine(line);
	}
	
	filestream.close();
	m_serialcfg.configurated = true;

	return true;
}

bool CSerialManager::OpenConnection()
{
	if (!ReadConfigFile())
	{
		return false;
	}

	char result = m_serialib->openDevice(m_serialcfg.devicename.c_str(), m_serialcfg.baudrate, m_serialcfg.databits, m_serialcfg.parity, m_serialcfg.stopbits);
	bool ret = false;

	switch (result)
	{
	case 1:
		ret = true;
		std::cout << "Serial connection open!" << std::endl;
		std::cout << "Device: " << m_serialcfg.devicename << " - Baud rate: " << m_serialcfg.baudrate << std::endl; 
		break;
	case -1:
		std::cout << "Failed to open serial connection. Error: Device " << m_serialcfg.devicename << " was not found!" << std::endl;
		break;
	case -2:
		std::cout << "Failed to open serial connection. Error while opening the device " << m_serialcfg.devicename << std::endl;
#ifdef __linux__
		std::cout << "On linux, this may also indicate that the device is not found." << std::endl;
		std::cout << "This will likely occur if the \"serial.cfg\" file is saved with CRLF instead of LF." << std::endl;
#endif
		break;
	case -3:
		std::cout << "Failed to open serial connection. Error while getting port parameters." << std::endl;
		break;
	case -4:
		std::cout << "Failed to open serial connection. Speed (baud rate) " << m_serialcfg.baudrate << " not recognized." << std::endl;
		break;
	case -5:
		std::cout << "Failed to open serial connection. Error while writing port parameters." << std::endl;
		break;
	case -6:
		std::cout << "Failed to open serial connection. Error while writing timeout parameters." << std::endl;
		break;
	default:
		std::cout << "Failed to open serial connection. Unhandled error code " << static_cast<int>(result) << std::endl;
		break;
	}

	return ret;
}

bool CSerialManager::IsConnected()
{
	return m_serialib->isDeviceOpen();
}

bool CSerialManager::IsAvailable()
{
	return m_serialib->available() > 0;
}

bool CSerialManager::ReloadConfig()
{
	if (IsConnected())
	{
		m_serialib->closeDevice();
	}

	m_serialcfg.configurated = false;
	return ReadConfigFile();
}

void CSerialManager::SendCommand(const SerialCommand cmd, const SetpointType spt, const float data)
{
	std::string command;

	switch (cmd)
	{
	case SERIAL_CMD_POWER_OFF:
		command = "coff?";
		SendCommandInternal(command);
		break;
	case SERIAL_CMD_POWER_ON:
		command = "con?";
		SendCommandInternal(command);
		break;
	case SERIAL_CMD_SETPOINT:
		command = FormatSetpointCommand(spt, data);
		SendCommandInternal(command);
		break;
	default:
		break;
	}

	std::cout << "CSerialManager::SendCommand -- \"" << command << "\" " << std::endl;
}

void CSerialManager::Update()
{
	if (!IsConnected())
		return;

	// Very basic timer, update is called approximately every 100 ms
	if (m_timer > 0)
	{
		m_timer--;
	}
	else
	{
		OnTimerLow();
	}
}

void CSerialManager::Notify_SerialReceiver()
{
	m_receiverdispatcher.emit();
}

void CSerialManager::OnSignal_ReceiveCommand()
{
	if (m_receiverthread && m_receiverworker.Done())
	{
		std::string command = std::string("");
		m_receiverworker.GetCommand(&command);
		std::cout << "Received command from multi-threaded serial reader: " << command << std::endl;
		m_last_cmd = command;

		if (m_receiverthread->joinable())
			m_receiverthread->join();

		delete m_receiverthread;
		m_receiverthread = nullptr;
		ProcessReceivedCommand();
	}
}

// Reads a single line from the config file
void CSerialManager::ReadConfigLine(const std::string line)
{
	auto spaceat = line.find(' ');
	auto newline = line.substr(0, spaceat);
	auto delimiterat = line.find(':');
	auto setting = line.substr(0, delimiterat);
	auto value = line.substr(delimiterat + 1);

	if (setting == "DeviceName")
	{
		m_serialcfg.devicename = value;
	}
	else if (setting == "BaudRate")
	{
		m_serialcfg.baudrate = std::stoi(value);
	}
	else if (setting == "Databits")
	{
		if (value == "SERIAL_DATABITS_5")
		{
			m_serialcfg.databits = SERIAL_DATABITS_5;
		}
		else if (value == "SERIAL_DATABITS_6")
		{
			m_serialcfg.databits = SERIAL_DATABITS_6;
		}
		else if (value == "SERIAL_DATABITS_7")
		{
			m_serialcfg.databits = SERIAL_DATABITS_7;
		}
		else if (value == "SERIAL_DATABITS_8")
		{
			m_serialcfg.databits = SERIAL_DATABITS_8;
		}
		else if (value == "SERIAL_DATABITS_16")
		{
			m_serialcfg.databits = SERIAL_DATABITS_16;
		}
	}
	else if (setting == "Parity")
	{
		if (value == "SERIAL_PARITY_NONE")
		{
			m_serialcfg.parity = SERIAL_PARITY_NONE;
		}
		else if (value == "SERIAL_PARITY_EVEN")
		{
			m_serialcfg.parity = SERIAL_PARITY_EVEN;
		}
		else if (value == "SERIAL_PARITY_ODD")
		{
			m_serialcfg.parity = SERIAL_PARITY_ODD;
		}
		else if (value == "SERIAL_PARITY_MARK")
		{
			m_serialcfg.parity = SERIAL_PARITY_MARK;
		}
		else if (value == "SERIAL_PARITY_SPACE")
		{
			m_serialcfg.parity = SERIAL_PARITY_SPACE;
		}
	}
	else if (setting == "Stopbits")
	{
		if (value == "SERIAL_STOPBITS_1")
		{
			m_serialcfg.stopbits = SERIAL_STOPBITS_1;
		}
		else if (value == "SERIAL_STOPBITS_1_5")
		{
			m_serialcfg.stopbits = SERIAL_STOPBITS_1_5;
		}
		else if (value == "SERIAL_STOPBITS_2")
		{
			m_serialcfg.stopbits = SERIAL_STOPBITS_2;
		}
	}
}

void CSerialManager::OnTimerLow()
{
	if (m_cmd_queue.size() > 0) // there are commands to be sent to the microcontroller
	{
		m_timer = DELAY_BETWEEN_COMMANDS;
		std::string command = m_cmd_queue.front();
		m_cmd_queue.pop();
		m_serialib->writeString(command.c_str());
		std::cout << "Command written to serial: \"" << command << "\"" << std::endl;
	}
	else if (m_receiverthread == nullptr) // We are not reading something already
	{
		// std::cout << "Trying to reading serial..." << std::endl;
		m_timer = DELAY_BETWEEN_READS;
		ReceiveCommandInternal();
	}
	else
	{
		m_timer = DELAY_NO_COMMANDS;
	}
}

void CSerialManager::SendCommandInternal(const std::string cmd)
{
	if (cmd.length() < 2)
		return;

	std::cout << "Command received: " << cmd << std::endl;
	m_cmd_queue.push(cmd);
}

void CSerialManager::ReceiveCommandInternal()
{
	if (m_serialib->available() <= 0)
	{
		std::cout << "No serial data available" << std::endl;
		return;
	}

	m_receiverthread = new std::thread(
		[this]
		{
			m_receiverworker.Update(this, m_serialib.get());
		});

	// constexpr int size = 40;
	// std::unique_ptr<char[]> buffer(new char[size]);
	// m_serialib->readString(buffer.get(), '?', size, SERIAL_READ_TIMEOUT_MS);
	// std::string command = std::string(buffer.get());
	// std::cout << "Received buffer from serial: " << command << std::endl;
}

void CSerialManager::ProcessReceivedCommand()
{
	std::unique_ptr<CSerialCommand> command (new CSerialCommand(m_last_cmd));
	command->Parse();

	if (command->GetType() != SETPOINT_INVALID)
	{
		// std::cout << "Last received command is valid!" << std::endl;
		m_mainwindow->OnReceiveSerialCommand(command.get());
	}
}

std::string CSerialManager::FormatSetpointCommand(const SetpointType type, const float data)
{
	std::string command;

	switch (type)
	{
	case SETPOINT_TEMPERATURE:
		command = "cspt_%.2f?";
		break;
	case SETPOINT_LED:
		command = "cspl_%.2f?";
		break;
	case SETPOINT_HUMIDITY:
		command = "csph_%.2f?";
		break;
	default:
		return std::string("");
		break;
	}

	// format using C functions since std::format is not available (C++20, we are at C++17)

	std::unique_ptr<char[]> buffer(new char[256]);
	std::sprintf(buffer.get(), command.c_str(), data);
	std::string retcmd = std::string(buffer.get());

	auto ntpos = retcmd.find('\0'); // find the null terminator if it has one
	if (ntpos != std::string::npos)
	{
		retcmd.erase(ntpos);
	}

	// HACK: Some languages uses comma as a decimal separator, the microcontroller expects a dot as a decimal separator
	auto commapos = retcmd.find(',');
	if (commapos != std::string::npos)
	{
		std::string dot = std::string(".");
		retcmd.replace(commapos, 1, dot);
	}
	
	retcmd.shrink_to_fit();
	return retcmd;
}
