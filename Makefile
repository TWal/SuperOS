.PHONY: run runqemu

AS = as
CC = gcc
CXX = g++

SRCDIR = src
OUTDIR = out

ASFLAGS = --32
CFLAGS = -m32 -nostdlib -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti -Wall -Wextra -Werror -c
CXXFLAGS = $(CFLAGS)
LDFLAGS = -T $(SRCDIR)/link.ld -melf_i386

SRCASM = $(wildcard $(SRCDIR)/*.s) 
SRCC = $(wildcard $(SRCDIR)/*.c)
SRCCXX = $(wildcard $(SRCDIR)/*.cpp)

OBJ = $(patsubst $(SRCDIR)/%, $(OUTDIR)/%.o, $(SRCASM)) \
      $(patsubst $(SRCDIR)/%, $(OUTDIR)/%.o, $(SRCC))   \
      $(patsubst $(SRCDIR)/%, $(OUTDIR)/%.o, $(SRCCXX)) \
      $(OUTDIR)/InterruptInt.o
      
      
LOOPDEV = /dev/loop0
MNTPATH = /mnt/test# must be absolute and not relative

all: kernel.elf

kernel.elf: $(OBJ)
	ld $(LDFLAGS) $(OBJ) -o kernel.elf --print-map > ld_mapping_full
	cat ld_mapping_full | sed -e '1,/text/d' -e '/rodata/,$$d' > ld_mapping	

os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o os.iso iso

run: os.iso
	bochs -f bochsrc.txt -q
	
rund: updatedisk
	bochs -f bochsrcd.txt -q
	
runqemu: os.iso
	qemu-system-x86_64 -boot d -cdrom os.iso -m 512

runqemud: os.iso
	qemu-system-x86_64 -boot c -drive format=raw,file=disk.img -m 512

$(OUTDIR)/%.s.o: $(SRCDIR)/%.s
	@mkdir -p $(OUTDIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OUTDIR)/%.c.o: $(SRCDIR)/%.c
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS)  $< -o $@

$(OUTDIR)/%.cpp.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OUTDIR)
	$(CXX) $(CXXFLAGS)  $< -o $@
	
disk.img :
	dd if=/dev/zero of=disk.img bs=512 count=131072
	
load: disk.img
	sudo losetup $(LOOPDEV) disk.img
	
partition: 
	sudo echo "," | sudo sfdisk $(LOOPDEV)
	sudo mkfs.fat -F 32 $(LOOPDEV)p1

unload: 
	sudo losetup -d $(LOOPDEV)
	

	
	
mount:
	sudo mkdir -p $(MNTPATH)
	sudo mount $(LOOPDEV)p1 $(MNTPATH)
	
umount:
	sudo umount $(MNTPATH)
	sudo rm -rf $(MNTPATH)
	
lm: load mount

ulm: umount unload
	
	
grubinst:
	sudo grub-install --root-directory=$(MNTPATH) --no-floppy --modules="normal part_msdos fat multiboot" $(LOOPDEV)
	
mvtoimg: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	sudo rsync -r iso/ $(MNTPATH)/
	
builddisk: load partition mount grubinst mvtoimg ulm

updatedisk: lm mvtoimg ulm
	




clean:
	rm -rf $(OUTDIR)
	rm -f disassembly
	rm -f ld_mapping
	rm -f os.iso
	rm -f kernel.elf
	rm -f bochslog.txt
	rm -f disk.img

dasm:
	objdump -D -C kernel.elf > disassembly

$(OUTDIR)/InterruptInt.o : $(SRCDIR)/Interrupt.py
	python3 $(SRCDIR)/Interrupt.py > $(OUTDIR)/InterruptInt.s
	$(AS) $(ASFLAGS) $(OUTDIR)/InterruptInt.s -o $(OUTDIR)/InterruptInt.o
	
