#pragma once

#include <azydev/embedded/logging/atmel/samd21/logger.h>
#include <azydev/embedded/pins/atmel/samd21/pins.h>

#include <asf.h>

class CI2CBus;

class CProgram final
{
	public:

	CProgram();
	~CProgram();
	void Main();

	private:
	
	// member variables
	CLoggerAtmelSAMD21 m_logger;
	CPinsAtmelSAMD21 m_pins;
	CI2CBus* m_i2c_bus;

private:
	// methods
	void OnInit();
	void OnUpdate();
};

