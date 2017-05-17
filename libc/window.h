#ifndef __SUPOS_WINDOW_H
#define WINDOW_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct{
        unsigned int x;
        unsigned int y;
    } vec_t;

    int   openwin   (vec_t size, vec_t offset, int workspace);
    int   opentwin  (vec_t size, vec_t offset, int workspace);
    int   resizewin (int fd,     vec_t size,   vec_t offset);
    vec_t getsize   (int fd);
    vec_t getoff    (int fd);
    int   getws     (int fd);

    typedef enum{
        K_ERROR, K_ESC, K_FL1, K_FL2, K_FL3, K_FL4, K_FL5, K_FL6, K_FL7, K_FL8,
        K_FL9, K_FL10, K_FL11, K_FL12,
        K_BACKSPACE, K_TAB,
        K_SL1, K_SL2, K_SL3, K_SL4, K_SL5, K_SL6, K_SL7, K_SL8, K_SL9, K_SL10, K_SL11, K_SL12,
        K_ENTER, K_CTRL,
        K_TL1, K_TL2, K_TL3, K_TL4, K_TL5, K_TL6, K_TL7, K_TL8, K_TL9, K_TL10, K_TL11,
        K_TOPLEFT, K_LSHIFT, K_SOMERIGHT,
        K_FOL1, K_FOL2, K_FOL3, K_FOL4, K_FOL5, K_FOL6, K_FOL7, K_FOL8, K_FOL9, K_FOL10,
        K_RSHIFT, K_KSTAR, K_ALT, K_SPACE, K_CAPSLOCK,
        K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10,
        K_NUMLOCK, K_SCROLL_LOCK,
        K_K7, K_K8, K_K9, K_KMINUS,
        K_K4, K_K5, K_K6, K_KPLUS,
        K_K1, K_K2, K_K3,
        K_K0, K_KDEL, K_T56 = 0x56, K_LWIN = 0x5b, K_RWIN = 0x5c
    }__attribute__((packed)) keycode_t;
    typedef struct{
        bool lShift : 1;
        bool rShift : 1;
        bool lCtrl  : 1;
        bool rCtrl  : 1;
        bool lAlt   : 1;
        bool rAlt   : 1; // Alt Gr
        bool lWin   : 1;
        bool rWin   : 1;
        bool capsLock : 1;
        bool numLock : 1;
        keycode_t code : 7;
        bool released :1;
        bool extended :1;
        bool valid : 1;
    }__attribute__((packed)) key_t;

    typedef enum{
        MOUSE_LEFT = 1, MOUSE_RIGHT = 2, MOUSE_MIDDLE = 4
    }__attribute__((packed)) mouse_button_t;
    typedef struct{
        short x;
        short y;
        char pressed;
        char released;
    }__attribute__((packed)) mouse_t;

    typedef enum{
        WIN_RESIZE, WIN_MOVE, WIN_FOCUS, WIN_WORKSPACE
    }__attribute__((packed)) win_type_t;
    typedef struct{
        unsigned short x;
        unsigned short y;
        unsigned char value;
        win_type_t type;
    }__attribute__((packed)) win_evt_t;


    typedef enum{
        EVT_KEYBOARD, EVT_MOUSE, EVT_WINDOW, EVT_INVALID
    }__attribute__((packed)) evt_type_t;
    typedef struct{
        evt_type_t type;
        union{
            key_t key;
            mouse_t mouse;
            win_evt_t win_evt;
        };
    }__attribute__((packed)) evt_t;
    evt_t getevt(int fd);


#ifdef __cplusplus
}
#endif

#endif
