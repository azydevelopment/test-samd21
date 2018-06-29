#include "program.h"

#include <azydev/embedded/clock/atmel/samd21/clock.h>
#include <azydev/embedded/dma/atmel/samd21/engine.h>
#include <azydev/embedded/dma/atmel/samd21/transfer.h>
#include <azydev/embedded/dma/common/node.h>
#include <azydev/embedded/dma/common/pool.h>
#include <cstring>

/* FILE SCOPED STATICS */

static const uint32_t NUM_BYTES  = 2;
static const uint8_t NUM_STEPS   = 2;
static const uint8_t TRANSFER_ID = 0xAA;

static void OnTransferEnded(const uint8_t transferId, const IDMAEntity::RESULT result) {
    uint8_t temp = transferId;
}

/* PUBLIC */

CProgram::CProgram()
    : m_dma_clock(nullptr)
    , m_dma_engine(nullptr)
    , m_dma_pool(nullptr)
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
        desc.num_steps_max                          = NUM_STEPS;

        // create DMA transfer
        m_dma_transfer = new CDMATransferAtmelSAMD21<uint8_t>(desc);
    }
}

void CProgram::OnUpdate() {
    // create DMA memory pool
    if (m_dma_pool == nullptr) {
        CDMAPool<uint8_t>::DESC descPool = {};
        descPool.num_allocations_max     = NUM_STEPS * 2;
        descPool.num_beats_max           = NUM_BYTES * NUM_STEPS * 2;

        m_dma_pool = new CDMAPool<uint8_t>(descPool);
    }

    // reset DMA pool
    for (uint8_t i = 0; i < NUM_STEPS * 2; i++) {
        m_dma_pool->PopAllocation();
    }

    // prepare transfer nodes
    const IDMANode<uint8_t>* nodesSrc[NUM_STEPS] = {};
    const IDMANode<uint8_t>* nodesDst[NUM_STEPS] = {};
    {
        // for each step
        for (uint8_t i = 0; i < NUM_STEPS; i++) {
            // temp variable used to acquire the DMA nodes after pushing allocations
            uint8_t allocationId = 0;

            // prepare source node
            m_dma_pool->PushAllocation(allocationId);
            for (uint32_t j = 0; j < NUM_BYTES; j++) {
                m_dma_pool->RecordWrite(rand());
            }
            nodesSrc[i] = m_dma_pool->GetAllocationDMANode(allocationId);

            // prepare destination node
            m_dma_pool->PushAllocation(allocationId);
            m_dma_pool->RecordRead(NUM_BYTES);
            nodesDst[i] = m_dma_pool->GetAllocationDMANode(allocationId);
        }
    }

    // reset DMA transfer
    m_dma_transfer->Reset(TRANSFER_ID);

    // add transfer step
    {
        for (uint8_t i = 0; i < NUM_STEPS; i++) {
            CDMATransferAtmelSAMD21<uint8_t>::STEP_DESC step = {};
            step.num_beats                                   = NUM_BYTES;
            step.node_source                                 = nodesSrc[i];
            step.node_destination                            = nodesDst[i];

            step.event_output_selection =
                CDMATransferAtmelSAMD21<uint8_t>::DESCRIPTOR::EVENT_OUTPUT_SELECTION::DISABLED;
            step.block_completed_action = CDMATransferAtmelSAMD21<
                uint8_t>::DESCRIPTOR::BLOCK_COMPLETED_ACTION::DISABLE_IF_LAST;
            step.beat_size = CDMATransferAtmelSAMD21<uint8_t>::DESCRIPTOR::BEAT_SIZE::BITS_8;
            step.step_size_select =
                CDMATransferAtmelSAMD21<uint8_t>::DESCRIPTOR::STEP_SIZE_SELECT::DESTINATION;
            step.step_size = CDMATransferAtmelSAMD21<uint8_t>::DESCRIPTOR::STEP_SIZE::X1;

            m_dma_transfer->AddStep(step);
        }
    }

    // create transfer config
    CDMATransferAtmelSAMD21<uint8_t>::CONFIG_DESC transferConfig = {};
    {
        transferConfig.callback_on_transfer_ended = &OnTransferEnded;
        transferConfig.priority = CDMATransferAtmelSAMD21<uint8_t>::PRIORITY::LVL_0;
        transferConfig.trigger  = CDMATransferAtmelSAMD21<uint8_t>::TRIGGER::SOFTWARE_OR_EVENT;
        transferConfig.trigger_action =
            CDMATransferAtmelSAMD21<uint8_t>::TRIGGER_ACTION::START_TRANSACTION;
        transferConfig.enable_event_output = false;
        transferConfig.enable_event_input  = false;
        transferConfig.event_input_action =
            CDMATransferAtmelSAMD21<uint8_t>::EVENT_INPUT_ACTION::NOACT;
    }

    // execute the transfer
    {
        CDMATransfer<uint8_t>::ITransferControl* transferControl = nullptr;
        m_dma_engine->StartTransfer(*m_dma_transfer, transferConfig, &transferControl);

        while (transferControl->IsTransferInProgress()) {
            if (transferControl->IsPendingTrigger()) {
                transferControl->TriggerTransferStep();
            }
        }
    }
}