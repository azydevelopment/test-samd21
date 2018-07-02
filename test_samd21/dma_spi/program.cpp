#include "program.h"

#include <azydev/embedded/bus/spi/atmel/samd21/bus.h>
#include <azydev/embedded/bus/spi/atmel/samd21/device.h>
#include <azydev/embedded/clock/atmel/samd21/clock.h>
#include <azydev/embedded/dma/atmel/samd21/engine.h>
#include <azydev/embedded/dma/common/pool.h>

/* FILE SCOPED STATICS */

static const uint8_t SPI_BUS_ID              = 0;
static const uint8_t NUM_SPI_BUS_DEVICES     = 1;
static const uint8_t SPI_BUS_DEVICE_0_ID     = 0;
static const uint8_t SPI_BUS_DEVICE_0_SS_PIN = PIN_PA05;
static const uint16_t NUM_BYTES              = 4;

/* PUBLIC */

CProgram::CProgram()
    : m_dma_clock(nullptr)
    , m_dma_engine(nullptr)
    , m_dma_pool(nullptr)
    , m_dma_transfer(nullptr)
    , m_spi_clock(nullptr)
    , m_spi_bus(nullptr)
    , m_spi_device(nullptr)
    , m_pins(CPinsAtmelSAMD21()) {
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

    // init DMA clock
    {
        CClockAtmelSAMD21::DESC desc = {};
        desc.id                      = 0;
        desc.clock_ahb               = CClockAtmelSAMD21::CLOCK_AHB::CLOCK_DMAC;
        desc.clock_apbb              = CClockAtmelSAMD21::CLOCK_APBB::CLOCK_DMAC;

        m_dma_clock = new CClockAtmelSAMD21(desc);

        // no additional config needed

        // leave clock enabled
        m_dma_clock->SetEnabled(true);
    }

    // init DMA engine
    {
        CDMAEngineAtmelSAMD21<uint8_t>::DESC desc = {};

        // create DMA engine
        m_dma_engine = new CDMAEngineAtmelSAMD21<uint8_t>(desc);

        // enable DMA engine
        m_dma_engine->SetEnabled(true);
    }

    // init DMA pool
    {
        CDMAPool<uint8_t>::DESC descPool = {};
        descPool.num_allocations_max     = 1;
        descPool.num_beats_max           = NUM_BYTES;

        m_dma_pool = new CDMAPool<uint8_t>(descPool);
    }

    // init DMA transfer object
    {
        CDMATransferAtmelSAMD21<uint8_t>::DESC desc = {};
        desc.id_initial                             = 0;
        desc.num_steps_max                          = 1;

        // create DMA transfer
        m_dma_transfer = new CDMATransferAtmelSAMD21<uint8_t>(desc);
    }

    // init SPI clock
    {
        CClockAtmelSAMD21::DESC desc = {};
        desc.id                      = 0;
        desc.clock_gclk              = CClockAtmelSAMD21::CLOCK_GCLK::CLOCK_SERCOM2_CORE;
        desc.clock_apbc              = CClockAtmelSAMD21::CLOCK_APBC::CLOCK_SERCOM2;

        m_spi_clock = new CClockAtmelSAMD21(desc);

        CClockAtmelSAMD21::CONFIG_DESC config = {};
        config.gclk_generator                 = CClockAtmelSAMD21::GCLK_GENERATOR::GEN0;

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
            // TODO IMPLEMENT
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
        desc.pin_config               = pinConfig;
        desc.sercomSpi                = &(SERCOM2->SPI);
        desc.num_devices              = NUM_SPI_BUS_DEVICES;
        desc.devices                  = &m_spi_device;

        // create SPI bus
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
            busConfig.clock_phase        = CSPIBusAtmelSAMD21::CLOCK_PHASE::SAMPLE_TRAILING;
            busConfig.frame_format       = CSPIBusAtmelSAMD21::FRAME_FORMAT::SPI;
            busConfig.immediate_buffer_overflow_notification = false;
            busConfig.run_in_standby                         = false;
            busConfig.address_mode                         = CSPIBusAtmelSAMD21::ADDRESS_MODE::MASK;
            busConfig.enable_manager_worker_select         = false;
            busConfig.enable_worker_select_low_detect      = false;
            busConfig.enable_worker_data_preload           = false;
            busConfig.baud_rate                            = 128;
            busConfig.enable_interrupt_error               = false;
            busConfig.enable_interrupt_worker_select_low   = false;
            busConfig.enable_interrupt_receive_complete    = false;
            busConfig.enable_interrupt_transmit_complete   = false;
            busConfig.enable_interrupt_data_register_empty = false;

            // DMA
            busConfig.dma_transfer_id = 0;
            busConfig.dma_engine      = m_dma_engine;
            busConfig.dma_transfer    = m_dma_transfer;
        }

        // set role
        m_spi_bus->SetRole(CSPIEntity::ROLE::MANAGER);

        // set config
        m_spi_bus->SetConfig(busConfig);

        // enable SPI
        m_spi_bus->SetEnabled(true);
    }

    m_spi_bus->SetDeviceRole(SPI_BUS_DEVICE_0_ID, CSPIEntity::ROLE::WORKER);
}

void CProgram::OnUpdate() {
    // reset DMA pool
    m_dma_pool->PopAllocation();

    // prepare transfer node
    const IDMANode<uint8_t>* nodeSrc = nullptr;
    {
        // prepare source node
        uint8_t allocationId = 0;
        m_dma_pool->PushAllocation(allocationId);
        for (uint32_t j = 0; j < NUM_BYTES; j++) {
            m_dma_pool->RecordWrite(j);
        }
        nodeSrc = m_dma_pool->GetAllocationDMANode(allocationId);
    }

    // execute the SPI transfer
    {
        m_spi_bus->Start(SPI_BUS_DEVICE_0_ID);

        // uint16_t data;
        // m_spi_bus->Read(data);
        // data = 0;

        CSPIBusAtmelSAMD21* spiBusSAMD21 = static_cast<CSPIBusAtmelSAMD21*>(m_spi_bus);

        spiBusSAMD21->Write(*nodeSrc, NUM_BYTES);

        m_spi_bus->Stop();
    }
}
