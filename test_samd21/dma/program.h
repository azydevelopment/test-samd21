#pragma once

#include <azydev/embedded/pins/atmel/samd21/pins.h>

#include <asf.h>

class CDMAEngine;
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
    CDMAEngine* m_dma_engine;
    CDMATransfer* m_dma_transfer;

    // methods
    void OnInit();
    void OnUpdate();
};
