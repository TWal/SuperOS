.PHONY: all run rund runqemu runqemud load partition unload mount umount lm ulm grubinst mvtoimg builddisk updatedsk clean mrproper dasm

AS = as
CC = gcc
CXX = g++

SRCDIR = src
OUTDIR = out
DEPDIR = .dep
LIBC = libc
LIBCXX = libc++

LIB32GCC = /usr/lib/gcc/x86_64-*linux-gnu/6.*/32

OPTILVL = -O3 -mno-sse

ASFLAGS = --32
CFLAGS = -m32  -nostdlib -ffreestanding -fno-stack-protector -Wall -Wextra \
				 -Wno-packed-bitfield-compat \
				 $(OPTILVL) \
				 -isystem $(LIBC) \
				 -isystem $(LIBCXX)\
				 -DSUP_OS_KERNEL

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS = -T $(SRCDIR)/link.ld -Wl,-melf_i386 -nostdlib -Wl,--build-id=none
LIBS = -L. -L $(LIB32GCC) -lc -lgcc -lc++


SRCCONTENT = $(shell find src -type f)

SRCASM = $(filter %.s,$(SRCCONTENT))
SRCC = $(filter %.c,$(SRCCONTENT))
SRCCXX = $(filter %.cpp,$(SRCCONTENT))

DEPF = $(wildcard $(DEPDIR)/*.d) $(wildcard $(DEPDIR)/$(LIBC)/*.d)

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


all: kernel.elf

kernel.elf: $(OBJ) $(SRCDIR)/link.ld libc.a libc++.a
	g++ $(LDFLAGS) $(OBJ) -o kernel.elf -Xlinker --print-map $(LIBS) > ld_mapping_full
	@cat ld_mapping_full | sed -e '1,/text/d' -e '/rodata/,$$d' > ld_mapping

os.iso: kernel.elf
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

$(OUTDIR)/%.c.o: $(SRCDIR)/%.c libc.a libc++.a Makefile
	@mkdir -p `dirname $@`
	@mkdir -p `dirname $(patsubst $(SRCDIR)/%.c, $(DEPDIR)/%.c.d, $<)`
	$(CC) $(CFLAGS)  -c $< -o $@
	@$(CC) -MM -MT '$@' -MF $(patsubst $(SRCDIR)/%.c, $(DEPDIR)/%.c.d, $<)  $<

$(OUTDIR)/%.cpp.o: $(SRCDIR)/%.cpp libc.a libc++.a Makefile
	@mkdir -p `dirname $@`
	@mkdir -p `dirname $(patsubst $(SRCDIR)/%.cpp, $(DEPDIR)/%.cpp.d, $<)`
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@$(CXX) -isystem libc -MM -MT '$@' -MF $(patsubst $(SRCDIR)/%.cpp, $(DEPDIR)/%.cpp.d, $<)  $<

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
	rm -f os.iso
	rm -f kernel.elf
	rm -f bochslog.txt
	rm -f libc.a

mrproper: clean
	rm -f disk.img
	rm -rf $(DEPDIR)
	rm -f iso/boot/kernel.elf

dasm:
	objdump -D -C kernel.elf > disassembly


include $(DEPF)

