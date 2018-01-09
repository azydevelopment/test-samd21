#include "program.h"

#include <azydev/embedded/bus/spi/atmel/samd21/bus.h>
#include <azydev/embedded/bus/spi/atmel/samd21/device.h>
#include <azydev/embedded/clock/atmel/samd21/clock.h>
#include <cmath>
#include <cstring>

/* STATICS */

const uint8_t CProgram::SPI_BUS_ID              = 0;
const uint8_t CProgram::NUM_SPI_BUS_DEVICES     = 1;
const uint8_t CProgram::SPI_BUS_DEVICE_0_ID     = 0;
const uint8_t CProgram::SPI_BUS_DEVICE_0_SS_PIN = PIN_PA05;
const uint8_t CProgram::NUM_TEST_CASES          = 1;

/* PUBLIC */

CProgram::CProgram()
    : m_pins()
    , m_spi_clock(nullptr)
    , m_spi_bus(nullptr)
    , m_spi_device(nullptr)
    , m_transcoder_max7219(CTranscoderMAX7219<uint16_t>())
    , m_time(0) {
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

    // init SPI clock
    {
        CClockAtmelSAMD21::DESC desc = {};
        desc.id                      = 0;
        desc.clock_gclk              = CClockAtmelSAMD21::CLOCK_GCLK::CLOCK_SERCOM2_CORE;
        desc.clock_apbc              = CClockAtmelSAMD21::CLOCK_APBC::CLOCK_SERCOM2;

        m_spi_clock = new CClockAtmelSAMD21(desc);

        CClockAtmelSAMD21::CONFIG_DESC config = {};
        config.generator                      = CClockAtmelSAMD21::CLOCK_GENERATOR::GCLKGEN0;

        m_spi_clock->SetConfig(config);

        // leave clock enabled
        m_spi_clock->SetEnabled(true);
    }

    // init SPI target device
    {
        // init pin config
        CSPIDeviceAtmelSAMD21::PIN_CONFIG_DESC pinConfig = {};
        { pinConfig.ss = SPI_BUS_DEVICE_0_SS_PIN; }

        CSPIDeviceAtmelSAMD21::DESC desc = {};
        desc.id                          = SPI_BUS_DEVICE_0_ID;
        desc.pin_config                  = pinConfig;

        // create SPI target device
        // TODO HACK: Usage of 'new'
        m_spi_device = new CSPIDeviceAtmelSAMD21(desc, m_pins);
    }

    // create SPI bus
    {
        // setup pin config
        CSPIBusAtmelSAMD21::PIN_CONFIG_DESC pinConfig = {};
        {
            pinConfig.pad0           = PINMUX_PA08D_SERCOM2_PAD0;
            pinConfig.pad1           = PINMUX_UNUSED;
            pinConfig.pad2           = PINMUX_PA10D_SERCOM2_PAD2;
            pinConfig.pad3           = PINMUX_PA11D_SERCOM2_PAD3;
            pinConfig.data_in_pinout = CSPIBusAtmelSAMD21::DATA_IN_PINOUT::PAD_0;
            pinConfig.data_out_pinout =
                CSPIBusAtmelSAMD21::DATA_OUT_PINOUT::DO_PAD2_SCK_PAD3_SS_PAD1;
        }

        CSPIBusAtmelSAMD21::DESC desc = {};
        desc.id                       = SPI_BUS_ID;
        desc.sercomSpi                = &(SERCOM2->SPI);
        desc.pin_config               = pinConfig;
        desc.num_devices              = NUM_SPI_BUS_DEVICES;
        desc.devices                  = &m_spi_device;

        // create SPI bus
        // TODO HACK: Usage of 'new'
        m_spi_bus = new CSPIBusAtmelSAMD21(desc, m_pins);
    }

    // configure SPI bus
    {
        // create config
        CSPIBusAtmelSAMD21::CONFIG_DESC busConfig = {};
        {
            busConfig.endianness         = CSPIBusAtmelSAMD21::ENDIANNESS::BIG;
            busConfig.duplex_mode_intial = CSPIBusAtmelSAMD21::DUPLEX_MODE::FULL;
            busConfig.clock_polarity     = CSPIBusAtmelSAMD21::CLOCK_POLARITY::IDLE_LOW;
            busConfig.clock_phase        = CSPIBusAtmelSAMD21::CLOCK_PHASE::SAMPLE_LEADING;
            busConfig.frame_format       = CSPIBusAtmelSAMD21::FRAME_FORMAT::SPI;
            busConfig.immediate_buffer_overflow_notification = false;
            busConfig.run_in_standby                         = false;
            busConfig.address_mode                    = CSPIBusAtmelSAMD21::ADDRESS_MODE::MASK;
            busConfig.enable_manager_worker_select    = false;
            busConfig.enable_worker_select_low_detect = false;
            busConfig.enable_worker_data_preload      = false;
            busConfig.character_size                  = CSPIBusAtmelSAMD21::CHARACTER_SIZE::BITS_8;
            busConfig.baud_rate                       = 128;
            busConfig.enable_interrupt_error          = false;
            busConfig.enable_interrupt_worker_select_low   = false;
            busConfig.enable_interrupt_receive_complete    = false;
            busConfig.enable_interrupt_transmit_complete   = false;
            busConfig.enable_interrupt_data_register_empty = false;
        }

        // set role
        m_spi_bus->SetRole(CSPIEntity::ROLE::MANAGER);

        // set config
        m_spi_bus->SetConfig(busConfig);

        // enable SPI
        m_spi_bus->SetEnabled(true);
    }

    m_spi_bus->SetDeviceRole(SPI_BUS_DEVICE_0_ID, CSPIEntity::ROLE::WORKER);

    // set up MAX7219 transcoder
    {
        m_transcoder_max7219.AttachToBus(*m_spi_bus, SPI_BUS_DEVICE_0_ID);
        m_transcoder_max7219.SetEnabled(false);
        m_transcoder_max7219.SetDecodeMode(0);
        m_transcoder_max7219.SetScanLimit(7);
        m_transcoder_max7219.SetIntensity(1);
        m_transcoder_max7219.SetEnabled(true);
    }
}

void CProgram::OnUpdate() {
    m_time += 0.01;
    if (m_time > 1000000) {
        m_time = 0;
    }

    uint8_t x = static_cast<uint8_t>(sin(10000 * m_time + 3.14159265358f / 2) * 2.5 + 4);
    uint8_t y = static_cast<uint8_t>(sin(10007 * m_time) * 2.5 + 4);

    m_spi_clock->SetEnabled(true);

    for (uint8_t i = 0; i < 8; i++) {
        if (i == x) {
            m_transcoder_max7219.SetRow(i, 1 << y);
        } else {
            m_transcoder_max7219.SetRow(i, 0);
        }
    }

    m_spi_clock->SetEnabled(false);
}
