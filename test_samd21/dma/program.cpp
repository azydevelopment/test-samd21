#include "program.h"

#include <azydev/embedded/clock/atmel/samd21/clock.h>
#include <azydev/embedded/dma/atmel/samd21/engine.h>
#include <azydev/embedded/dma/atmel/samd21/transfer.h>
#include <azydev/embedded/dma/common/node.h>
#include <azydev/embedded/dma/common/pool.h>
#include <cstring>

/* FILE SCOPED STATICS */

static const uint16_t NUM_BYTES = 8;

static void OnTransferEnded(const uint8_t transferId, const IDMAEntity::RESULT result) {
    uint8_t temp = transferId;
}

/* PUBLIC */

CProgram::CProgram()
    : m_dma_clock(nullptr)
    , m_dma_engine(nullptr)
    , m_dma_transfer(nullptr) {
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

    // init DMA transfer object
    {
        CDMATransferAtmelSAMD21<uint8_t>::DESC desc = {};
        desc.id_initial                             = 0;
        desc.num_steps_max                          = 1;

        // create DMA transfer
        m_dma_transfer = new CDMATransferAtmelSAMD21<uint8_t>(desc);
    }
}

CDMAPool<uint8_t>* dmaPool = nullptr;

void CProgram::OnUpdate() {
    if (dmaPool == nullptr) {
        // create DMA memory pool
        CDMAPool<uint8_t>::DESC descPool = {};
        descPool.num_allocations_max     = 4;
        descPool.num_beats_max           = 8;

        dmaPool = new CDMAPool<uint8_t>(descPool);
    }

    IDMANode<uint8_t>* node = nullptr;

    dmaPool->PushAllocation(&node);
    dmaPool->RecordWrite(rand());
    dmaPool->RecordWrite(rand());
    dmaPool->RecordWrite(rand());

    dmaPool->PushAllocation(&node);
    dmaPool->RecordWrite(rand());
    dmaPool->RecordWrite(rand());
    dmaPool->RecordWrite(rand());

    dmaPool->PopAllocation();

    dmaPool->PushAllocation(&node);
    dmaPool->RecordRead(3);
    // dmaPool->RecordWrite(rand());
    // dmaPool->RecordWrite(rand());
    // dmaPool->RecordWrite(rand());

    //// reset transfer
    // m_dma_transfer->Reset(111);
    //
    //// setup source node
    // IDMANodePacket<uint8_t>::DESC descSrc = {};
    // descSrc.is_incrementing               = true;
    // descSrc.num_beats_max                 = NUM_BYTES;
    //
    // IDMANodePacket<uint8_t> nodeSrc(descSrc);
    //
    //// populate source data
    // for (uint8_t i = 0; i < NUM_BYTES; i++) {
    // nodeSrc.RecordWrite(i + 1);
    //}
    //
    //// setup destination node
    // IDMANodePacket<uint8_t>::DESC descDst = {};
    // descDst.is_incrementing               = true;
    // descDst.num_beats_max                 = NUM_BYTES;
    //
    // IDMANodePacket<uint8_t> nodeDst(descDst);
    //
    //// track that this packet will receive data
    // nodeDst.RecordRead(NUM_BYTES);
    //
    //// add transfer step
    //{
    // CDMATransferAtmelSAMD21<uint8_t>::STEP_DESC step = {};
    // step.num_beats                                   = NUM_BYTES;
    // step.node_source                                 = &nodeSrc;
    // step.node_destination                            = &nodeDst;
    //
    // step.event_output_selection =
    // CDMATransferAtmelSAMD21<uint8_t>::DESCRIPTOR::EVENT_OUTPUT_SELECTION::DISABLED;
    // step.block_completed_action =
    // CDMATransferAtmelSAMD21<uint8_t>::DESCRIPTOR::BLOCK_COMPLETED_ACTION::DISABLE_IF_LAST;
    // step.beat_size = CDMATransferAtmelSAMD21<uint8_t>::DESCRIPTOR::BEAT_SIZE::BITS_8;
    // step.step_size_select =
    // CDMATransferAtmelSAMD21<uint8_t>::DESCRIPTOR::STEP_SIZE_SELECT::DESTINATION;
    // step.step_size = CDMATransferAtmelSAMD21<uint8_t>::DESCRIPTOR::STEP_SIZE::X1;
    //
    // m_dma_transfer->AddStep(step);
    //}
    //
    //// create transfer config
    // CDMATransferAtmelSAMD21<uint8_t>::CONFIG_DESC transferConfig = {};
    //{
    // transferConfig.callback_on_transfer_ended = &OnTransferEnded;
    // transferConfig.priority = CDMATransferAtmelSAMD21<uint8_t>::PRIORITY::LVL_0;
    // transferConfig.trigger  = CDMATransferAtmelSAMD21<uint8_t>::TRIGGER::SOFTWARE_OR_EVENT;
    // transferConfig.trigger_action =
    // CDMATransferAtmelSAMD21<uint8_t>::TRIGGER_ACTION::START_TRANSACTION;
    // transferConfig.enable_event_output = false;
    // transferConfig.enable_event_input  = false;
    // transferConfig.event_input_action =
    // CDMATransferAtmelSAMD21<uint8_t>::EVENT_INPUT_ACTION::NOACT;
    //}
    //
    //// execute the transfer
    //{
    // CDMATransfer<uint8_t>::ITransferControl* transferControl = nullptr;
    // m_dma_engine->StartTransfer(*m_dma_transfer, transferConfig, &transferControl);
    //
    // while (transferControl->IsTransferInProgress()) {
    // if (transferControl->IsPendingTrigger()) {
    // transferControl->TriggerTransferStep();
    //}
    //}
    //}
}
