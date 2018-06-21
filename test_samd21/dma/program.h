#pragma once

#include <azydev/embedded/pins/atmel/samd21/pins.h>

#include <asf.h>

template<typename BEAT_PRIMITIVE>
class CDMAEngine;

template<typename BEAT_PRIMITIVE>
class CDMAPool;

template<typename BEAT_PRIMITIVE>
class CDMATransfer;

class CClock;

class CProgram final
{
public:
    CProgram();
    virtual ~CProgram();
    void Main();

private:
    CClock* m_dma_clock;
    CDMAEngine<uint8_t>* m_dma_engine;
    CDMAPool<uint8_t>* m_dma_pool;
    CDMATransfer<uint8_t>* m_dma_transfer;

    // methods
    void OnInit();
    void OnUpdate();
};
