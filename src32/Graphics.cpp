#include "Graphics.h"
#include "../src/IO/FrameBuffer.h"

void copy(char* a, char* b, u8 size){
    for(u32 i = 0 ; i < size ; ++i){
            a[i] = b[i];
    }
}


void fail(ModeInfoBlock* mib){
    kprintf("Entering fail\n");
    u8 bytesPerp = mib->bpp / 8;
    char* FB = (char*)mib->physbase;
    u32 red = ((1 << mib->red_mask) -1) << mib->red_position;
    u32 green = ((1 << mib->green_mask) -1) << mib->green_position;
    u32 blue = ((1 << mib->blue_mask) -1) << mib->blue_position;
    kprintf("blue %d, %d, %x\n",mib->blue_mask,mib->blue_position,blue);
    u32 white = red | green | blue;
    for(int i = 0 ; i < mib->Yres ; ++i){
        for(int j = 0 ; j < mib->Xres ; ++j){
            if( j < mib->Xres / 3){
                copy(FB+(i * mib->pitch + bytesPerp*j),(char*)&blue,bytesPerp);
                continue;
            }if( j < 2*mib->Xres / 3){
                copy(FB+(i * mib->pitch + bytesPerp*j),(char*)&white,bytesPerp);
                FB[i * mib->pitch + bytesPerp*j+2] = 255;
                continue;
            }
            copy(FB+(i * mib->pitch + bytesPerp*j),(char*)&red,bytesPerp);
        }
    }


    stop;
}

GraphicalParam graphParse(VbeInfoBlock* vid, ModeInfoBlock* mib){
    kprintf("Checking graphics Format :\n");
    kprintf("VBE Signature: %c%c%c%c\n",vid->VbeSignature[0],vid->VbeSignature[1],
            vid->VbeSignature[2],vid->VbeSignature[3]);
    if(!(vid->VbeSignature[0] == 'V' and vid->VbeSignature[1] == 'E'
         and vid->VbeSignature[2] =='S' and vid->VbeSignature[3] == 'A' )){
        kprintf("Fail\n");
        fail(mib);
    }
    kprintf("VBE mode attributes : %d\n",mib->attributes);
    if(!((mib->attributes &(8 + 16)) == (8 + 16))){
        kprintf("Fail\n");
        fail(mib);
    }
    kprintf("Resolution : %d * %d\n", mib->Xres * mib->Yres);
    kprintf("Pitch : %d\n", mib->pitch);
    kprintf("Bit per pixel : %d",mib->bpp);
    if(mib->bpp !=32){
        kprintf("Fail\n");
        fail(mib);
    }
    kprintf("Memory Model : %d\n",mib->memory_model);
    if(mib->memory_model !=6){ // Direct Color
        kprintf("Fail\n");
        fail(mib);
    }
    kprintf("Red width : %d and pos : %d\n", mib->red_mask,mib->red_position);
    if(!(mib->red_mask == 8 and mib->red_position == 16)){
        kprintf("Fail\n");
        fail(mib);
    }
    kprintf("Green width : %d and pos : %d\n", mib->green_mask,mib->green_position);
    if(!(mib->green_mask == 8 and mib->green_position == 8)){
        kprintf("Fail\n");
        fail(mib);
    }
    kprintf("Blue width : %d and pos : %d\n", mib->blue_mask,mib->blue_position);
    if(!(mib->blue_mask == 8 and mib->blue_position == 0)){
        kprintf("Fail\n");
        fail(mib);
    }
    GraphicalParam params;
    params.physptr = mib->physbase;
    params.Xsize = mib->Xres;
    params.Ysize = mib->Yres;
    params.pitch = mib->pitch;
    return params;
}
