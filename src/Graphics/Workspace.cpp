#include "Workspace.h"
#include "Window.h"

namespace video{

    Workspace Workspace::_elems[Workspace::_totalNumber];
    uint Workspace::active = 0;
    void Workspace::init(){
    }

    void Workspace::drawMe(){
        assert(_number == active);
        if(_wins.empty())return;
        for(const auto& win : _wins){
                win->send();
        }
        _wins.front()->drawEdge();
    }

};
