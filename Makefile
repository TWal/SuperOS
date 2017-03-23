.PHONY: all run rund runqemu runqemud load partition unload mount umount lm ulm grubinst mvtoimg builddisk updatedsk clean mrproper dasm

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

OPTILVL = -O0 -mno-sse

ASFLAGS =
AS32FLAGS = --32
CBASEFLAGS = -nostdlib -ffreestanding -fno-stack-protector -Wall -Wextra \
				 -Wno-packed-bitfield-compat \
				 $(OPTILVL)
C32FLAGS = $(CBASEFLAGS) -m32 -DSUP_OS_LOADER
CFLAGS = $(CBASEFLAGS) -isystem $(LIBC) \
				   -isystem $(LIBCXX)\
				   -DSUP_OS_KERNEL \
					 -mcmodel=kernel # 64 bit high-half kernel

CXXBASEFLAGS = -fno-exceptions -fno-rtti -std=c++14
CXX32FLAGS = $(C32FLAGS) $(CXXBASEFLAGS)
CXXFLAGS = $(CFLAGS) $(CXXBASEFLAGS)

LDFLAGS =  -nostdlib -Wl,--build-id=none
LD32FLAGS = $(LDFLAGS) -m32 -T $(SRC32DIR)/link.ld -Wl,-melf_i386
LIBS32 = -L $(LIB32GCC) -lgcc
LD64FLAGS = $(LDFLAGS) -T $(SRCDIR)/link.ld -Wl,-melf_x86_64
LIBS64 = -L. -L $(LIBGCC) -lc -lgcc -lc++

SRC32ASM = $(wildcard $(SRC32DIR)/*.s)
SRC32CXX = $(wildcard $(SRC32DIR)/*.cpp)

SRCCONTENT = $(shell find src -type f)

SRCCONTENT = $(shell find src -type f)

SRCASM = $(SRCDIR)/Interrupts/Interrupt.s
SRCCXX = $(SRCDIR)/kmain.cpp $(SRCDIR)/IO/FrameBuffer.cpp $(SRCDIR)/utility.cpp \
				 $(SRCDIR)/Interrupts/Interrupt.cpp $(SRCDIR)/Interrupts/Pic.cpp \
				 $(SRCDIR)/Memory/PhysicalMemoryAllocator.cpp \
				 $(SRCDIR)/Memory/Paging.cpp \
				 $(SRCDIR)/Memory/Heap.cpp

#SRCASM = $(filter %.s,$(SRCCONTENT))
#SRCC = $(filter %.c,$(SRCCONTENT))
#SRCCXX = $(filter %.cpp,$(SRCCONTENT))

DEPF = $(wildcard $(DEPDIR)/*.d) $(wildcard $(DEPDIR)/$(LIBC)/*.d)  $(wildcard $(DEPDIR)/$(LIBCXX)/*.d) $(wildcard $(DEP32DIR)/*.d)

OBJ32 = $(patsubst $(SRC32DIR)/%, $(OUT32DIR)/%.o, $(SRC32ASM)) \
      $(patsubst $(SRC32DIR)/%, $(OUT32DIR)/%.o, $(SRC32CXX)) \

OBJ = $(patsubst $(SRCDIR)/%, $(OUTDIR)/%.o, $(SRCASM)) \
      $(patsubst $(SRCDIR)/%, $(OUTDIR)/%.o, $(SRCC))   \
      $(patsubst $(SRCDIR)/%, $(OUTDIR)/%.o, $(SRCCXX)) \
      $(OUTDIR)/InterruptInt.o


LOOPDEV = /dev/loop0
MNTPATH = /mnt/test# must be absolute and not relative

LIBCSRC = $(wildcard $(LIBC)/src/*.c)
LIBCOBJ = $(patsubst $(LIBC)/src/%.c,$(OUTDIR)/$(LIBC)/%.o,$(LIBCSRC))
LIBCH = $(wildcard $(LIBC)/*.h)

LIBCXXSRC = $(wildcard $(LIBCXX)/src/*.cpp)
LIBCXXOBJ = $(patsubst $(LIBCXX)/src/%.cpp,$(OUTDIR)/$(LIBCXX)/%.o,$(LIBCXXSRC))
LIBCXXH = $(wildcard $(LIBCXX)/*) $(wildcard $(LIBCXX)/include/*.h)


all: loader.elf kernel.elf

loader.elf: $(OBJ32) $(SRC32DIR)/link.ld
	g++ $(LD32FLAGS) $(OBJ32) -o loader.elf $(LIBS32) -Xlinker --print-map > ld_mapping_loader_full
	@cat ld_mapping_loader_full | sed -e '1,/text/d' -e '/rodata/,$$d' > ld_mapping_loader


kernel.elf: $(OBJ) $(SRCDIR)/link.ld libc.a libc++.a
	g++ $(LD64FLAGS) $(OBJ) -o kernel.elf $(LIBS64) -Xlinker --print-map > ld_mapping_full
	@cat ld_mapping_full | sed -e '1,/text/d' -e '/rodata/,$$d' > ld_mapping

os.iso: all
	cp loader.elf iso/boot/loader.elf
	cp kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o os.iso iso

run: os.iso
	bochs -f bochsrc.txt -q

rund: updatedisk
	bochs -f bochsrcd.txt -q

runqemu: os.iso
	qemu-system-x86_64 -boot d -cdrom os.iso -m 512

runqemud: updatedisk
	qemu-system-x86_64 -boot c -drive format=raw,file=disk.img -m 512


$(OUTDIR)/%.s.o: $(SRCDIR)/%.s libc.a libc++.a Makefile
	@mkdir -p `dirname $@`
	$(AS) $(ASFLAGS) $< -o $@

$(OUT32DIR)/%.s.o: $(SRC32DIR)/%.s Makefile
	@mkdir -p $(OUT32DIR)
	$(AS) $(AS32FLAGS) $< -o $@

$(OUTDIR)/%.c.o: $(SRCDIR)/%.c libc.a libc++.a Makefile
	@mkdir -p `dirname $@`
	@mkdir -p `dirname $(patsubst $(SRCDIR)/%.c, $(DEPDIR)/%.c.d, $<)`
	$(CC) $(CFLAGS)  -c $< -o $@
	@$(CC) -MM -MT '$@' -MF $(patsubst $(SRCDIR)/%.c, $(DEPDIR)/%.c.d, $<)  $<

$(OUTDIR)/%.cpp.o: $(SRCDIR)/%.cpp libc.a libc++.a Makefile
	@mkdir -p `dirname $@`
	@mkdir -p `dirname $(patsubst $(SRCDIR)/%.cpp, $(DEPDIR)/%.cpp.d, $<)`
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@$(CXX) -MM -MT '$@' -MF $(patsubst $(SRCDIR)/%.cpp, $(DEPDIR)/%.cpp.d, $<)  $<

$(OUT32DIR)/%.cpp.o: $(SRC32DIR)/%.cpp Makefile
	@mkdir -p $(OUT32DIR)
	@mkdir -p `dirname $(patsubst $(SRC32DIR)/%.cpp, $(DEP32DIR)/%.cpp.d, $<)`
	$(CXX) $(CXX32FLAGS) -c $< -o $@
	@$(CXX) -MM -MT '$@' -MF $(patsubst $(SRC32DIR)/%.cpp, $(DEP32DIR)/%.cpp.d, $<)  $<

$(OUTDIR)/InterruptInt.o : $(SRCDIR)/Interrupts/Interrupt.py Makefile
	python3 $(SRCDIR)/Interrupts/Interrupt.py > $(OUTDIR)/InterruptInt.s
	$(AS) $(ASFLAGS) $(OUTDIR)/InterruptInt.s -o $(OUTDIR)/InterruptInt.o




$(OUTDIR)/$(LIBC)/%.o: $(LIBC)/src/%.c Makefile
	@mkdir -p $(OUTDIR)/libc
	@mkdir -p $(DEPDIR)/libc
	$(CC) $(CFLAGS)  -c $< -o $@
	@$(CC) -MM -MT '$@' -MF $(patsubst $(LIBC)/src/%.c, $(DEPDIR)/$(LIBC)/%.c.d, $<)  $<

libc.a: $(LIBCOBJ) $(LIBCH) Makefile
	ar rcs libc.a $(LIBCOBJ)


$(OUTDIR)/$(LIBCXX)/%.o: $(LIBCXX)/src/%.cpp libc.a Makefile
	@mkdir -p $(OUTDIR)/libc++
	@mkdir -p $(DEPDIR)/libc++
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@$(CXX) -MM -MT '$@' -MF $(patsubst $(LIBCXX)/src/%.cpp, $(DEPDIR)/$(LIBCXX)/%.cpp.d, $<)  $<

libc++.a: $(LIBCXXOBJ) $(LIBCXXH) libc.a Makefile
	ar rcs libc++.a $(LIBCXXOBJ)







disk.img :
	dd if=/dev/zero of=disk.img bs=512 count=131072

load: disk.img
	sudo losetup $(LOOPDEV) disk.img
	sudo partprobe $(LOOPDEV)

partition:
	sudo echo "," | sudo sfdisk $(LOOPDEV)
	sudo partprobe $(LOOPDEV)
	sudo mkfs.fat -F 32 $(LOOPDEV)p1

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
	sudo grub-install --root-directory=$(MNTPATH) --no-floppy --modules="normal part_msdos fat multiboot" --target=i386-pc $(LOOPDEV)

mvtoimg: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	sudo rsync -r iso/ $(MNTPATH)/

builddisk: load partition mount grubinst mvtoimg ulm

updatedisk: kernel.elf lm mvtoimg ulm





clean:
	rm -rf $(OUTDIR)
	rm -f disassembly
	rm -f ld_mapping
	rm -f ld_mapping_full
	rm -f ld_mapping_loader
	rm -f ld_mapping_full_loader
	rm -f os.iso
	rm -f kernel.elf
	rm -f loader.elf
	rm -f bochslog.txt
	rm -f libc.a
	rm -f libc++.a

mrproper: clean
	rm -f disk.img
	rm -rf $(DEPDIR)
	rm -f iso/boot/kernel.elf

dasm:
	objdump -D -C kernel.elf > disassembly

dasml:
	objdump -D -C loader.elf > disassemblyl

include $(DEPF)

