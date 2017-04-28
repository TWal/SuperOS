#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../src/utility.h"

/**
   @brief This struct containt the graphical paramters parsed by the loader.
*/
struct GraphicalParam{
    u32 physptr;///< Pointer to the physical buffer for graphics
    u16 Xsize,Ysize;
    u32 pitch; ///< Bytes per line
};

/// Genric VBE Information
struct VbeInfoBlock {
    char VbeSignature[4];             // == "VESA"
    uint16_t VbeVersion;                 // == 0x0300 for VBE 3.0
    uint16_t OemStringPtr[2];            // isa vbeFarPtr
    uint8_t Capabilities[4];
    uint16_t VideoModePtr[2];         // isa vbeFarPtr
    uint16_t TotalMemory;             // as # of 64KB blocks
} __attribute__((packed));


struct ModeInfoBlock {
    uint16_t attributes;
    uint8_t winA,winB;
    uint16_t granularity;
    uint16_t winsize;
    uint16_t segmentA, segmentB;
    u32 fctptr;
    uint16_t pitch; // bytes per scanline

    uint16_t Xres, Yres;
    uint8_t Wchar, Ychar, planes, bpp, banks;
    uint8_t memory_model, bank_size, image_pages;
    uint8_t reserved0;

    uint8_t red_mask, red_position;
    uint8_t green_mask, green_position;
    uint8_t blue_mask, blue_position;
    uint8_t rsv_mask, rsv_position;
    uint8_t directcolor_attributes;

    uint32_t physbase;  // your LFB (Linear Framebuffer) address ;)
    uint32_t reserved1;
    uint16_t reserved2;
} __attribute__((packed));

void fail(ModeInfoBlock* mib);
/// Parse both structures and fail with a red screen or return a GraphicalParam.
GraphicalParam graphParse(VbeInfoBlock* vid, ModeInfoBlock* mib);


#endif
