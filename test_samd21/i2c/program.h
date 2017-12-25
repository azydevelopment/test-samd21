#pragma once

#include <azydev/embedded/logging/atmel/samd21/logger.h>
#include <azydev/embedded/pins/atmel/samd21/pins.h>

class CI2CBus;
class CClock;

class CProgram final
{
public:
    CProgram();
    virtual ~CProgram();
    void Main();

private:
    // member variables
    CPinsAtmelSAMD21 m_pins;
    CClock* m_i2c_clock;
    CI2CBus* m_i2c_bus;

    // methods
    void OnInit();
    void OnUpdate();
};
