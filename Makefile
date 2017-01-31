.PHONY: out

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
      $(patsubst $(SRCDIR)/%, $(OUTDIR)/%.o, $(SRCCXX))

all: kernel.elf

kernel.elf: $(OBJ)
	ld $(LDFLAGS) $(OBJ) -o kernel.elf --print-map | sed -e '1,/text/d' -e '/rodata/,$$d' > ld_mapping

os.iso: kernel.elf
	cp kernel.elf iso/boot/kernel.elf
	genisoimage -R                              \
				-b boot/grub/stage2_eltorito    \
				-no-emul-boot                   \
				-boot-load-size 4               \
				-A os                           \
				-input-charset utf8             \
				-quiet                          \
				-boot-info-table                \
				-o os.iso                       \
				iso

run: os.iso
	bochs -f bochsrc.txt -q

$(OUTDIR)/%.s.o: $(SRCDIR)/%.s out
	$(AS) $(ASFLAGS) $< -o $@

$(OUTDIR)/%.c.o: $(SRCDIR)/%.c out
	$(CC) $(CFLAGS)  $< -o $@

$(OUTDIR)/%.cpp.o: $(SRCDIR)/%.cpp out
	$(CXX) $(CXXFLAGS)  $< -o $@

out:
	mkdir -p $(OUTDIR)

clean:
	rm -rf $(OUTDIR)
	rm -f disassembly
	rm -f ld_mapping
	rm -f os.iso
	rm -f kernel.elf
	rm -f bochslog.txt

dasm:
	objdump -D -C kernel.elf > disassembly
