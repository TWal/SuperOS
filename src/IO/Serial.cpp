#include "Serial.h"
#include "../utility.h"

const u16 dataPort = 0x3F8;
const u16 fifoPort = dataPort +2;
const u16 linePort = dataPort +3;
const u16 modemPort = dataPort +4;
const u16 statusPort = dataPort +5;



Serial::Serial(){
    outb(dataPort+1,0);
    outb(linePort,0x80); // sending divisor of frequency
    outb(dataPort,3); // 9600 Hz
    outb(dataPort+1,0);
    outb(linePort,0x3);
    outb(fifoPort,0xC7);
    outb(modemPort,0x3);
}
void Serial::write(char data){
    while (!(inb(statusPort)&0x20));
    outb(dataPort,data);
}
void Serial::write(const char* data){
    while (*data){
        write(*data);
        ++data;
    }
}

Serial ser;
