#include "program.h"

#include <azydev/embedded/clock/atmel/samd21/clock.h>
#include <azydev/embedded/dma/atmel/samd21/engine.h>
#include <azydev/embedded/dma/common/node.h>
#include <azydev/embedded/dma/common/node_address.h>
#include <azydev/embedded/dma/common/node_packet.h>
#include <cstring>

/* FILE SCOPED STATICS */

static const uint16_t NUM_BYTES = 8;

static void OnTransferComplete(const uint8_t transferId) {
    uint8_t temp = transferId;
}

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

void CProgram::OnUpdate() {
	// setup source node
	IDMANodePacket::DESC descSrc = {};
	descSrc.data_type = IDMANode::BEAT_PRIMITIVE::UINT8_T;
	descSrc.is_incrementing = true;
	descSrc.num_beats_max = NUM_BYTES;
	
	IDMANodePacket nodeSrc(descSrc);
	
	// populate source data
	for (uint8_t i = 0; i < NUM_BYTES; i++) {
		nodeSrc.Write(i);
	}
	
	// setup destination node
	IDMANodePacket::DESC descDst = {};
	descDst.data_type = IDMANode::BEAT_PRIMITIVE::UINT8_T;
	descDst.is_incrementing = true;
	descDst.num_beats_max = NUM_BYTES;

	IDMANodePacket nodeDst(descDst);
	
    // init DMA transfer
    CDMAChannelAtmelSAMD21::TRANSFER_DESC transferDesc = {};
    {
        transferDesc.id                         = 0;
		transferDesc.num_beats = NUM_BYTES;
		transferDesc.node_source = &nodeSrc;
		transferDesc.node_destination = &nodeDst;
        transferDesc.callback_transfer_complete = &OnTransferComplete;
		
        transferDesc.priority                   = CDMAChannelAtmelSAMD21::PRIORITY::LVL_0;
        transferDesc.trigger        = CDMAChannelAtmelSAMD21::TRIGGER::SOFTWARE_OR_EVENT;
        transferDesc.trigger_action = CDMAChannelAtmelSAMD21::TRIGGER_ACTION::START_TRANSACTION;
        transferDesc.enable_event_output = false;
        transferDesc.enable_event_input  = false;
        transferDesc.event_input_action  = CDMAChannelAtmelSAMD21::EVENT_INPUT_ACTION::NOACT;
        transferDesc.event_output_selection =
            CDMAChannelAtmelSAMD21::DESCRIPTOR::EVENT_OUTPUT_SELECTION::DISABLED;
        transferDesc.block_completed_action =
            CDMAChannelAtmelSAMD21::DESCRIPTOR::BLOCK_COMPLETED_ACTION::DISABLE_IF_LAST;
        transferDesc.beat_size = CDMAChannelAtmelSAMD21::DESCRIPTOR::BEAT_SIZE::BITS_8;
        transferDesc.step_size_select =
            CDMAChannelAtmelSAMD21::DESCRIPTOR::STEP_SIZE_SELECT::DESTINATION;
        transferDesc.step_size           = CDMAChannelAtmelSAMD21::DESCRIPTOR::STEP_SIZE::X1;
        transferDesc.enable_interrupt_transfer_error  = true;
        transferDesc.enable_interrupt_channel_suspend = true;
    }

    IDMAEntity::ITransferControl* transferControl = nullptr;
    m_dma_engine->StartTransfer(transferDesc, &transferControl);

    while (transferControl->IsTransferInProgress()) {
        if (transferControl->IsPendingTrigger()) {
            transferControl->TriggerTransferStep();
        }
    }
}
