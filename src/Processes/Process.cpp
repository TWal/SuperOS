#include"Process.h"
#include"../Memory/Paging.h"
#include"../Memory/PhysicalMemoryAllocator.h"
#include"../User/Elf64.h"
#include "Scheduler.h"

ProcessGroup::ProcessGroup(u16 gid) : _gid(gid){
    schedul.addG(_gid,this);
}


void* const loadedProcess = (void*)-0x100000000ll; // -4G
const uptr stackStart =  0x8000000000ull; // 512 G


Process::Process(u32 pid, ProcessGroup* pg, std::vector<FileDescriptor*> fds) :
    _pid(pid), _gid(pg->getGid()), _parent(nullptr), _terminated(false), _mainTerminated(false),
    _returnCode(0), _fds(fds){
    schedul.addP(_pid,this);
    pg->addProcess(this);
}


Process::Process(const Process& other,Thread* toth, u16 pid): Process(pid,schedul.getG(other.getGid()),other._fds){

    _usermem = other._usermem;
    _heap = other._heap;
    // Creating the new thread and coping the context of the calling thread
    // the new process has only one thread : the copy of the thread that called fork
    Thread* tcopy = new Thread(pid,0,this);
    tcopy->context = toth->context;

    // child process has 0 as a return.
    tcopy->context.rax = 0;
}



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
            printf("loading at %p\n",ph.vaddr);
            // if this program header is to be loaded
            assert(!(ph.vaddr & (0x1000 -1))); // assert ph.vaddr aligned on 4K

            size_t offset = ph.offset;
            printf("offset is %llu",offset);
            if(offset % 0x1000 == 0){
                // the mapping is 4K aligned : better case
                size_t po = offset / 0x1000;
                size_t nbfPages = (ph.filesz + 0x1000-1) / 0x1000;
                size_t nbvPages = (ph.memsz + 0x1000-1) / 0x1000;
                printf("file pages : %llu ans virtual pages %llu\n",nbfPages,nbvPages);
                //mapping page by page
                for(size_t i = 0 ; i < nbvPages ; ++i){
                    if(i>= nbfPages){
                        paging.createMapping(physmemalloc.alloc()
                                             ,(char*)ph.vaddr + i * 0x1000);
                        __builtin_memset((char*)ph.vaddr + i * 0x1000,0,0x1000);
                        printf("Mapping %p to nothing",(char*)ph.vaddr + i * 0x1000);
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
    printf("Initializing heap with %p",lastUsedAddr);
    _heap.init(lastUsedAddr);

    printf("User mem after loading :");
    _usermem.DumpTree();

    //printf("Creating thread\n");
    //the file is now ready to be executed. Creating main thread
    return new Thread(_pid,elf.entry,this);
}






void Process::addThread(Thread* thread){
    _threads.insert(thread);
}

Process::~Process(){
    assert(_terminated); // check the process is effectively a zombie
    schedul.freeP(_pid);
    // TODO orphan children ...
}

void Process::terminate(u64 returnCode){
    _returnCode = returnCode;
    _terminated = true;
    _usermem.clear();
    //printf("middle termination\n");
    for(auto th : _threads){
        delete th;
    }
    /*for(auto fd : _fds){
        fd->drop();
        }*/
    // this process is now a zombie
}


void Process::mainTerm(Thread* main,u64 returnCode){
    assert(main->getTid() == _pid);
    _mainTerminated = true;
    _returnCode = returnCode;
    remThread(main);
}
void Process::remThread(Thread* thread){
    _threads.erase(thread);
    if(_threads.empty()){
        assert(_mainTerminated);
        _terminated = true;
        // send something like SIGCHILD
    }
}


void Process::prepare(){
    //paging.switchUser(_userPDP);
    _usermem.activate();
}


//------------------------------Thread-----------------------------


Thread::Thread(u16 tid, u64 rip,Process* process)
    : _tid(tid), _pid(process->getPid()), _gid(process->getGid()),_process(process), wr(nullptr){
    schedul.addT(_tid,this);
    process->addThread(this);
    context.rip = rip;
    context.rflags = 2 | (1 << 9);
    context.rsp = stackStart;
}
Thread::~Thread(){
    printf("Deletion of thread %d\n",_tid);
    schedul.freeT(_tid);
    //("thread %d deleted\n",_tid);
}

[[noreturn]] void Thread::run(){
    printf("Starting thread %d in %d at %p\n",_tid,_pid,context.rip);
    _process->prepare();
    printf("started\n");
    context.launch();
}
void Thread::terminate(u64 returnCode){
    if(isMain())_process->mainTerm(this,returnCode);
    else _process->remThread(this);
}
