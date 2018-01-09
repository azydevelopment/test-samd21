#pragma once

#include <azydev/embedded/logging/atmel/samd21/logger.h>
#include <azydev/embedded/pins/atmel/samd21/pins.h>

#include <asf.h>

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
    static const uint8_t SPI_BUS_ID;
    static const uint8_t NUM_SPI_BUS_DEVICES;
    static const uint8_t SPI_BUS_DEVICE_0_ID;
    static const uint8_t SPI_BUS_DEVICE_0_SS_PIN;

    // member variables
    CPinsAtmelSAMD21 m_pins;
    CClock* m_spi_clock;
    CSPIBus<uint16_t>* m_spi_bus;
    CSPIDevice* m_spi_device;

    // methods
    void OnInit();
    void OnUpdate();
};
