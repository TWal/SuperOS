#include "Workspace.h"
#include "Window.h"
#include "../IO/Mouse.h"
#include "../log.h"

using namespace input;

namespace video{

    Workspace Workspace::_elems[Workspace::_totalNumber];
    uint Workspace::active = 0;
    uint Workspace::_currentKeymap = 0;

    void Workspace::init(){
        for(uint i = 0 ; i <_totalNumber ; ++i){
            get(i)._number =i;
        }
        kbd.setKeymap(input::keymaps[_currentKeymap]);
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
            (*it)->drawEdge(Color({0, 50, 150}));
        }
        _wins.front()->drawEdge(Color::red);
        mouse.draw();
        screen.send();
    }

    void Workspace::cycle(){
        debug(Graphics,"Cycle on Workspace %d",_number);
        if(_wins.empty()) return;
        Window* tmp = _wins.front();
        _wins.pop_front();
        _wins.push_back(tmp);
    }

    void Workspace::handleEventOnMe(input::Event e){
        if(e.type == Event::KEYBOARD){
            if((e.kcode.state.lWin or e.kcode.state.rWin)
               and e.kcode.scanCode.code == Keyboard::TAB
               and !e.kcode.scanCode.release){
                cycle();
            }
            if(e.kcode.state.lWin or e.kcode.state.rWin) {
                _superState = true;
            } else {
                _superState = false;
                _state = NORMAL;
            }
        }

        if(e.type == Event::MOUSE) {
            //Update state
            if(_superState && !_wins.empty()) {
                if(e.mousec.get(MouseEvent::LEFT, MouseEvent::JUSTPRESSED)) {
                    _state = MOVING;
                    _currentWindow = _wins.front();
                    _startMousePos = {e.mousec.x, e.mousec.y};
                    _startWindow = _currentWindow->getOffset();
                } else if(e.mousec.get(MouseEvent::RIGHT, MouseEvent::JUSTPRESSED)) {
                    _state = RESIZING;
                    _currentWindow = _wins.front();
                    _startMousePos = {e.mousec.x, e.mousec.y};
                    _startWindow = _currentWindow->getSize();
                } else if(!e.mousec.get(MouseEvent::LEFT, MouseEvent::PRESSED)
                       && !e.mousec.get(MouseEvent::RIGHT, MouseEvent::PRESSED)) {
                    _state = NORMAL;
                    _currentWindow = nullptr;
                }
            }

            if(_wins.empty() || _currentWindow != _wins.front()) {
                _state = NORMAL;
            } else {
                if(_superState) {
                    if(_state == MOVING) {
                        Vec2i newPos = _startWindow - _startMousePos + Vec2i(e.mousec.x, e.mousec.y);
                        Vec2i winSize = _currentWindow->getSize();
                        Vec2i screenSize = screen.getSize();
                        newPos.x = max(0, min(newPos.x, screenSize.x - winSize.x));
                        newPos.y = max(0, min(newPos.y, screenSize.y - winSize.y));
                        _currentWindow->setOffset(newPos);
                    }
                    if(_state == RESIZING) {
                        Vec2i newSize = _startWindow - _startMousePos + Vec2i(e.mousec.x, e.mousec.y);
                        Vec2i winOff = _currentWindow->getOffset();
                        Vec2i screenSize = screen.getSize();
                        newSize.x = max(10, min(newSize.x, screenSize.x - winOff.x));
                        newSize.y = max(10, min(newSize.y, screenSize.y - winOff.y));
                        _currentWindow->setSize(newSize);
                    }
                } else {
                    _state = NORMAL;
                }
            }

            Window* win = nullptr;
            for(Window* w : _wins) {
                if(w->isInside(Vec2u(e.mousec.x, e.mousec.y))) {
                    win = w;
                    break;
                }
            }

            if(win != nullptr) {
                if((e.mousec.pressed & e.mousec.changed) != 0) {
                    //TODO: implement erase in deque!!
                    while(_wins.front() != win) {
                        Window* w = _wins.front();
                        _wins.pop_front();
                        _wins.push_back(w);
                    }
                }
                e.mousec.x -= win->getOffset().x;
                e.mousec.y -= win->getOffset().y;
                win->handleEvent(e);
            }
        }

        if(e.type == Event::KEYBOARD) {
            if(!_wins.empty()) {
                _wins.front()->handleEvent(e);
            }
        }
    }

    void Workspace::handleEvent(Event e){
        if(e.type == Event::KEYBOARD){
            if((e.kcode.state.lWin or e.kcode.state.rWin) and !e.kcode.scanCode.release) {
                if(e.kcode.scanCode.code >= Keyboard::F1
                && e.kcode.scanCode.code <= Keyboard::F10 ){
                    uint workspaceNum= (e.kcode.scanCode.code - Keyboard::F1 +1) %10;
                    if(e.kcode.state.lShift or e.kcode.state.rShift) {
                        Workspace& currentWorkspace = _elems[active];
                        Workspace& targetWorkspace = _elems[workspaceNum];
                        if(!currentWorkspace._wins.empty()) {
                            Window* win = currentWorkspace._wins.front();
                            currentWorkspace._wins.pop_front();
                            targetWorkspace._wins.push_front(win);
                            win->_ws = workspaceNum;
                        }
                    } else {
                        Workspace::active = workspaceNum;
                    }
                    return;
                }
                if(e.kcode.scanCode.code == Keyboard::SPACE) {
                    _currentKeymap = (_currentKeymap+1)%input::NB_KEYMAPS;
                    kbd.setKeymap(input::keymaps[_currentKeymap]);
                    return;
                }
            }
        }
        get(Workspace::active).handleEventOnMe(e);
    }

    void Workspace::addWin(Window* win){
        _wins.push_front(win);
        win->_ws = _number;
    }
};
