#ifndef SCREEN_H
#define SCREEN_H

#include "../utility.h"
#include "../../src32/Graphics.h"
#include "Font.h"

namespace video{

    /**
       @brief Represent a color on the screen;
    */
    struct Color{
        u8 blue,green,red;
        u8 zero;
        static Color white;
        static Color black;
        operator uint(){return *reinterpret_cast<uint*>(this);}
    };

    /**
       @brief Singleton that represent a graphical screen

    */
    class Screen{
        u16 Xsize, Ysize;
        u32 pitch; ///< Bytes per line
        /// Pointer to the real buffer corresponding to the screen
        static char* const VGAbuffer;
        static Color* const buffer; ///< buffer which is written to (double buffering)
    public:
        void init(GraphicalParam* gp);
        ///slow low-level access;
        Color get(int x, int y);
        void set(int x, int y,Color);
        void send(); // do not clear buffer
        void clear();
        void putChar(char c,int x, int y,const Font& font,Color fg,Color bg);
    };

    extern Screen screen;
};

#endif
