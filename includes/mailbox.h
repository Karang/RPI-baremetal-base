#ifndef __MAILBOX_H__
#define __MAILBOX_H__

#include <stdint.h>

typedef enum {
    MBOX0_POWER_MANAGEMENT = 0,
    MBOX0_FRAMEBUFFER,
    MBOX0_VIRTUAL_UART,
    MBOX0_VCHIQ,
    MBOX0_LEDS,
    MBOX0_BUTTONS,
    MBOX0_TOUCHSCREEN,
    MBOX0_UNUSED,
    MBOX0_TAGS_ARM_TO_VC,
    MBOX0_TAGS_VC_TO_ARM,
} mailbox0_channel_t;

typedef enum {
    /* Videocore */
    TAG_GET_FIRMWARE_VERSION = 0x1,

    /* Hardware */
    TAG_GET_BOARD_MODEL = 0x10001,
    TAG_GET_BOARD_REVISION,
    TAG_GET_BOARD_MAC_ADDRESS,
    TAG_GET_BOARD_SERIAL,
    TAG_GET_ARM_MEMORY,
    TAG_GET_VC_MEMORY,
    TAG_GET_CLOCKS,

    /* Config */
    TAG_GET_COMMAND_LINE = 0x50001,

    /* Shared resource management */
    TAG_GET_DMA_CHANNELS = 0x60001,

    /* Power */
    TAG_GET_POWER_STATE = 0x20001,
    TAG_GET_TIMING,
    TAG_SET_POWER_STATE = 0x28001,

    /* Clocks */
    TAG_GET_CLOCK_STATE = 0x30001,
    TAG_SET_CLOCK_STATE = 0x38001,
    TAG_GET_CLOCK_RATE = 0x30002,
    TAG_SET_CLOCK_RATE = 0x38002,
    TAG_GET_MAX_CLOCK_RATE = 0x30004,
    TAG_GET_MIN_CLOCK_RATE = 0x30007,
    TAG_GET_TURBO = 0x30009,
    TAG_SET_TURBO = 0x38009,

    /* Voltage */
    TAG_GET_VOLTAGE = 0x30003,
    TAG_SET_VOLTAGE = 0x38003,
    TAG_GET_MAX_VOLTAGE = 0x30005,
    TAG_GET_MIN_VOLTAGE = 0x30008,
    
    /* Temperature */
    TAG_GET_TEMPERATURE = 0x30006,
    TAG_GET_MAX_TEMPERATURE = 0x3000A,
    
    /* Memory */
    TAG_ALLOCATE_MEMORY = 0x3000C,
    TAG_LOCK_MEMORY = 0x3000D,
    TAG_UNLOCK_MEMORY = 0x3000E,
    TAG_RELEASE_MEMORY = 0x3000F,
    
    TAG_EXECUTE_CODE = 0x30010,
    TAG_GET_DISPMANX_MEM_HANDLE = 0x30014,
    TAG_GET_EDID_BLOCK = 0x30020,

    /* Framebuffer */
    TAG_ALLOCATE_BUFFER = 0x40001,
    TAG_RELEASE_BUFFER = 0x48001,
    TAG_BLANK_SCREEN = 0x40002,
    TAG_GET_PHYSICAL_SIZE = 0x40003,
    TAG_TEST_PHYSICAL_SIZE = 0x44003,
    TAG_SET_PHYSICAL_SIZE = 0x48003,
    TAG_GET_VIRTUAL_SIZE = 0x40004,
    TAG_TEST_VIRTUAL_SIZE = 0x44004,
    TAG_SET_VIRTUAL_SIZE = 0x48004,
    TAG_GET_DEPTH = 0x40005,
    TAG_TEST_DEPTH = 0x44005,
    TAG_SET_DEPTH = 0x48005,
    TAG_GET_PIXEL_ORDER = 0x40006,
    TAG_TEST_PIXEL_ORDER = 0x44006,
    TAG_SET_PIXEL_ORDER = 0x48006,
    TAG_GET_ALPHA_MODE = 0x40007,
    TAG_TEST_ALPHA_MODE = 0x44007,
    TAG_SET_ALPHA_MODE = 0x48007,
    TAG_GET_PITCH = 0x40008,
    TAG_GET_VIRTUAL_OFFSET = 0x40009,
    TAG_TEST_VIRTUAL_OFFSET = 0x44009,
    TAG_SET_VIRTUAL_OFFSET = 0x48009,
    TAG_GET_OVERSCAN = 0x4000A,
    TAG_TEST_OVERSCAN = 0x4400A,
    TAG_SET_OVERSCAN = 0x4800A,
    TAG_GET_PALETTE = 0x4000B,
    TAG_TEST_PALETTE = 0x4400B,
    TAG_SET_PALETTE = 0x4800B,
    TAG_SET_CURSOR_INFO = 0x8011,
    TAG_SET_CURSOR_STATE = 0x8010
} mailbox_tag_t;

typedef enum {
    TAG_STATE_REQUEST = 0,
    TAG_STATE_RESPONSE = 1,
} tag_state_t;

typedef enum {
    PT_OSIZE = 0,
    PT_OREQUEST_OR_RESPONSE = 1,
} tag_buffer_offset_t;

typedef enum {
    T_OIDENT = 0,
    T_OVALUE_SIZE = 1,
    T_ORESPONSE = 2,
    T_OVALUE = 3,
} tag_offset_t;

typedef struct {
    uint32_t tag;
    uint32_t byte_length;
    union {
        uint32_t value_32;
        uint8_t buffer_8[256];
        uint32_t buffer_32[64];
    } data;
} mailbox_property_t;

typedef enum {
    TAG_CLOCK_RESERVED = 0,
    TAG_CLOCK_EMMC,
    TAG_CLOCK_UART,
    TAG_CLOCK_ARM,
    TAG_CLOCK_CORE,
    TAG_CLOCK_V3D,
    TAG_CLOCK_H264,
    TAG_CLOCK_ISP,
    TAG_CLOCK_SDRAM,
    TAG_CLOCK_PIXEL,
    TAG_CLOCK_PWM,
} tag_clock_id_t;

void mailbox_write(mailbox0_channel_t channel, uint32_t value);
uint32_t mailbox_read(mailbox0_channel_t channel);

void mailbox_property_init(void);
void mailbox_property_add_tag(mailbox_tag_t tag, ...);
uint32_t mailbox_property_process(void);
mailbox_property_t* mailbox_get_property(mailbox_tag_t tag);

/* Memory management */

#define MEM_FLAG_DISCARDABLE        1        /* can be resized to 0 at any time. Use for cached data */
#define MEM_FLAG_NORMAL             (0 << 2) /* normal allocating alias. Don't use from ARM */
#define MEM_FLAG_DIRECT             (1 << 2) /* 0xC alias uncached */
#define MEM_FLAG_COHERENT           (2 << 2) /* 0x8 alias. Non-allocating in L2 but coherent */
#define MEM_FLAG_L1_NONALLOCATING   (MEM_FLAG_DIRECT | MEM_FLAG_COHERENT) /* Allocating in L2 */
#define MEM_FLAG_ZERO               (1 << 4) /* initialise buffer to all zeros */
#define MEM_FLAG_NO_INIT            (1 << 5) /* don't initialise (default is initialise to all ones */
#define MEM_FLAG_HINT_PERMALOCK     (1 << 6) /* Likely to be locked for long periods of time. */

uint32_t mailbox_malloc(uint32_t size, uint32_t alignement, uint32_t flags);
void* mailbox_mem_lock(uint32_t handle);
uint32_t mailbox_mem_unlock(uint32_t handle);
uint32_t mailbox_free(uint32_t handle);

#endif