#ifndef WAITING_H
#define WAITING_H

#include <assert.h>
#include <vector>
#include <set>

class Thread;
class Waiting;
/**
   @brief An interface of an object that can be waited (i.e a stream, a child
   process, ...)

   This class is friend with Waiting and they work together.

   The invariant is that all object in the entry number i of _waitings have
   their _val to i and their _wait pointing to this class.
 */
class Waitable{
    friend Waiting;
    std::vector<std::set<Waiting*>> _waitings;
protected:
    /**
       @brief The deriving class must call this when the reason number i is no longer needed
    */
    void free(size_t i);
public:
    /// Create new Waitable object with nb reason to wait.
    explicit Waitable(size_t nb) : _waitings(nb){};
    ~Waitable(){
        for(auto s : _waitings){
            assert(s.empty());
        }
    }
};
/**
   @brief An interface for an object that will have to wait for something (a Thread).

   This class is friend with Waitable and they work together.

   The invariant is that this is in _wait->_waitings[_val] or _wait = nultptr.
 */
class Waiting{
    friend Waitable;
    Waitable* _wait;
    size_t _val;
public:
    Waiting() : _wait(nullptr),_val(0){};
    explicit Waiting(Waitable* wait, size_t val) : _wait(wait),_val(val){
        assert(wait->_waitings.size() > val);
        wait->_waitings[val].insert(this);
    }
    ~Waiting(){assert(!_wait);};
    /// When you finally decide not to wait.
    inline void stopWait(){
        assert(_wait->_waitings[_val].count(this));
        _wait->_waitings[_val].erase(this);
        _wait = nullptr;
    }
    inline void wait(Waitable * wait,size_t val){
        assert(wait->_waitings.size() > val);
        wait->_waitings[val].insert(this);
    }
    /// Check if we are still waiting.
    inline bool OK(){return !_wait;}
};


inline void Waitable::free(size_t i){
    for(auto w : _waitings[i]){
        assert(w->_wait == this);
        w->_wait = nullptr;
    }
}

#endif
