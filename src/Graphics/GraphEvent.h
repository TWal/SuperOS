#ifndef GRAPHEVENT_H
#define GRAPHEVENT_H

#include "../utility.h"
#include "Vec2.h"

namespace video{
    /**
       @brief Represent a graphical event.
    */
    struct GraphEvent{
        /**
           @brief Vector value for @ref RESIZE and @ref MOVE.
        */
        u16 x;
        u16 y;
        /**
           @brief Value on @ref FOCUS or @ref WORKSPACE.

           When @ref type is @ref FOCUS, `val` will be 1 or 0 depending if we have
           gained or lost the focus.

           When @ref type is @ref WORKSPACE, `val` is the number of the workspace
           which has been switches to.
        */
        u8 value;

        enum class Type : char {RESIZE, MOVE, FOCUS, WORKSPACE};
        Type type;

        GraphEvent(Type t, u8 val) : x(0), y(0), value(val), type(t){
            assert(t == Type::FOCUS or t == Type::WORKSPACE);
            if(t == Type::FOCUS) assert(val <= 1);
        }
        GraphEvent(Type t, u16 nx, u16 ny) : x(nx), y(ny), value(0), type(t){
            assert(t == Type::RESIZE or t == Type::MOVE);
        }
    } __attribute__((packed));
    static_assert(sizeof(GraphEvent) == 6, "GraphEvent is too big");
}

#endif
