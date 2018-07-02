#pragma once

#include <azydev/embedded/pins/atmel/samd21/pins.h>

#include <asf.h>

template<typename BEAT_PRIMITIVE>
class CDMAEngine;

template<typename BEAT_PRIMITIVE>
class CDMAPool;

template<typename BEAT_PRIMITIVE>
class CDMATransfer;

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
    CDMAEngine<uint8_t>* m_dma_engine;
    CDMAPool<uint8_t>* m_dma_pool;
    CDMATransfer<uint8_t>* m_dma_transfer;
    CClock* m_spi_clock;
    CSPIBus<uint8_t>* m_spi_bus;
    CSPIDevice* m_spi_device;
    CPinsAtmelSAMD21 m_pins;

    // methods
    void OnInit();
    void OnUpdate();
};
