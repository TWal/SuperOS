#include "../cstdlib"
#include "../vector"
#include "../deque"
#include "../string"
#include "../memory"
#include "../new"
#include "../iterator"
#include "../algorithm"
#include "../map"
#include "../set"
#include "../ios"
#include "../istream"
#include "../ostream"
#include "../fstream"

#ifndef SUP_OS_KERNEL

namespace std{

    istream cin;
    ostream cout;
    ostream cerr;
    ostream clog;

    int ios_base::Init::init_cnt = 0;
    ios_base::Init::Init(){
        if(init_cnt){
            ++init_cnt;
            return;
        }
        filebuf *f = new filebuf();
        f->open(0,ios_base::in);
        new(&cin) istream(f);
        f = new filebuf();
        f->open(1,ios_base::out);
        new(&cout) ostream(f);
        f = new filebuf();
        f->open(2,ios_base::out);
        new(&cerr) ostream(f);
        f = new filebuf();
        f->open(3,ios_base::out);
        new(&clog) ostream(f);

    }
    extern "C" void __cxa_pure_virtual(){
        assert(!"Pure virtual call");
    }
}

#endif
