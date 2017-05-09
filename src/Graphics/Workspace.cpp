#include "Workspace.h"
#include "Window.h"
#include "../IO/Mouse.h"
#include "../log.h"

using namespace input;

namespace video{

    Workspace Workspace::_elems[Workspace::_totalNumber];
    uint Workspace::active = 0;
    void Workspace::init(){
        for(uint i = 0 ; i <_totalNumber ; ++i){
            get(i)._number =i;
        }
    }

    void Workspace::drawMe(){
        //debug(Graphics,"Drawing Workspace number %u",_number);
        screen.clear();
        assert(_number == active);
        if(_wins.empty()){
            screen.send();
            return;
        }
        auto it = _wins.end();
        while(it != _wins.begin()){
            --it;
            (*it)->send();
        }
        _wins.front()->drawEdge();
        mouse.draw();
        screen.send();
    }

    void Workspace::cycle(){
        debug(Graphics,"Cycle on Workspace %d",_number);
        if(_wins.empty()) return;
        Window* tmp = _wins.front();
        _wins.pop_front();
        _wins.push_back(tmp);
        _wins.front()->show();
    }

    bool Workspace::handleEventOnMe(input::Event e){
        if(e.type == Event::KEYBOARD){
            if(e.kcode.state.lAlt and e.kcode.scanCode.code == Keyboard::TAB){
                cycle();
                return true;
            }
        }
        for(const auto& win : _wins){
            if(win->handleEvent(e)) return true;
        }
        return false;
    }

    bool Workspace::handleEvent(Event e){
        if(e.type == Event::KEYBOARD){
            if(e.kcode.state.lCtrl or e.kcode.state.rCtrl){
                if(e.kcode.scanCode.code >= Keyboard::F1
                   and e.kcode.scanCode.code <= Keyboard::F10 ){
                    Workspace::active = (e.kcode.scanCode.code - Keyboard::F1 +1) %10;
                    return true;
                }
            }
        }
        return get(Workspace::active).handleEventOnMe(e);
    }

};
