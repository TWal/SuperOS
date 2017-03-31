#ifndef SERIAL_H
#define SERIAL_H

class Serial{
public :
    Serial();
    void write(char data);
    void write(const char* data);
};

extern Serial ser;

#endif
