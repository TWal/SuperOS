#ifndef DEQUE_H
#define DEQUE_H

#include "utility.h"

template<typename T, uint N>
class Deque {
    public:
        Deque() :_begin(0), _end(0), _size(0) { }

        void push_back(const T& v) {
            assert(_size < N);
            _array[_end] = v;
            _end = (_end+1)%N;
            _size += 1;
        }

        void push_front(const T& v) {
            assert(_size < N);
            _begin = (_begin+N-1)%N;
            _array[_begin] = v;
            _size += 1;
        }

        void pop_back() {
            assert(_size > 0);
            _end = (_end+N-1)%N;
            _size -= 1;
        }

        void pop_front() {
            assert(_size > 0);
            _begin = (_begin+1)%N;
            _size -= 1;
        }

        T& operator[](uint n) {
            return _array[(_begin+n)%N];
        }

        const T& operator[](uint n) const {
            return _array[(_begin+n)%N];
        }

        T& at(uint n) {
            assert(n < _size);
            return _array[(_begin+n)%N];
        }

        const T& at(uint n) const {
            assert(n < _size);
            return _array[(_begin+n)%N];
        }

        T& front() {
            return _array[_begin];
        }

        const T& front() const {
            return _array[_begin];
        }

        T& back() {
            return _array[(_end+N-1)%N];
        }

        const T& back() const {
            return _array[(_end+N-1)%N];
        }

        uint size() const {
            return _size;
        }

        bool empty() const {
            return _size == 0;
        }

    private:
        uint _begin;
        uint _end;
        uint _size;
        T _array[N];
};

#endif

