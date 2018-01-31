#pragma once

#include <azydev/embedded/pins/atmel/samd21/pins.h>
#include <azydev/embedded/pins/atmel/samd21/pins.h>

#include <asf.h>

class CDMAEngine;
class CClock;

template<typename TRANSFER_PRIMITIVE>
class CSPIBus;
class CSPIDevice;
class CClock;

class CProgram final
{
public:
	CProgram();
	virtual ~CProgram();
	void Main();

private:
	// member variables
	CClock* m_dma_clock;
	CDMAEngine* m_dma_engine;
	CClock* m_spi_clock;
	CSPIBus<uint16_t>* m_spi_bus;
	CSPIDevice* m_spi_device;
	CPinsAtmelSAMD21 m_pins;
	
	// methods
	void OnInit();
	void OnUpdate();
};
