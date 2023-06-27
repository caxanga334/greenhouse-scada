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

#ifndef _H_SERIAL_MANAGER_
#define _H_SERIAL_MANAGER_

#include <gtkmm.h>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>

class MainWindow;

// BUG? serialib must be included after gtkmm.h or else you get 100+ errors
#include "lib/serialib.h"

enum SerialCommand
{
	SERIAL_CMD_INVALID = 0,
	SERIAL_CMD_POWER_OFF,
	SERIAL_CMD_POWER_ON,
	SERIAL_CMD_SETPOINT,

	SERIAL_CMD_TYPE_COUNT
};

enum SetpointType
{
	SETPOINT_INVALID = 0,
	SETPOINT_TEMPERATURE,
	SETPOINT_LED,
	SETPOINT_HUMIDITY,

	SETPOINT_TYPE_COUNT
};

class CSerialManager;

// Represents a single command received from serial
class CSerialCommand
{
public:
	CSerialCommand(std::string rawcommand);

	void Parse();

	inline std::string GetSetpointData() const { return m_setpoint; }
	inline std::string GetSensorData() const { return m_sensor; }
	inline std::string GetPWMData() const { return m_pwm; }
	inline SetpointType GetType() const { return m_type; }
private:
	const std::vector<std::string> Explode();

	std::string m_string;
	std::string m_setpoint;
	std::string m_sensor;
	std::string m_pwm;
	SetpointType m_type;
};

// Multi-threaded serial receiver worker
class CSerialReceiver
{
public:
	CSerialReceiver();

	void Update(CSerialManager* caller, serialib* serialib);
	void FormatCommand();
	void GetCommand(std::string* command);
	bool Done() const;
private:
	// Synchronizes access to member data.
	mutable std::mutex m_Mutex;

	bool m_done;
	std::string m_message;
};

class CSerialConfiguration
{
public:
	CSerialConfiguration() :
	devicename()
	{
		baudrate = 0;
		databits = SERIAL_DATABITS_5;
		stopbits = SERIAL_STOPBITS_1;
		parity = SERIAL_PARITY_NONE;
		configurated = false;
	}

	std::string devicename;
	unsigned int baudrate;
	SerialDataBits databits;
	SerialStopBits stopbits;
	SerialParity parity;
	bool configurated; // True if the serial is configurated and not just using default values
};

class CSerialManager
{
public:
	CSerialManager();
	virtual ~CSerialManager();

	/// @brief Opens and read the serial configuration file
	/// @return true if the file was read successfully
	bool ReadConfigFile();
	/// @brief Tries to open a serial connection
	/// @return true if the connection was open, false otherwise
	bool OpenConnection();
	bool IsConnected();
	/// @brief Checks if there is data available at the serial port
	/// @return true if there is at least 1 byte available in the serial data
	bool IsAvailable();
	bool ReloadConfig();
	void SendCommand(const SerialCommand cmd, const SetpointType spt = SETPOINT_INVALID, const float data = 0.0f);
	void Update();
	void Notify_SerialReceiver();

	void SetMainWindow(MainWindow* window) { m_mainwindow = window; }

protected:
	void OnSignal_ReceiveCommand();

private:
	void ReadConfigLine(const std::string line);
	bool CheckWrite();
	void CheckRead();
	void WriteNextCommand();
	void SendCommandInternal(const std::string cmd);
	void ReceiveCommandInternal();
	void ProcessReceivedCommand();
	std::string FormatSetpointCommand(const SetpointType type, const float data);

	CSerialConfiguration m_serialcfg;
	std::shared_ptr<serialib> m_serialib;
	int m_writetimer;
	int m_readtimer;
	std::queue<std::string> m_cmd_queue;
	std::string m_last_cmd; // Last received command from the microcontroller
	Glib::Dispatcher m_receiverdispatcher;
	CSerialReceiver m_receiverworker;
	std::thread* m_receiverthread;
	MainWindow* m_mainwindow;
};

#endif