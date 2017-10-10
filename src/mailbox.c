#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "mailbox.h"

enum {
    PERIPH_BASE = 0x3F000000, // 0x20000000 pour rpi1
    
    MBOX_READ   = (PERIPH_BASE + 0xB880),
    MBOX_POLL   = (PERIPH_BASE + 0xB890),
    MBOX_SENDER = (PERIPH_BASE + 0xB894),
    MBOX_STATUS = (PERIPH_BASE + 0xB898),
    MBOX_CONF   = (PERIPH_BASE + 0xB89C),
    MBOX_WRITE  = (PERIPH_BASE + 0xB8A0)
};

/* Make sure the property tag buffer is aligned to a 16-byte boundary because
   we only have 28-bits available in the property interface protocol to pass
   the address of the buffer to the VC. */
static uint32_t pt[8192] __attribute__ ((aligned(16)));
static uint32_t pt_index = 0;

static inline void mmio_write(uint32_t reg, uint32_t data) {
	*(volatile uint32_t *)reg = data;
}
 
static inline uint32_t mmio_read(uint32_t reg) {
	return *(volatile uint32_t *)reg;
}

void mailbox_write(mailbox0_channel_t channel, uint32_t value) {
    // Waits until the Status field has a 0 in the top bit
    while (mmio_read(MBOX_STATUS) & (1 << 31)) {}
    mmio_write(MBOX_WRITE, value | channel);
}

uint32_t mailbox_read(mailbox0_channel_t channel) {
    uint32_t val;
    do {
        // Waits until the status field has a 0 in the 30th bit
        while (mmio_read(MBOX_STATUS) & (1 << 30)) {}
        val = mmio_read(MBOX_READ);
    } while ((val & 0xF)!=channel);
    return val & 0xFFFFFFF0;
}

void mailbox_property_init(void) {
     /* Fill in the size on-the-fly */
    pt[PT_OSIZE] = 12;

    /* Process request (All other values are reserved!) */
    pt[PT_OREQUEST_OR_RESPONSE] = 0;

    /* First available data slot */
    pt_index = 2;

    /* NULL tag to terminate tag list */
    pt[pt_index] = 0;
}

void mailbox_property_add_tag(mailbox_tag_t tag, ...) {
    va_list vl;
    va_start(vl, tag);

    pt[pt_index++] = tag;

    switch (tag) {
        case TAG_GET_FIRMWARE_VERSION:
        case TAG_GET_BOARD_MODEL:
        case TAG_GET_BOARD_REVISION:
        case TAG_GET_BOARD_MAC_ADDRESS:
        case TAG_GET_BOARD_SERIAL:
        case TAG_GET_ARM_MEMORY:
        case TAG_GET_VC_MEMORY:
        case TAG_GET_DMA_CHANNELS:
            /* Provide an 8-byte buffer for the response */
            pt[pt_index++] = 8;
            pt[pt_index++] = 0; /* Request */
            pt_index += 2;
            break;
        case TAG_GET_CLOCKS:
        case TAG_GET_COMMAND_LINE:
            /* Provide a 256-byte buffer */
            pt[pt_index++] = 256;
            pt[pt_index++] = 0; /* Request */
            pt_index += 256 >> 2;
            break;
        case TAG_ALLOCATE_BUFFER:
        case TAG_GET_MAX_CLOCK_RATE:
        case TAG_GET_MIN_CLOCK_RATE:
        case TAG_GET_CLOCK_RATE:
            pt[pt_index++] = 8;
            pt[pt_index++] = 0; /* Request */
            pt[pt_index++] = va_arg(vl, int);
            pt[pt_index++] = 0;
            break;
        case TAG_SET_CLOCK_RATE:
            pt[pt_index++] = 12;
            pt[pt_index++] = 0; /* Request */
            pt[pt_index++] = va_arg(vl, int); /* Clock ID */
            pt[pt_index++] = va_arg(vl, int); /* Rate (in Hz) */
            pt[pt_index++] = va_arg(vl, int); /* Skip turbo setting if == 1 */
            break;
        case TAG_ALLOCATE_MEMORY:
            pt[pt_index++] = 12;
            pt[pt_index++] = 0; /* Request */
            pt[pt_index++] = va_arg(vl, int); /* Size (bytes) */
            pt[pt_index++] = va_arg(vl, int); /* Alignement (bytes) */
            pt[pt_index++] = va_arg(vl, int); /* Flags */
            break;
        case TAG_LOCK_MEMORY:
        case TAG_UNLOCK_MEMORY:
        case TAG_RELEASE_MEMORY:
            pt[pt_index++] = 4;
            pt[pt_index++] = 0; /* Request */
            pt[pt_index++] = va_arg(vl, int); /* Handle */
            break;
        case TAG_GET_PHYSICAL_SIZE:
        case TAG_SET_PHYSICAL_SIZE:
        case TAG_TEST_PHYSICAL_SIZE:
        case TAG_GET_VIRTUAL_SIZE:
        case TAG_SET_VIRTUAL_SIZE:
        case TAG_TEST_VIRTUAL_SIZE:
        case TAG_GET_VIRTUAL_OFFSET:
        case TAG_SET_VIRTUAL_OFFSET:
            pt[pt_index++] = 8;
            pt[pt_index++] = 0; /* Request */

            if (( tag == TAG_SET_PHYSICAL_SIZE ) ||
                ( tag == TAG_SET_VIRTUAL_SIZE ) ||
                ( tag == TAG_SET_VIRTUAL_OFFSET ) ||
                ( tag == TAG_TEST_PHYSICAL_SIZE ) ||
                ( tag == TAG_TEST_VIRTUAL_SIZE ) )
            {
                pt[pt_index++] = va_arg( vl, int ); /* Width */
                pt[pt_index++] = va_arg( vl, int ); /* Height */
            } else {
                pt_index += 2;
            }
            break;

        case TAG_GET_ALPHA_MODE:
        case TAG_SET_ALPHA_MODE:
        case TAG_GET_DEPTH:
        case TAG_SET_DEPTH:
        case TAG_GET_PIXEL_ORDER:
        case TAG_SET_PIXEL_ORDER:
        case TAG_GET_PITCH:
            pt[pt_index++] = 4;
            pt[pt_index++] = 0; /* Request */

            if (( tag == TAG_SET_DEPTH ) ||
                ( tag == TAG_SET_PIXEL_ORDER ) ||
                ( tag == TAG_SET_ALPHA_MODE ) )
            {
                /* Colour Depth, bits-per-pixel \ Pixel Order State */
                pt[pt_index++] = va_arg( vl, int );
            } else {
                pt_index += 1;
            }
            break;

        case TAG_GET_OVERSCAN:
        case TAG_SET_OVERSCAN:
            pt[pt_index++] = 16;
            pt[pt_index++] = 0; /* Request */

            if (tag == TAG_SET_OVERSCAN) {
                pt[pt_index++] = va_arg(vl, int); /* Top pixels */
                pt[pt_index++] = va_arg(vl, int); /* Bottom pixels */
                pt[pt_index++] = va_arg(vl, int); /* Left pixels */
                pt[pt_index++] = va_arg(vl, int); /* Right pixels */
            } else {
                pt_index += 4;
            }
            break;

        default:
            /* Unsupported tags, just remove the tag from the list */
            pt_index--;
            break;
    }

    /* Make sure the tags are 0 terminated to end the list and update the buffer size */
    pt[pt_index] = 0;

    va_end( vl );
}

uint32_t mailbox_property_process(void) {
    uint32_t result;

    /* Fill in the size of the buffer */
    pt[PT_OSIZE] = (pt_index + 1) << 2;
    pt[PT_OREQUEST_OR_RESPONSE] = 0;

    mailbox_write(MBOX0_TAGS_ARM_TO_VC, (uint32_t) pt);

    result = mailbox_read(MBOX0_TAGS_ARM_TO_VC);
    
    return result;
}

mailbox_property_t* mailbox_get_property(mailbox_tag_t tag) {
    static mailbox_property_t property;
    uint32_t* tag_buffer = NULL;

    property.tag = tag;

    /* Get the tag from the buffer. Start at the first tag position  */
    uint32_t index = 2;

    while (index < (pt[PT_OSIZE] >> 2)) {
        /* printf( "Test Tag: [%d] %8.8X\r\n", index, pt[index] ); */
        if (pt[index] == (uint32_t)tag) {
            tag_buffer = &pt[index];
            break;
        }

        /* Progress to the next tag if we haven't yet discovered the tag */
        index += (pt[index + 1] >> 2) + 3;
    }

    /* Return NULL of the property tag cannot be found in the buffer */
    if (tag_buffer == NULL)
        return NULL;

    /* Return the required data */
    property.byte_length = tag_buffer[T_ORESPONSE] & 0xFFFF;
    memcpy(property.data.buffer_8, &tag_buffer[T_OVALUE], property.byte_length);
    
    return &property;
}

/* Memory management */

// Allocates contiguous memory on the GPU. size and alignment are in bytes. 
uint32_t mailbox_malloc(uint32_t size, uint32_t alignement, uint32_t flags) {
    mailbox_property_init();
    mailbox_property_add_tag(TAG_ALLOCATE_MEMORY, size, alignement, flags);
    mailbox_property_process();
    
    mailbox_property_t* mp = mailbox_get_property(TAG_ALLOCATE_MEMORY);
    
    return mp->data.value_32;
}

// Lock buffer in place, and return a bus address. Must be done before memory can be accessed
void* mailbox_mem_lock(uint32_t handle) {
    mailbox_property_init();
    mailbox_property_add_tag(TAG_LOCK_MEMORY, handle);
    mailbox_property_process();
    
    mailbox_property_t* mp = mailbox_get_property(TAG_LOCK_MEMORY);
    
    return (void*) mp->data.value_32;
}

// Unlock buffer. It retains contents, but may move. Needs to be locked before next use. status=0 is success.
uint32_t mailbox_mem_unlock(uint32_t handle) {
    mailbox_property_init();
    mailbox_property_add_tag(TAG_UNLOCK_MEMORY, handle);
    mailbox_property_process();
    
    mailbox_property_t* mp = mailbox_get_property(TAG_UNLOCK_MEMORY);
    
    return mp->data.value_32;
}

// Free the memory buffer. status=0 is success.
uint32_t mailbox_free(uint32_t handle) {
    mailbox_property_init();
    mailbox_property_add_tag(TAG_RELEASE_MEMORY, handle);
    mailbox_property_process();
    
    mailbox_property_t* mp = mailbox_get_property(TAG_RELEASE_MEMORY);
    
    return mp->data.value_32;
}