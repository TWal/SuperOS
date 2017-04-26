#ifndef WAITING_H
#define WAITING_H

#include <assert.h>
#include <vector>
#include <set>
#include <functional>

class Thread;
class Waiting;
/**
   @brief An interface of an object that can be waited (i.e a stream, a child
   process, ...)

   This class is friend with Waiting and they work together.

   Invariant : When w is in _waitings, then w->_waiting = this.

 */
class Waitable{
    friend Waiting;
    std::set<Waiting*> _waitings;
protected:
    /**
       @brief The deriving class must call this when the reason number i is no longer needed
    */
    void free();
public:
    /// Create new Waitable object with nb reason to wait.
    explicit Waitable() : _waitings(){};
    ~Waitable(){
        assert(_waitings.empty());
    }
};


/**
   @brief An interface for an object that will have to wait for something (a Thread).

   This class is friend with Waitable and they work together.

   There is three state :
       - Nothing : _waited = nullptr, _waiting = {} and _checker = nullptr
       - Waiting : _waited = nullptr, _waiting = {waited waitable objects},
         _checker = function to check if we have finished
       - Waiting finished : _waited = the class that has signaled its
       should not be waited for, _waiting = the same thing, _checker = the same thing

 */
class Waiting{
    friend Waitable;
    std::set<Waitable*> _waiting;
    Waitable* _waited;
    using checker = std::function<void(Waiting*, Waitable*)>;
    //using checker = void(*)(Waiting*, Waitable*);
    checker _checker; // checker must call accept or refuse.
public:
    Waiting() : _waited(nullptr),_checker(nullptr){};
    explicit Waiting(std::set<Waitable*> waiting,checker checker)
        : _waited(nullptr),_checker(checker){
        wait(waiting,checker);
    }
    ~Waiting(){
        assert(!_waited && _waiting.empty());
    };
    /// When you finally decide not to wait.
    inline void wait(std::set<Waitable*> waiting,checker checker){
        assert(!_waited && _waiting.empty());
        _checker = checker;
        _waiting = std::move(waiting);
        for(auto w : _waiting){
            printf("Registering Waiting %p to Waitable %p",this,w);
            w->_waitings.insert(this);
        }
    }
    /// Check if we are still waiting.
    inline bool OK(){
        if(!_waited) return _waiting.empty();
        printf("Non trivial OK on %p\n",this);
        auto tmp = _waited;
        _waited = nullptr;
        _checker(this,tmp);
        return OK();
    }
    inline void accept(){
        _waiting.clear();
    }
    inline void refuse(){
        for(auto w : _waiting){
            w->_waitings.insert(this);
        }
    }
};


inline void Waitable::free(){
    printf("waitable::Free : %p with %llu waiters\n",this,_waitings.size());
    for(auto w : _waitings){
        w->_waited = this;
    }
    _waitings.clear();
}


#endif
