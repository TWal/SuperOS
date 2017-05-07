#ifndef FONT_H
#define FONT_H

#include"../utility.h"
namespace video{

    /**
       @brief Represent a font char in PSF format 8 by 16.
     */
    using FontChar = char[16];

    /**
       @brief Contains a Font in PSF1 format.

       This class is directly the file
       (you can reinterpret_cast the file data into this class).
     */
    class Font{
        u16 magic; ///< Must be 0x0436
        u8 mode; ///< Ignored.
        u8 size; ///< Must be 16. Other sizes not supported.
        FontChar data[256];
    public:
        ///< Default font loaded during boot.
        static Font* def;
        /// Initialize default font from its physical pointer.
        static void defInit(u64 fontPhyPtr);
        /// initialize a font.
        void init(){
            assert(magic == 0x0436);
            assert(size == 16);
        }
        /**
           @brief Write a line of a certain letter to a buffer.

           @param c The char to be printed.
           @param line The line of that char to be printed 0<= line <16
           @param buf The buffer on which it will be printed. (linear graphic buffer)
           @param fg The foregroung color
           @param bg the backgorund color
         */
        void writeLine(uchar c,uint line, void* buf,uint fg, uint bg) const ;
    };

};

#endif
