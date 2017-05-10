#ifndef TEXTWINDOW_H
#define TEXTWINDOW_H

#include "Window.h"
#include "../Streams/Stream.h"
#include <deque>

namespace video{

    /**
       @brief Provide a textual interface for text applications.

       An application can open a text window and it will generate 2 files
       descriptors, one for the window and one for the textual In-Out.

     */

    class TextWindow : public Window, public Stream{
        Font* _font;///< font
        struct Format{
            Color24 fg;
            bool underline;
            Color24 bg;

            Format(): fg(Color24::lwhite), underline(false), bg(Color24::black){}
        };
        static_assert(sizeof(Format) == 7);

        struct FormatedChar : public Format{
            char c;
            FormatedChar() : Format(), c(' '){}
            FormatedChar(char nc, Format fmt):
                Format(fmt), c(nc){}
        };
        static_assert(sizeof(FormatedChar) == 8);

        struct Line{
            std::vector<FormatedChar> data;
            explicit Line(){}
            void write(size_t i, FormatedChar fmc){
                if(data.size() <= i) data.resize(i+1);
                data[i]=fmc;
            }
        };

        Format _cursFormat;
        u8 _height;
        u8 _width;
        Vec2u _curs;
        std::deque<Line> _lineBuffer;
        enum State{NORMAL, WAITINGOB, ESCAPE};
        State _state;
        std::vector<uint> _stack;
        bool _bright;
        static const uint LineNum = 256;

        //input
        std::deque<char> _inputBuffer;
        std::string _keyboardBuffer;




        Color24 getByNum(u8 val){
            if(_bright){
                switch(val){
                    case 0: return Color24::black;
                    case 1: return Color24::red;
                    case 2: return Color24::green;
                    case 3: return Color24::yellow;
                    case 4: return Color24::blue;
                    case 5: return Color24::magenta;
                    case 6: return Color24::cyan;
                    case 7: return Color24::white;
                }
            }
            else{
                switch(val){
                    case 0: return Color24::black;
                    case 1: return Color24::lred;
                    case 2: return Color24::lgreen;
                    case 3: return Color24::lyellow;
                    case 4: return Color24::lblue;
                    case 5: return Color24::lmagenta;
                    case 6: return Color24::lcyan;
                    case 7: return Color24::lwhite;
                }
            }
            assert(false);
        }

        /// Handle SGR commands
        void SGR();
        void newLine();
        void addPrintableChar(char c);
        void error();
        void drawFMC(Vec2u pos, FormatedChar fmc) const;

    public:
        /// Create window
        TextWindow(Vec2u offset, Vec2u size, Font* font)
            : Window(offset,size), _font(font),
              _height((size.y -2) / 16), _width((size.x -2) / 8),
              _state(State::NORMAL), _bright(false), allowInput(true)
            {
                _lineBuffer.push_back(Line());
        }

        u64 getMask() const {return Stream::READABLE | Stream::WRITABLE |
                Stream::APPENDABLE;}
        /// Read something typed by keyboard
        size_t read(void* buf, size_t count);
        /// TODO handle ctrl-D
        bool eof() const {return false;};

        size_t write(const void* buf, size_t count);
        /// Draw the text on the screen.
        void send() const;
        /// Add char to the buffer (parse escape codes)
        void putChar(char c);
        /// handle Event (Ignore mouse).
        virtual void handleEvent(input::Event e);
        /// if false all keyboard input is ignored
        bool allowInput;
        virtual void setSize(const Vec2u& v);
    };

};

#endif
