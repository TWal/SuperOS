#include "Mouse.h"
#include <stdio.h>
#include "../Graphics/Screen.h"
#include "../log.h"

Mouse mouse;

MouseEvent::MouseEvent() :
    x(0), y(0), pressed(255), changed(255) {}
MouseEvent::MouseEvent(u16 x_, u16 y_, u8 pressed_, u8 lastPressed_) :
    x(x_), y(y_), pressed(pressed_), changed(pressed_ ^ lastPressed_) {}


//Code adapted from http://forum.osdev.org/viewtopic.php?t=10247
//and http://wiki.osdev.org/PS/2_Mouse

Mouse::Mouse() :
    _totalX(0), _totalY(0), _byte{0, 0, 0}, _cycle(0), _lastPressed(0) {}

void Mouse::init() {
    u8 status;

    //Enable the auxiliary mouse device
    _wait(true);
    outb(0x64, 0xA8);

    //Enable the interrupts
    _wait(true);
    outb(0x64, 0x20);
    _wait(false);
    status=(inb(0x60) | 2);
    _wait(true);
    outb(0x64, 0x60);
    _wait(true);
    outb(0x60, status);

    //Reference: http://www.computer-engineering.org/ps2mouse/

    //Reset the mouse
    _write(0xFF);
    // <= 1000000000
    //WAIT(1000);
    assert(_read() == 0xfa); //Acknowledge
    assert(_read() == 0xaa); //Self-test passed
    assert(_read() == 0x00); //Mouse ID

    //Tell the mouse to use default settings
    _write(0xF6);
    assert(_read() == 0xfa); //Acknowledge

    //Enable the mouse
    _write(0xF4);
    assert(_read() == 0xfa); //Acknowledge

    _totalX = 0;
    _totalY = 0;
}

void Mouse::handleByte(u8 b) {
    i16 x;
    i16 y;
    switch(_cycle) {
        case 0:
            //Dirty hack: on qemu, there is a single interrupt
            //that "shifts" everything. This condition may detect this.
            if((b & (1<<3)) == 0) break;
            _byte[0] = b;
            _cycle += 1;
            break;
        case 1:
            _byte[1] = b;
            _cycle += 1;
            break;
        case 2:
            _byte[2] = b;
            x = _byte[1] - ((_byte[0] << 4) & 0x100);
            y = _byte[2] - ((_byte[0] << 3) & 0x100);
            _totalX = min(max(_totalX+x, 0), (i32)video::screen.getSize().x-1);
            _totalY = min(max(_totalY-y, 0), (i32)video::screen.getSize().y-1);
            if(_lastPressed != (_byte[0] & 0x7)) {
                _deque.push_back(MouseEvent(_totalX, _totalY, _byte[0] & 0x7, _lastPressed));
                _lastPressed = _byte[0] & 0x7;
            }
            //video::screen.set(_totalX, _totalY, video::Color24::white);
            _cycle = 0;
            break;
    }
}

MouseEvent Mouse::poll() {
    if(_deque.empty()) {
        return MouseEvent();
    } else {
        MouseEvent m = _deque.front();
        _deque.pop_front();
        return m;
    }
}

MouseEvent Mouse::pollLast() {
    return MouseEvent(_totalX, _totalY, _lastPressed, _lastPressed);
}


static const u16 black[28] = {
    0x0000, 0x4000, 0x6000, 0x7000,
    0x7e00, 0x6380, 0x6b7c, 0x62fe,
    0x7efe, 0x7efe, 0x7f7e, 0x7f9e,
    0x7fc0, 0x7ff8, 0x7ff8, 0x7ff8,
    0x7ff8, 0x7ff0, 0x7ff0, 0x3ff0,
    0x3fe0, 0x1fc0, 0x0f80, 0x0400,
    0x0400, 0x05f8, 0x0308, 0x0000
};

static const u16 white[28] = {
    0xc000, 0xa000, 0x9000, 0x8e00,
    0x8180, 0x9c7c, 0x9482, 0x9d01,
    0x8101, 0x8101, 0x8081, 0x8061,
    0x803e, 0x8004, 0x8004, 0x8004,
    0x8004, 0x8008, 0x8008, 0x4008,
    0x4010, 0x2020, 0x1040, 0x0b80,
    0x0bf8, 0x0a0c, 0x04f4, 0x0308
};

void Mouse::draw() {
    video::Vec2u size = video::screen.getSize();
    uint startx = 0;
    if((u32)_totalX + 16 >= size.x) {
        startx = _totalX + 16 - size.x + 1;
    }

    for(uint y = 0; y < 28; ++y) {
        if(_totalY + y >= size.y) break;
        for(uint x = startx; x < 16; ++x) {
            bool bblack = ((black[y] >> x) & 1) != 0;
            bool bwhite = ((white[y] >> x) & 1) != 0;
            if(bblack) {
                video::screen.set(_totalX + 16 - x, _totalY+y, video::Color24::black);
            }
            if(bwhite) {
                video::screen.set(_totalX + 16 - x, _totalY+y, video::Color24::white);
            }
        }
    }
}

void Mouse::_wait(bool type) {
    u32 timeout=5000000;
    if(type==false) {
        while(timeout--) {
            if((inb(0x64)&1) == 1) return;
        }
        printf("MOUSE TIMEOUT\n");
        return;
    } else {
        while(timeout--) {
            if((inb(0x64)&2) == 0) return;
        }
        printf("MOUSE TIMEOUT\n");
        return;
    }
}

void Mouse::_write(u8 a_write) {
    //Wait to be able to send a command
    _wait(true);
    //Tell the mouse we are sending a command
    outb(0x64, 0xD4);
    //Wait for the final part
    _wait(true);
    //Finally write
    outb(0x60, a_write);
}

u8 Mouse::_read() {
    //Get response from mouse
    _wait(false);
    return inb(0x60);
}

