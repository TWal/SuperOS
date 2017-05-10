#ifndef SCREEN_H
#define SCREEN_H

#include "../utility.h"
#include "../../src32/Graphics.h"
#include "Vec2.h"
#include "Font.h"

/**
   @brief This namespace contains all content related to the screen and graphics.

   The class Screen is a singleton encapsulating the screen. Use only for
   direct screen access

   The class Workspace represent a workspace for a tiling environment i.e a
   area where you can place window and can be switched from one to another.

   The class Window represent a drawable area in Workspace (and Maybe in
   another Window in the future)
 */

namespace video{

    struct Color24{
        u8 B,G,R;
        static Color24 white;
        static Color24 black;
        static Color24 red;
        static Color24 blue;
        static Color24 green;
        static Color24 yellow;
        static Color24 magenta;
        static Color24 cyan;
        static Color24 lred;
        static Color24 lblue;
        static Color24 lgreen;
        static Color24 lyellow;
        static Color24 lmagenta;
        static Color24 lcyan;
        static Color24 lwhite;
    };

    /**
       @brief Represent a color on the screen;
    */
    struct Color : public Color24{
        u8 zero;
        Color(): Color24(Color24::black), zero(0){}
        Color(Color24 c24): Color24(c24), zero(0){}
        operator uint(){return *reinterpret_cast<uint*>(this);}
    };
    static_assert(sizeof(Color) == 4);


    /**
       @brief Singleton that represent the graphical screen

       Screen is handled by the VBE standards, GRUB set up a linear buffer of
       32bit depth color on boot, The is really inefficient in terms of performance
       but does the the job for a small experimental OS like this one.

    */
    class Screen{
        u16 Xsize, Ysize;
        u32 pitch; ///< Bytes per line
        /// Pointer to the real buffer corresponding to the screen
        static char* const VGAbuffer;
        static Color* const buffer; ///< buffer which is written to (double buffering)
        bool _OK = false;
    public:
        void init(GraphicalParam* gp);
        ///slow low-level access;
        Color get(uint x, uint y);
        void set(uint x, uint y, Color c);
        void send(); // do not clear buffer
        void writeLine(uint nb, uint offset, uint size, Color* buffer);
        void clear();
        void clear(Vec2u offset,Vec2u size);
        void putChar(char c, uint x, uint y, const Font& font, Color fg, Color bg);
        Vec2u getSize(){return {Xsize,Ysize};}
        bool isOK(){return _OK;}
    };

    extern Screen screen;
};

#endif
