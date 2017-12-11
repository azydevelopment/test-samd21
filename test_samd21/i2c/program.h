#pragma once

#include <azydev/embedded/logging/atmel/samd21/logger.h>
#include <azydev/embedded/pins/atmel/samd21/pins.h>

class CI2CBus;

class CProgram final {
	public:

	CProgram();
	virtual ~CProgram();
	void Main();

	private:
	
	// member variables
	CLoggerAtmelSAMD21 m_logger;
	CPinsAtmelSAMD21 m_pins;
	CI2CBus* m_i2c_bus;

	// methods
	void OnInit();
	void OnUpdate();
};

