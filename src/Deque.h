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

        T& operator[](size_t n) {
            return _array[(_begin+n)%N];
        }

        const T& operator[](size_t n) const {
            return _array[(_begin+n)%N];
        }

        T& at(size_t n) {
            assert(n < _size);
            return _array[(_begin+n)%N];
        }

        const T& at(size_t n) const {
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

        size_t size() const {
            return _size;
        }

        bool empty() const {
            return _size == 0;
        }

    private:
        size_t _begin;
        size_t _end;
        size_t _size;
        T _array[N];
};

#endif

