#ifndef OUTMIXSTREAM_H
#define OUTMIXSTREAM_H

#include "Stream.h"
#include <vector>

/**
   @brief Write Its output to a combination of other Streams.

   The other Stream must be instant (write always writes all) and without end.
 */
class OutMixStream : public Stream{
    std::vector<Stream*> _outputs;
public:
    u64 getMask() const {return Stream::WRITABLE;}
    OutMixStream(){}
    OutMixStream(std::vector<Stream*> outputs): _outputs(outputs){
        for(auto s : _outputs){
            assert(s->check(Stream::WRITABLE));
        }
    }
    size_t write(const void * buf,size_t count){
        for(auto s: _outputs){
            size_t v =  s->bwrite(buf,count);
            assert(v == count);
        }
    }
};

#endif
