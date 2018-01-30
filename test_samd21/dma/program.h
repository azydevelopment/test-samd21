#pragma once

#include <azydev/embedded/pins/atmel/samd21/pins.h>

#include <asf.h>

class CDMAEngineAtmelSAMD21;
class CClock;

class CProgram final
{
public:
    CProgram();
    virtual ~CProgram();
    void Main();

private:
    CClock* m_dma_clock;
    CDMAEngineAtmelSAMD21* m_dma_engine;

    // methods
    void OnInit();
    void OnUpdate();
};
