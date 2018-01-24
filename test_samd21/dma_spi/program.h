#pragma once

#include <azydev/embedded/pins/atmel/samd21/pins.h>

#include <asf.h>

template<typename TRANSFER_PRIMITIVE>
class CSPIBus;
class CSPIDevice;
class CDMAEngine;
class CClock;

class CProgram final
{
public:
    CProgram();
    virtual ~CProgram();
    void Main();

private:
    static const uint8_t SPI_BUS_ID;
    static const uint8_t NUM_SPI_BUS_DEVICES;
    static const uint8_t SPI_BUS_DEVICE_0_ID;
    static const uint8_t SPI_BUS_DEVICE_0_SS_PIN;
	static const uint8_t SPI_TEST_DATA_LENGTH = 8;
	static const uint8_t SPI_TEST_DATA[SPI_TEST_DATA_LENGTH];

    // member variables
    CPinsAtmelSAMD21 m_pins;

    CClock* m_spi_clock;
    CClock* m_dma_clock;

    CSPIBus<uint16_t>* m_spi_bus;
    CSPIDevice* m_spi_device;

    CDMAEngine* m_dma_engine;

    // methods
    void OnInit();
    void OnUpdate();
};
