#include "gpu.h"

#include <stdio.h>

#define ADDR_OFFSET  0xC0000000 // 0x40000000 for rpi1

static FramebufferInfos framebuffer;

uint16_t* gpu_init(uint32_t width, uint32_t height, uint32_t bpp) {
    framebuffer.physical_width = width;
    framebuffer.physical_height = height;
    framebuffer.virtual_width = width;
    framebuffer.virtual_height = height;
    framebuffer.pitch = 0;
    framebuffer.bit_depth = bpp;
    framebuffer.X = 0;
    framebuffer.Y = 0;
    framebuffer.pointer = 0;
    framebuffer.size = 0;
    
    mailbox_write(MBOX0_FRAMEBUFFER, ADDR_OFFSET + (uint32_t)(&framebuffer));
    
    uint32_t result = mailbox_read(MBOX0_FRAMEBUFFER);
    if (result != 0) {
        return 0;
    }
    
    return (uint16_t*)(framebuffer.pointer-ADDR_OFFSET);
}
