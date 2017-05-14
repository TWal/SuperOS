#include <errno.h>
#include "Process.h"
#include "Thread.h"
#include "../Memory/Paging.h"
#include "../Memory/PhysicalMemoryAllocator.h"
#include "../User/Elf64.h"
#include "../User/Syscall.h"
#include "Scheduler.h"
#include "../log.h"

using namespace std;


/* ____                                 ____
  |  _ \ _ __ ___   ___ ___  ___ ___   / ___|_ __ ___  _   _ _ __
  | |_) | '__/ _ \ / __/ _ \/ __/ __| | |  _| '__/ _ \| | | | '_ \
  |  __/| | | (_) | (_|  __/\__ \__ \ | |_| | | | (_) | |_| | |_) |
  |_|   |_|  \___/ \___\___||___/___/  \____|_|  \___/ \__,_| .__/
                                                            |_|
*/
ProcessGroup::ProcessGroup(u16 gid) : _gid(gid){
    schedul.addG(_gid,this);
}

ProcessGroup::~ProcessGroup(){
    schedul.freeG(_gid);
}

void ProcessGroup::remProcess(Process* pro){
    assert(_processes.count(pro));
    _processes.erase(pro);
    if(_processes.empty()) delete this;
}

/* ____                                _                    _ _
  |  _ \ _ __ ___   ___ ___  ___ ___  | |    ___   __ _  __| (_)_ __   __ _
  | |_) | '__/ _ \ / __/ _ \/ __/ __| | |   / _ \ / _` |/ _` | | '_ \ / _` |
  |  __/| | | (_) | (_|  __/\__ \__ \ | |__| (_) | (_| | (_| | | | | | (_| |
  |_|   |_|  \___/ \___\___||___/___/ |_____\___/ \__,_|\__,_|_|_| |_|\__, |
                                                                      |___/
*/
void* const loadedProcess = (void*)-0x100000000ll; // -4G
const uptr stackStart =  0x8000000000ull; // 512 G

Thread* Process::loadFromBytes(Bytes* file){
    std::vector<uptr> allocPages;
    //paging.switchUser(_userPDP);
    _usermem.activate();

    // get last addr used;
    void* lastUsedAddr = 0;
    auto usedAddr = [&lastUsedAddr](void* addr){
        if(u64(addr) > (u64)lastUsedAddr) lastUsedAddr = addr;
    };

    // smart reader to allow reading beyond end
    auto readaddr = [file](uptr addr, void* buffer,size_t size){
        file->readaddr(addr,buffer,max((size_t)0,min(size,file->getSize() - addr)));
    };

    //loading first page of file
    uptr firstblock = physmemalloc.alloc();
    allocPages.push_back(firstblock);
    paging.createMapping(firstblock,loadedProcess);
    readaddr(0,loadedProcess,0x1000);

    // Parsing executable
    Elf64::Elf64 elf((char*)loadedProcess,file->getSize());
    size_t toLoad = elf.toLoadSize();
    toLoad += 0x1000 -1;
    toLoad /= 0x1000;

    //loading enough to have all information in kernel Space
    for(size_t i = 1 ; i < toLoad ; ++i){
        uptr block = physmemalloc.alloc();
        allocPages.push_back(block);
        paging.createMapping(block,(char*)loadedProcess + i * 0x1000);
        readaddr(i*0x1000,(char*)loadedProcess+i*0x1000,0x1000);
    }

    //Creating mapping for each program header
    for(int i = 0; i < elf.phnum ; ++ i){
        auto ph = elf.getProgramHeader(i);
        //printf("%d, t : %d, off : %p, virt : %llx, size :%d %d\n",i,ph.type,ph.getData(),ph.vaddr,ph.filesz,ph.memsz);

        if(ph.type == Elf64::PT_LOAD){
            debug(Proc,"Loading Program Header at %p",ph.vaddr);
            // if this program header is to be loaded
            assert(!(ph.vaddr & (0x1000 -1))); // assert ph.vaddr aligned on 4K

            size_t offset = ph.offset;
            debug(Proc,"Offset in file is %llu",offset);
            if(offset % 0x1000 == 0){
                // the mapping is 4K aligned : better case
                size_t po = offset / 0x1000;
                size_t nbfPages = (ph.filesz + 0x1000-1) / 0x1000;
                size_t nbvPages = (ph.memsz + 0x1000-1) / 0x1000;
                debug(Proc,"file pages : %llu ans virtual pages %llu",nbfPages,nbvPages);
                //mapping page by page
                for(size_t i = 0 ; i < nbvPages ; ++i){
                    if(i>= nbfPages){
                        paging.createMapping(physmemalloc.alloc()
                                             ,(char*)ph.vaddr + i * 0x1000);
                        __builtin_memset((char*)ph.vaddr + i * 0x1000,0,0x1000);
                        debug(Proc,"Mapping %p to nothing",(char*)ph.vaddr + i * 0x1000);
                        continue;
                    }
                    uptr phyblock;
                    if(po + i < toLoad){
                        phyblock = allocPages[po + i];
                        allocPages[po + i] = 0;
                    }
                    else phyblock = physmemalloc.alloc();
                    assert(phyblock);
                    paging.createMapping(phyblock,(char*)ph.vaddr + i * 0x1000);
                    if( po + i >= toLoad){
                        readaddr(offset + i * 0x1000,(char*)ph.vaddr + i * 0x1000,0x1000);
                    }
                }
                usedAddr((void*)(ph.vaddr + nbvPages * 0x1000));
            }
            else bsod("Loading unaligned elf64 is not supported for now");
        }
    }
    //printf("loading stack");
    //WAIT(1000000000000);

    // allocating stack
    for(size_t i = 1 ; i <= FIXED_SIZE_STACK ; ++i){
        uptr block = physmemalloc.alloc();
        paging.createMapping(block,(char*)stackStart - i * 0x1000);
    }
    //TODO preparing Heap
    debug(Proc,"Initializing heap with %p",lastUsedAddr);
    _heap.init(lastUsedAddr);

    debug(Proc,"User mem after loading :");
    _usermem.DumpTree();

    //printf("Creating thread\n");
    //the file is now ready to be executed. Creating main thread
    auto t =  new Thread(_pid,elf.entry,this);
    t->context.rsp = stackStart;
    return t;
}




/* ____
  |  _ \ _ __ ___   ___ ___  ___ ___
  | |_) | '__/ _ \ / __/ _ \/ __/ __|
  |  __/| | | (_) | (_|  __/\__ \__ \
  |_|   |_|  \___/ \___\___||___/___/
*/
Process::Process(u32 pid, ProcessGroup* pg, std::vector<FileDescriptor> fds) :
    _pid(pid), _gid(pg->getGid()), _parent(nullptr), _terminated(false), _mainTerminated(false),
    _returnCode(0), _fds(fds){
    schedul.addP(_pid,this);
    pg->addProcess(this);
    info(Proc,"Creating Process %p with pid %d",this,pid);
}


Process::Process(Process& other, Thread* toth, u16 pid):
    Process(pid,schedul.getG(other.getGid()),other._fds){

    _usermem = other._usermem;
    _heap = other._heap;
    // Creating the new thread and coping the context of the calling thread
    // the new process has only one thread : the copy of the thread that called fork
    Thread* tcopy = new Thread(pid,0,this);
    tcopy->context = toth->context;

    // we are a new child of other.
    _parent = &other;
    other.addChild(this);

    // child process has 0 as a return.
    tcopy->context.rax = 0;
}



void Process::addThread(Thread* thread){
    _threads.insert(thread);
}

// Only called when its parent process has wait him.
Process::~Process(){
    debug(Proc,"Deleting Process %p",this);
    assert(_terminated); // check the process is effectively a zombie
    schedul.freeP(_pid);
    for(auto p : _sons){
        p->orphan();
    }
    schedul.getG(_gid)->remProcess(this);
    _parent->_sons.erase(this); // TODO Does the parent does it already ?
}

void Process::terminate(u64 returnCode){
    _returnCode = returnCode;
    _terminated = true;
    _usermem.clear();
    for(auto th : _threads){
        delete th;
    }
    _threads.clear();
    if(_pid == 1){
        info(Proc,"init died with code %lld",returnCode);
        kend(); // shutdown.
    }
    free(); // free the waiting ressources.
    /*for(auto fd : _fds){
        fd->drop();
        }*/
    // this process is now a zombie
}


void Process::mainTerm(Thread* main, u64 returnCode){
    assert(main->getTid() == _pid);
    _mainTerminated = true;
    _returnCode = returnCode;
    remThread(main);
}
void Process::remThread(Thread* thread){
    _threads.erase(thread);
    if(_threads.empty()){
        assert(_mainTerminated);
        terminate(_returnCode);
    }
}

void Process::prepare(){
    _usermem.activate();
}
void Process::addChild(Process* pro){
    _sons.insert(pro);
}

void Process::orphan(){
    Process* init = schedul.getP(1);
    _parent = init;
    init->addChild(this);
}
