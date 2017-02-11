#include "HardDrive.h"
#include "globals.h"

static const uint bus1BasePort = 0x1F0;
static const uint bus1SpecialPort = 0x3F6;
static const uint bus2BasePort = 0x170;
static const uint bus2SpecialPort = 0x376;


StatusByte::StatusByte(uchar byte){
    *reinterpret_cast<uchar*>(this) = byte;
}

void StatusByte::printStatus(){
    fb.printf("byte: %d,error : %d, ready : %d, busy : %d ",*reinterpret_cast<uchar*>(this),error,ready,busy);
}

StatusByte HDD::getStatus(){
    return inb(_basePort + 7);
}

bool HDD::isThereADisk(){
    return inb(_basePort + 7) != 0xFF;
}

void HDD::init(){
}

HDD::HDD(uchar bus,bool master) : _master(master),active(false),table{}
{
    switch(bus){
        case 1:
            _basePort = bus1BasePort;
            break;
        case 2:
            _basePort = bus2BasePort;
            break;
        default:
            bsod("Unknown IDE bus number : %d\n",bus);
    }
}


void HDD::writelba (lint LBA , void* data, uint size){
    (void) data;
    assert(size > 0 && size < (0x200000 - 512) && (size &2)); // 0x200000 = 512 * nb max de secteur
    activate();
    ushort nbsector = size / 512;
    if (size % 512) ++size;
    outb (_basePort + 2, nbsector >> 8);
    outb (_basePort + 3, (LBA >> 24) & 0xFF);
    outb (_basePort + 4, (LBA >> 32) & 0xFF);
    outb (_basePort + 5, (LBA >> 40) & 0xFF);
    outb (_basePort + 2, nbsector & 0xFF);
    outb (_basePort + 3, LBA & 0xFF);
    outb (_basePort + 4, (LBA >> 8) & 0xFF);
    outb (_basePort + 5, (LBA >> 16) & 0xFF);
    while(!getStatus().isReadyOrFailed()){}
    assert(getStatus().isOk());

    //for (int i = 0 ; i < size /2)

}


void HDD::readlba (lint LBA, void * data, uint size){
    assert(size > 0 && size < (0x200000 - 512) && (size %2 == 0)); // 0x200000 = 512 * nb max de secteur
    activate();
    ushort nbsector = (size + 511) / 512;
    outb (_basePort + 2, nbsector >> 8);
    outb (_basePort + 3, (LBA >> 24) & 0xFF);
    outb (_basePort + 4, (LBA >> 32) & 0xFF);
    outb (_basePort + 5, (LBA >> 40) & 0xFF);
    outb (_basePort + 2, nbsector & 0xFF);
    outb (_basePort + 3, LBA & 0xFF);
    outb (_basePort + 4, (LBA >> 8) & 0xFF);
    outb (_basePort + 5, (LBA >> 16) & 0xFF);
    outb (_basePort + 7,0x24);
    while(!getStatus().isReadyOrFailed()){}
    assert(getStatus().isOk());
    for(uint i = 0 ; i < size/2 ; ++ i ){
        reinterpret_cast<ushort*>(data)[i] = inw(_basePort);
    }

}

