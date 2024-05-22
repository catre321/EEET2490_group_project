#include "dma.h"
#include "mem.h"
#include "mm.h"
#include "../uart/uart0.h"

#include "../kernel/framebf.h"

dma_channel channels[15];

static unsigned int channel_map = 0x1F35;


static unsigned int allocate_channel(unsigned int channel) {
    if (!(channel & ~0x0F)) {
        if (channel_map & (1 << channel)) {
            channel_map &= ~(1 << channel);
            return channel;
        }

        return -1;
    }

    unsigned int i = channel == CT_NORMAL ? 6 : 12;

    for (; i >= 0; i--) {
        if (channel_map & (1 << i)) {
            channel_map &= ~(1 << i);
            return i;
        }
    }

    return CT_NONE;
}

dma_channel *dma_open_channel(unsigned int channel) {
    unsigned int _channel = allocate_channel(channel);

    if (_channel == CT_NONE) {
        uart0_puts("INVALID CHANNEL!");
        return 0;
    }

    dma_channel *dma = (dma_channel *)&channels[_channel];
    dma->channel = _channel;

    //LOW_MEMORY = bottom of RAM...  Hack for now since no allocate function
    // dma->block = (dma_control_block *)((LOW_MEMORY + 31) & ~31);
    static dma_control_block __attribute__((aligned(32))) dma_block;
    dma->block = &dma_block;
    dma->block->res[0] = 0;
    dma->block->res[1] = 0;

    REGS_DMA_ENABLE |= (1 << dma->channel);
    // timer_sleep(3);
    // uart0_puts("Run to this!!!!!");
    wait_msec(300);
    REGS_DMA(dma->channel)->control |= CS_RESET;
    
    while(REGS_DMA(dma->channel)->control & CS_RESET) ;
    uart0_puts("DMA open successfully!!!");

    return dma;
}

void dma_close_channel(dma_channel *channel) {
    channel_map |= (1 << channel->channel);
}

void dma_setup_mem_copy(dma_channel *channel, void *dest, const void *src, unsigned int length, unsigned int burst_length) {
    channel->block->transfer_info = (burst_length << TI_BURST_LENGTH_SHIFT)
						    | TI_SRC_WIDTH
						    | TI_SRC_INC
						    | TI_DEST_WIDTH
						    | TI_DEST_INC;

    channel->block->src_addr = (unsigned long)src;
    channel->block->dest_addr = (unsigned long)dest;
    channel->block->transfer_length = length;
    channel->block->mode_2d_stride = 0;
    channel->block->next_block_addr = 0;
   
}

void dma_start(dma_channel *channel) {
    // uart0_puts("The DMA CBlock: ");
    // uart0_hex(channel->block->transfer_info);
    
    REGS_DMA(channel->channel)->control_block_addr = BUS_ADDRESS((unsigned long)channel->block);

    REGS_DMA(channel->channel)->control = CS_WAIT_FOR_OUTSTANDING_WRITES
					      | (DEFAULT_PANIC_PRIORITY << CS_PANIC_PRIORITY_SHIFT)
					      | (DEFAULT_PRIORITY << CS_PRIORITY_SHIFT)
					      | CS_ACTIVE;
    // uart0_puts("\n The DMA start: ");
    // uart0_dec(REGS_DMA(channel->channel)->control & CS_ACTIVE);
}

int dma_wait(dma_channel *channel) {
    while(REGS_DMA(channel->channel)->control & CS_ACTIVE) ;

    channel->status = REGS_DMA(channel->channel)->control & CS_ERROR ? 0 : 1;

    return channel->status;

}

// Test DMA functionality
int compare_memory(const unsigned int *a, const unsigned int *b, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        uart0_dec(a[i]);
        uart0_puts("-");
        uart0_dec(b[i]);
        uart0_puts(" ");
        if (a[i] != b[i]) {
            
            return 0; // Memory differs
        }
    }
    return 1; // Memory is the same
}
const unsigned int src_data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    unsigned int dest_data[10]; // Initialize destination array with zeros
// Test DMA functionality
void test_dma() {
    uart0_puts("Testing DMA...\n");

    // Test data
    // const unsigned int src_data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    // unsigned int dest_data[10]; // Initialize destination array with zeros

    // Perform DMA operation (assuming size is in bytes)
    do_dma(dest_data, src_data, sizeof(src_data));
    uart0_puts("AFter do_dma\n");
    // Verify the copy
    if (compare_memory(src_data, dest_data, sizeof(src_data)/sizeof(unsigned int))) {
        uart0_puts("\nDMA Test PassedVVVV.\n");
    } else {
        uart0_puts("\nDMA Test Failed.\n");
    }
}





