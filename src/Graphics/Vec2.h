#ifndef VEC2_H
#define VEC2_H

#include "../utility.h"

namespace video{
    template<typename T>
    struct Vec2{
        T x;
        T y;
        Vec2() :x(0),y(0){}
        template<typename U>
        explicit Vec2(U u){
            static_assert(sizeof(U) == 2* sizeof(T),"Vec2 unique input has wrong size");
            x = u & ((U(1) << (sizeof(T)*8)) -1);
            y = u >> (sizeof(T)*8);
        }


        Vec2(T nx, T ny) : x(nx),y(ny){}
        template<typename U>
        Vec2(const Vec2<U>& v) : x(v.x),y(v.y){}

        template<typename U>
        Vec2& operator=(const Vec2<U>& v){
            x = v.x;
            y = v.y;
            return *this;
        }
        template<typename U>
        Vec2& operator*= (U fact){
            x *= fact;
            y *= fact;
            return *this;
        }
        template<typename U>
        Vec2& operator+= (Vec2<U> v){
            x+= v.x;
            y+= v.y;
            return *this;
        }
        template<typename U>
        Vec2& operator-= (Vec2<U> v){
            x-= v.x;
            y-= v.y;
            return *this;
        }
        T area()const{
            assert(x >= 0 && y >= 0);
            return x * y;
        }

    };

    template<typename T,typename U>
    Vec2<T> operator+(Vec2<T> v1, const Vec2<U>& v2){
        return v1+=v2;
    }
    template<typename T,typename U>
    Vec2<T> operator-(Vec2<T> v1, const Vec2<U>& v2){
        return v1-=v2;
    }
    template<typename T,typename U>
    Vec2<T> operator*(Vec2<T> v1, U t){
        return v1*=t;
    }

    using Vec2i = Vec2<int>;
    using Vec2u = Vec2<uint>;
};

#endif
