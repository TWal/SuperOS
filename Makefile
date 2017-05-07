.PHONY: all run rund runs runqemu runqemud runqemus load partition unload mount umount lm ulm grubinst mvtoimg builddisk updatedsk clean mrproper dasm count dofsck fsck doc

AS = as
CC = gcc
CXX = g++

SRCDIR = src
SRC32DIR = src32
OUTDIR = out
OUT32DIR = out/loader
DEPDIR = .dep
DEP32DIR = .dep/loader
LIBC = libc
LIBCXX = libc++

LIB32GCC = /usr/lib/gcc/x86_64-*linux-gnu/6.*/32
LIBGCC = /usr/lib/gcc/x86_64-*linux-gnu/6.*

OPTILVL = -Og -g -mno-sse

ASFLAGS =
AS32FLAGS = --32
CBASEFLAGS = -nostdlib -ffreestanding -fno-stack-protector -Wall -Wextra \
				 -Wno-packed-bitfield-compat -fno-builtin -mno-red-zone \
				 $(OPTILVL)
C32FLAGS = $(CBASEFLAGS) -m32 -DSUP_OS_LOADER
CFLAGS = $(CBASEFLAGS) -isystem $(LIBC) \
				   -isystem $(LIBCXX)\
				   -DSUP_OS_KERNEL \
					 -mcmodel=kernel # 64 bit high-half kernel

CNKFLAGS = -nostdlib -ffreestanding -fno-stack-protector -Wall -Wextra \
					 -fno-builtin $(OPTILVL)


CXXBASEFLAGS = -fno-rtti -fno-exceptions -std=c++14
CXX32FLAGS = $(C32FLAGS) $(CXXBASEFLAGS)
CXXFLAGS = $(CFLAGS) $(CXXBASEFLAGS)
CXXNKFLAGS = $(CNKFLAGS) $(CXXBASEFLAGS)

LDFLAGS =  -nostdlib -Wl,--build-id=none
LD32FLAGS = $(LDFLAGS) -m32 -T $(SRC32DIR)/link.ld -Wl,-melf_i386
LIBS32 = -L $(LIB32GCC) -lgcc
LD64FLAGS = $(LDFLAGS) -T $(SRCDIR)/link.ld -Wl,-melf_x86_64
LIBS64 = -L. -L $(LIBGCC) -lc++ -lgcc -lk

SRC32ASM = $(wildcard $(SRC32DIR)/*.s)
SRC32CXX = $(wildcard $(SRC32DIR)/*.cpp)

SRCCONTENT = $(shell find $(SRCDIR) -type f)

#SRCASM = $(SRCDIR)/Interrupts/Interrupt.s
#SRCCXX = $(SRCDIR)/kmain.cpp $(SRCDIR)/IO/FrameBuffer.cpp $(SRCDIR)/utility.cpp \
				 $(SRCDIR)/Interrupts/Interrupt.cpp $(SRCDIR)/Interrupts/Pic.cpp \
				 $(SRCDIR)/Memory/PhysicalMemoryAllocator.cpp \
				 $(SRCDIR)/Memory/Paging.cpp \
				 $(SRCDIR)/Memory/Heap.cpp

SRCASM = $(filter %.s,$(SRCCONTENT))
SRCC = $(filter %.c,$(SRCCONTENT))
SRCCXX = $(filter %.cpp,$(SRCCONTENT))

DEPF = $(shell find $(DEPDIR) -type f)

OBJ32 = $(patsubst $(SRC32DIR)/%, $(OUT32DIR)/%.o, $(SRC32ASM)) \
      $(patsubst $(SRC32DIR)/%, $(OUT32DIR)/%.o, $(SRC32CXX)) \

OBJ = $(patsubst $(SRCDIR)/%, $(OUTDIR)/%.o, $(SRCASM)) \
      $(patsubst $(SRCDIR)/%, $(OUTDIR)/%.o, $(SRCC))   \
      $(patsubst $(SRCDIR)/%, $(OUTDIR)/%.o, $(SRCCXX)) \
      $(OUTDIR)/InterruptInt.o


LOOPDEV = /dev/loop0
MNTPATH = /mnt/test# must be absolute and not relative

LIBCSRCASM = $(wildcard $(LIBC)/src/*.s)
LIBCSRC = $(wildcard $(LIBC)/src/*.c)
LIBCSRCXX = $(wildcard $(LIBC)/src/*.cpp)
LIBCOBJ = $(patsubst $(LIBC)/src/%.c,$(OUTDIR)/$(LIBC)/%.o,$(LIBCSRC)) \
	$(patsubst $(LIBC)/src/%.cpp,$(OUTDIR)/$(LIBC)/%.o,$(LIBCSRCXX)) \
	$(patsubst $(LIBC)/src/%.s,$(OUTDIR)/$(LIBC)/%.o,$(LIBCSRCASM))

LIBKOBJ = $(patsubst $(LIBC)/src/%.c,$(OUTDIR)/libk/%.o,$(LIBCSRC)) \
	$(patsubst $(LIBC)/src/%.cpp,$(OUTDIR)/libk/%.o,$(LIBCSRCXX)) \
	$(patsubst $(LIBC)/src/%.s,$(OUTDIR)/libk/%.o,$(LIBCSRCASM))

LIBCH = $(wildcard $(LIBC)/*.h)

LIBCXXSRC = $(wildcard $(LIBCXX)/src/*.cpp)
LIBCXXOBJ = $(patsubst $(LIBCXX)/src/%.cpp,$(OUTDIR)/$(LIBCXX)/%.o,$(LIBCXXSRC))
LIBCXXH = $(wildcard $(LIBCXX)/*) $(wildcard $(LIBCXX)/include/*.h)

FSTYPE = ext2
MKFSARGS = -b 2048
FSCKARGS = -f -n
#FSTYPE = fat
#MKFSARGS = -F 32
#FSCKARGS = -n

all: loader.elf kernel.elf crt0.o init

os.iso: all
	cp loader.elf iso/boot/loader.elf
	cp kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o os.iso iso

run: os.iso
	bochs -q -f bochsrc.txt

rund: updatedisk
	bochs -f bochsrcd.txt -q

runs: updatedisk
	cp disk.img disko.img
	bochs -f bochsrcd.txt -q
	mv disk.img diskout.img
	mv disko.img disk.img


runqemu: os.iso
	qemu-system-x86_64 -boot d -cdrom os.iso -m 512 -s -serial file:logqemu.txt

runqemud: updatedisk
	qemu-system-x86_64 -boot c -drive format=raw,file=disk.img -m 512 -s -serial file:logqemu.txt

runqemus: updatedisk
	cp disk.img disko.img
	qemu-system-x86_64 -boot c -drive format=raw,file=disk.img -m 512 -s -serial file:logqemu.txt
	mv disk.img diskout.img
	mv disko.img disk.img

runqemuk: updatedisk
	cp disk.img disko.img
	qemu-system-x86_64 -boot c -drive format=raw,file=disk.img -m 512 -s -serial stdio
	mv disk.img diskout.img
	mv disko.img disk.img

runqemuu: updatedisk
	cp disk.img disko.img
	qemu-system-x86_64 -boot c -drive format=raw,file=disk.img -m 512 -s -nographic
	mv disko.img disk.img

connect : all
	gdb -ex "set arch i386:x86-64" -ex "symbol-file kernel.elf" -ex "target remote localhost:1234"



#-------------------------------kernel rules------------------------------------

kernel.elf: $(OBJ) $(SRCDIR)/link.ld libk.a libc++.a
	@echo Linking kernel
	@g++ $(LD64FLAGS) $(OBJ) -o kernel.elf $(LIBS64) -Xlinker --print-map > ld_mapping_full
	@cat ld_mapping_full | sed -e '1,/text/d' -e '/rodata/,$$d' > ld_mapping
	@echo --------------------------kernel built.------------------------------
	@echo
	@echo

$(OUTDIR)/%.s.o: $(SRCDIR)/%.s libk.a libc++.a Makefile
	@mkdir -p `dirname $@`
	@$(AS) $(ASFLAGS) $< -o $@
	@echo Assembling kernel file : $<

$(OUTDIR)/%.c.o: $(SRCDIR)/%.c libk.a libc++.a Makefile
	@mkdir -p `dirname $@`
	@mkdir -p `dirname $(patsubst $(SRCDIR)/%.c, $(DEPDIR)/%.c.d, $<)`
	@echo Compiling kernel file : $<
	@$(CC) $(CFLAGS) -MMD -MT '$@' -MF	$(patsubst $(SRCDIR)/%.c, $(DEPDIR)/%.c.d, $<) -c $< -o $@

$(OUTDIR)/%.cpp.o: $(SRCDIR)/%.cpp libk.a libc++.a Makefile
	@mkdir -p `dirname $@`
	@mkdir -p `dirname $(patsubst $(SRCDIR)/%.cpp, $(DEPDIR)/%.cpp.d, $<)`
	@echo Compiling kernel file : $<
	@$(CXX) $(CXXFLAGS) -MMD -MT '$@' -MF $(patsubst $(SRCDIR)/%.cpp, $(DEPDIR)/%.cpp.d, $<) -c $< -o $@


$(OUTDIR)/InterruptInt.o : $(SRCDIR)/Interrupts/Interrupt.py Makefile
	@python3 $(SRCDIR)/Interrupts/Interrupt.py > $(OUTDIR)/InterruptInt.s
	@$(AS) $(ASFLAGS) $(OUTDIR)/InterruptInt.s -o $(OUTDIR)/InterruptInt.o
	@echo Generating and assembling from kernel file : $<



#----------------------------------loader rules---------------------------------

loader.elf: $(OBJ32) $(SRC32DIR)/link.ld
	@echo Linking loader
	@g++ $(LD32FLAGS) $(OBJ32) -o loader.elf $(LIBS32) -Xlinker --print-map > ld_mapping_loader_full
	@cat ld_mapping_loader_full | sed -e '1,/text/d' -e '/rodata/,$$d' > ld_mapping_loader
	@echo --------------------------loader built.------------------------------
	@echo
	@echo



$(OUT32DIR)/%.s.o: $(SRC32DIR)/%.s Makefile
	@mkdir -p $(OUT32DIR)
	@$(AS) $(AS32FLAGS) $< -o $@
	@echo Assembling loader file : $<

$(OUT32DIR)/%.cpp.o: $(SRC32DIR)/%.cpp Makefile
	@mkdir -p $(OUT32DIR)
	@mkdir -p `dirname $(patsubst $(SRC32DIR)/%.cpp, $(DEP32DIR)/%.cpp.d, $<)`
	@echo Compiling loader file : $<
	@$(CXX) $(CXX32FLAGS) -MMD -MT '$@' -MF$(patsubst $(SRC32DIR)/%.cpp, $(DEP32DIR)/%.cpp.d, $<) -c $< -o $@


#----------------------------------libc rules-----------------------------------

$(OUTDIR)/libc/%.o: $(LIBC)/src/%.s Makefile
	@mkdir -p $(OUTDIR)/libc
	@echo Assembling libc file : $<
	@$(AS) $(ASFLAGS) $< -o $@

$(OUTDIR)/$(LIBC)/%.o: $(LIBC)/src/%.c Makefile
	@mkdir -p $(OUTDIR)/libc
	@mkdir -p $(DEPDIR)/libc
	@echo Compiling libc file : $<
	@$(CC) $(CNKFLAGS) -MD -MT '$@' -MF	$(patsubst $(LIBC)/src/%.c, $(DEPDIR)/$(LIBC)/%.c.d, $<) -c $< -o $@

$(OUTDIR)/$(LIBC)/%.o: $(LIBC)/src/%.cpp Makefile
	@mkdir -p $(OUTDIR)/libc
	@mkdir -p $(DEPDIR)/libc
	@echo Compiling libc file : $<
	@$(CXX) $(CXXNKFLAGS) -MD -MT '$@' -MF	$(patsubst $(LIBC)/src/%.cpp, $(DEPDIR)/$(LIBC)/%.cpp.d, $<) -c $< -o $@

libc.a: $(LIBCOBJ) $(LIBCH) Makefile

	ar rcs libc.a $(LIBCOBJ)
	@echo --------------------------libc built.------------------------------
	@echo
	@echo

#----------------------------------libk rules-----------------------------------

$(OUTDIR)/libk/%.o: $(LIBC)/src/%.s Makefile
	@mkdir -p $(OUTDIR)/libk
	@echo Assembling libk file : $<
	@$(AS) $(ASFLAGS) $< -o $@

$(OUTDIR)/libk/%.o: $(LIBC)/src/%.c Makefile
	@mkdir -p $(OUTDIR)/libk
	@mkdir -p $(DEPDIR)/libk
	@echo Compiling libk file : $<
	@$(CC) $(CFLAGS) -MD -MT '$@' -MF	$(patsubst $(LIBC)/src/%.c, $(DEPDIR)/libk/%.c.d, $<) -c $< -o $@

$(OUTDIR)/libk/%.o: $(LIBC)/src/%.cpp Makefile
	@mkdir -p $(OUTDIR)/libk
	@mkdir -p $(DEPDIR)/libk
	@echo Compiling libk file : $<
	@$(CXX) $(CXXFLAGS) -MD -MT '$@' -MF	$(patsubst $(LIBC)/src/%.cpp, $(DEPDIR)/libk/%.cpp.d, $<) -c $< -o $@

libk.a: $(LIBKOBJ) $(LIBCH) Makefile

	ar rcs libk.a $(LIBKOBJ)
	@echo --------------------------libk built.------------------------------
	@echo
	@echo


#----------------------------------libc++ rules---------------------------------

$(OUTDIR)/$(LIBCXX)/%.o: $(LIBCXX)/src/%.cpp $(LIBCH) Makefile
	@mkdir -p $(OUTDIR)/libc++
	@mkdir -p $(DEPDIR)/libc++
	@echo Compiling libcpp file : $<
	@$(CXX) $(CXXFLAGS) -MD -MT '$@' -MF	$(patsubst $(LIBCXX)/src/%.cpp, $(DEPDIR)/$(LIBCXX)/%.cpp.d, $<) -c $< -o $@

libc++.a: $(LIBCXXOBJ) $(LIBCXXH) $(LIBCH) Makefile
	ar rcs libc++.a $(LIBCXXOBJ)
	@echo --------------------------libc++ built.------------------------------
	@echo
	@echo



#---------------------------------Unit tests-------------------------------

unittest:
	@./unitTests.sh


buildunit: $(OBJ) $(SRCDIR)/link.ld libk.a libc++.a
	@$(CXX) $(CXXFLAGS) -DUNITTEST -c $(SRCDIR)/kmain.cpp -o $(OUTDIR)/kmain.cpp.o
	@echo Linking kernel for unit test
	@g++ $(LD64FLAGS) $(OBJ) $(OUTDIR)/unittest.o -o kernel.elf $(LIBS64) -Xlinker --print-map > ld_mapping_full
	@cat ld_mapping_full | sed -e '1,/text/d' -e '/rodata/,$$d' > ld_mapping
	@echo ----------------------unittest kernel built.------------------------------
	@echo
	@echo

getCompileLine:
	@echo $(CXX) $(CXXFLAGS)

#---------------------------------User mode------------------------------------

$(OUTDIR)/start/crt0.s.o: user/start/crt0.s
	mkdir -p `dirname $@`
	$(AS) $(ASFLAGS) $< -o $@

$(OUTDIR)/start/crt0.c.o:  user/start/crt0.c
	mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c $< -o $@

crt0.o: $(OUTDIR)/start/crt0.s.o $(OUTDIR)/start/crt0.c.o
	ld -r $^ -o crt0.o

#---------------------------------Init Process------------------------------------

init: user/init/init.c libc.a crt0.o
	./gcc-supos $< -o $@

#---------------------------------Disk building------------------------------------
disk.img :
	dd if=/dev/zero of=disk.img bs=512 count=131072 #64 MiB
	#dd if=/dev/zero of=disk.img bs=512 count=2097152 # 1 GiB
	#dd if=/dev/zero of=disk.img bs=512 count=4194304 # 2 GiB

load: disk.img
	sudo losetup $(LOOPDEV) disk.img
	sudo partprobe $(LOOPDEV)

partition:
	#sudo echo "," | sudo sfdisk $(LOOPDEV)
	sudo echo -e ",32MiB\n,32MiB" | sudo sfdisk $(LOOPDEV)
	sudo partprobe $(LOOPDEV)
	sudo mkfs.$(FSTYPE) $(MKFSARGS) $(LOOPDEV)p1
	sudo mkfs.$(FSTYPE) $(MKFSARGS) $(LOOPDEV)p2

unload:
	sudo losetup -d $(LOOPDEV)
  #sudo partprobe "$(LOOPDEV)"

mount:
	sudo mkdir -p $(MNTPATH)
	sudo mount $(LOOPDEV)p1 $(MNTPATH)

umount:
	sudo umount $(MNTPATH)
	sudo rm -rf $(MNTPATH)

lm: load mount

ulm: umount unload


grubinst:
	sudo grub-install --root-directory=$(MNTPATH) --no-floppy --modules="normal part_msdos $(FSTYPE) multiboot" --target=i386-pc $(LOOPDEV)

mvtoimg: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	cp loader.elf iso/boot/loader.elf
	cp init iso/init
	sudo rsync -r iso/ $(MNTPATH)/

builddisk: load partition mount grubinst mvtoimg ulm

updatedisk: kernel.elf loader.elf crt0.o init lm mvtoimg ulm



cleanunit:
	rm -f logqemu_*.txt


clean: cleanunit
	rm -rf $(OUTDIR)
	rm -rf $(DEPDIR)
	rm -f disassembly
	rm -f disassemblyl
	rm -f ld_mapping
	rm -f ld_mapping_full
	rm -f ld_mapping_loader
	rm -f ld_mapping_loader_full
	rm -f os.iso
	rm -f kernel.elf
	rm -f loader.elf
	rm -f bochslog.txt
	rm -f libk.a
	rm -f libc.a
	rm -f libc++.a
	rm -f log.txt
	rm -f logqemu.txt
	rm -f crt0.o
	rm -f init
	rm -f diskout.img
	rm -f index.html


mrproper: clean
	rm -f disk.img
	rm -f iso/boot/kernel.elf
	rm -f iso/boot/loader.elf
	rm -rf doc

dasm:
	objdump -D -C kernel.elf > disassembly

dasml:
	objdump -D -C loader.elf > disassemblyl

count:
	cloc libc libc++ src src32 unitTests user -lang-no-ext="C/C++ Header"

dofsck:
	sudo fsck.$(FSTYPE) $(FSCKARGS) $(LOOPDEV)p1
	sudo fsck.$(FSTYPE) $(FSCKARGS) $(LOOPDEV)p2

fsck: load dofsck unload

doc:
	doxygen Doxyfile
	ln -sf doc/html/index.html index.html

include $(DEPF)

