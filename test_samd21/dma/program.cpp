#include "program.h"

#include <azydev/embedded/clock/atmel/samd21/clock.h>
#include <azydev/embedded/dma/atmel/samd21/engine.h>
#include <cstring>

/* PUBLIC */

CProgram::CProgram()
    : m_dma_clock(nullptr)
    , m_dma_engine(nullptr) {
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
        CDMAEngineAtmelSAMD21::DESC desc = {};

        // create DMA engine
        m_dma_engine = new CDMAEngineAtmelSAMD21(desc);
		
		// enable DMA engine
		m_dma_engine->SetEnabled(true);
    }
}

const uint8_t NUM_BYTES = 8;
uint8_t dataSrc[NUM_BYTES] = {0,1,2,3,4,5,6,7};
uint8_t dataDst[NUM_BYTES] = {};

void CProgram::OnUpdate() {
    // init DMA transfer
    CDMAChannelAtmelSAMD21::TRANSFER_DESC transferDesc = {};
    {
        transferDesc.priority            = CDMAChannelAtmelSAMD21::PRIORITY::LVL_0;
        transferDesc.trigger             = CDMAChannelAtmelSAMD21::TRIGGER::SOFTWARE_OR_EVENT;
        transferDesc.trigger_action      = CDMAChannelAtmelSAMD21::TRIGGER_ACTION::START_TRANSACTION;
        transferDesc.enable_event_output = false;
        transferDesc.enable_event_input  = false;
        transferDesc.event_input_action  = CDMAChannelAtmelSAMD21::EVENT_INPUT_ACTION::NOACT;
        transferDesc.event_output_selection =
            CDMAChannelAtmelSAMD21::DESCRIPTOR::EVENT_OUTPUT_SELECTION::DISABLED;
        transferDesc.block_completed_action =
            CDMAChannelAtmelSAMD21::DESCRIPTOR::BLOCK_COMPLETED_ACTION::DISABLE_IF_LAST;
        transferDesc.beat_size = CDMAChannelAtmelSAMD21::DESCRIPTOR::BEAT_SIZE::BITS_8;
        transferDesc.enable_source_address_increment      = true;
        transferDesc.enable_destination_address_increment = true;
        transferDesc.step_size_select =
            CDMAChannelAtmelSAMD21::DESCRIPTOR::STEP_SIZE_SELECT::DESTINATION;
        transferDesc.step_size           = CDMAChannelAtmelSAMD21::DESCRIPTOR::STEP_SIZE::X1;
        transferDesc.num_beats_per_block = sizeof(dataSrc);
        transferDesc.source_address      = reinterpret_cast<uint32_t>(dataSrc) + NUM_BYTES;
        transferDesc.destination_address = reinterpret_cast<uint32_t>(dataDst) + NUM_BYTES;
        transferDesc.enable_interrupt_transfer_error    = false;
        transferDesc.enable_interrupt_transfer_complete = false;
        transferDesc.enable_interrupt_channel_suspend   = false;
    }

    m_dma_engine->StartTransfer(transferDesc);
	
	DMAC->SWTRIGCTRL.reg |= 1 << 0;
	
	while(DMAC->BUSYCH.reg != 0) {
		uint8_t temp = 2;
	} 
}
