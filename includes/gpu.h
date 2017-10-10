#ifndef __GPU_H__
#define __GPU_H__

#include <stdint.h>
#include <mailbox.h>

typedef struct _framebuffer_infos {
    uint32_t physical_width;
    uint32_t physical_height;
    uint32_t virtual_width;
    uint32_t virtual_height;
    uint32_t pitch; // GPU
    uint32_t bit_depth;
    uint32_t X;
    uint32_t Y;
    uint32_t pointer; // GPU
    uint32_t size; // GPU
} FramebufferInfos __attribute__ ((aligned(16)));

#define RGB16(r,g,b) (((r)<<11) | ((g)<<5) | (b))

uint16_t* gpu_init(uint32_t width, uint32_t height, uint32_t bpp);

#endif