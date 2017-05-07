#include "Graphics.h"
#include "../src/IO/FrameBuffer.h"
#include "log.h"

void copy(char* a, char* b, u8 size){
    for(u32 i = 0 ; i < size ; ++i){
            a[i] = b[i];
    }
}


void fail(ModeInfoBlock* mib, ErrNum errNum){
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
    for(int i = 1 ; i <= errNum ; ++i){
        for(int j = 0 ; j < 100 ; ++j){
            copy(FB+(i*10 * mib->pitch + bytesPerp*j),(char*)&white,bytesPerp);
            copy(FB+((i*10)+1 * mib->pitch + bytesPerp*j),(char*)&white,bytesPerp);
        }
    }
    stop;
}

ModeInfoBlock* _mib = nullptr;

void rfail(ErrNum errNum){
    fail(_mib,errNum);
}

GraphicalParam graphParse(VbeInfoBlock* vid, ModeInfoBlock* mib){
    info(Screenl,"Checking graphics Format");
    debug(Screenl,"VBE Signature: %c%c%c%c",vid->VbeSignature[0],vid->VbeSignature[1],
            vid->VbeSignature[2],vid->VbeSignature[3]);
    if(!(vid->VbeSignature[0] == 'V' and vid->VbeSignature[1] == 'E'
         and vid->VbeSignature[2] =='S' and vid->VbeSignature[3] == 'A' )){
        error(Screenl,"Wrong VBE Signature");
        fail(mib,WrongSignature);
    }

    debug(Screenl,"VBE mode attributes : %x",mib->attributes);
    if(!((mib->attributes &(8 + 16)) == (8 + 16))){ // TODO understand that again
        error(Screenl,"VBE Attributes not OK");
        fail(mib,WrongAttribute);
    }


    debug(Screenl,"Linear Buffer physical address : %p",mib->physbase);
    info(Screenl,"Resolution : %d * %d", mib->Xres, mib->Yres);
    debug(Screenl,"Pitch : %d", mib->pitch);
    info(Screenl,"Bit per pixel : %d",mib->bpp);
    if(mib->bpp !=32){
        error(Screenl,"No 32bit depth");
        fail(mib,WrongDepth);
    }

    debug(Screenl,"Memory Model : %d",mib->memory_model);
    if(mib->memory_model !=6){ // Direct Color
        error(Screenl,"Wrong Memory model");
        fail(mib,WrongMemoryModel);
    }

    debug(Screenl,"Red width : %d and pos : %d", mib->red_mask,mib->red_position);
    if(!(mib->red_mask == 8 and mib->red_position == 16)){
        error(Screenl, "Invalid Red mask");
        fail(mib,InvalidRedMask);
    }

    debug(Screenl,"Green width : %d and pos : %d", mib->green_mask,mib->green_position);
    if(!(mib->green_mask == 8 and mib->green_position == 8)){
        error(Screenl,"Invalid Green mask");
        fail(mib,InvalidGreenMask);
    }

    debug(Screenl,"Blue width : %d and pos : %d", mib->blue_mask,mib->blue_position);
    if(!(mib->blue_mask == 8 and mib->blue_position == 0)){
        error(Screenl,"Invalid Blue mask");
        fail(mib,InvalidBlueMask);
    }

    GraphicalParam params;
    params.physptr = mib->physbase;
    params.Xsize = mib->Xres;
    params.Ysize = mib->Yres;
    params.pitch = mib->pitch;
    return params;
}
