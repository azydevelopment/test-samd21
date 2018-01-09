#include "program.h"

#include <abdeveng/core/test/common/evaluator.h>
#include <abdeveng/core/test/common/harness.h>
#include <azydev/embedded/bus/i2c/atmel/samd21/bus.h>
#include <azydev/embedded/clock/atmel/samd21/clock.h>
#include <azydev/embedded/util/binary.h>
#include <cstring>

/* PUBLIC */

CProgram::CProgram()
    : m_pins()
    , m_i2c_clock(nullptr)
    , m_i2c_bus(nullptr) {
}

CProgram::~CProgram() {
}

void CProgram::Main() {
    OnInit();

    while (1) {
        OnUpdate();
    }
}

void CProgram::OnInit() {
    // init system
    system_init();

    // init I2C clock
    {
        CClockAtmelSAMD21::DESC desc = {};
        desc.id                      = 0;
        desc.clock_gclk              = CClockAtmelSAMD21::CLOCK_GCLK::CLOCK_SERCOM2_CORE;
        desc.clock_apbc              = CClockAtmelSAMD21::CLOCK_APBC::CLOCK_SERCOM2;

        m_i2c_clock = new CClockAtmelSAMD21(desc);

        CClockAtmelSAMD21::CONFIG_DESC config = {};
        config.gclk_generator                 = CClockAtmelSAMD21::GCLK_GENERATOR::GEN0;

        m_i2c_clock->SetConfig(config);

        // leave clock enabled
        m_i2c_clock->SetEnabled(true);
    }

    // create I2C bus
    {
        // setup pin config
        CI2CBusAtmelSAMD21::PIN_CONFIG_DESC pinConfig = {};
        {
            // TODO IMPLEMENT
            pinConfig.pad0 = PINMUX_PA08D_SERCOM2_PAD0;
            pinConfig.pad1 = PINMUX_PA09D_SERCOM2_PAD1;
            pinConfig.pad2 = PINMUX_UNUSED;
            pinConfig.pad3 = PINMUX_UNUSED;
        }

        CI2CBusAtmelSAMD21::DESC desc = {};
        desc.id                       = 0;
        desc.sercomI2c                = &(SERCOM2->I2CM);
        desc.pin_config               = pinConfig;

        // create I2C bus
        // TODO: Remove dynamic memory allocation
        m_i2c_bus = new CI2CBusAtmelSAMD21(desc, m_pins);
    }

    // configure I2C bus
    {
        CI2CBusAtmelSAMD21::CONFIG_DESC busConfig = {};
        {
            busConfig.enable_run_in_standby = true;
            busConfig.sda_hold_time         = CI2CBusAtmelSAMD21::SDA_HOLD_TIME::NS_450;
            busConfig.manager_baud          = 0xff; // TODO HACK
            busConfig.enable_smart_mode     = true;
            busConfig.transfer_speed        = CI2CBusAtmelSAMD21::TRANSFER_SPEED::STANDARD;
        }

        // set config
        m_i2c_bus->SetConfig(busConfig);

        // set role
        // TODO HACK: This I2C bus only supports ROLE::MASTER
        m_i2c_bus->SetRole(CI2CEntity::ROLE::MANAGER);

        // enable I2C
        m_i2c_bus->SetEnabled(true);
    }
}

void CProgram::OnUpdate() {
    m_i2c_bus->Start(0x11, CI2CBus::TRANSFER_DIRECTION::WRITE);
    for (uint8_t i = 0; i < 8; i++) {
        m_i2c_bus->Write(i);
    }
    m_i2c_bus->Stop();
}
