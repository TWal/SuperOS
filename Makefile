.PHONY: out

AS = as
CC = gcc
CXX = g++

SRCDIR = src
OUTDIR = out

ASFLAGS = --32
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
CXXFLAGS = $(CFLAGS)
LDFLAGS = -T $(SRCDIR)/link.ld -melf_i386

SRCASM = $(wildcard $(SRCDIR)/*.s)
SRCC = $(wildcard $(SRCDIR)/*.c)
SRCCXX = $(wildcard $(SRCDIR)/*.cpp)

OBJ = $(patsubst $(SRCDIR)/%.s, $(OUTDIR)/%.o, $(SRCASM)) \
      $(patsubst $(SRCDIR)/%.c, $(OUTDIR)/%.o, $(SRCC))   \
      $(patsubst $(SRCDIR)/%.cpp, $(OUTDIR)/%.o, $(SRCCXX))

all: kernel.elf

kernel.elf: $(OBJ)
	ld $(LDFLAGS) $(OBJ) -o kernel.elf

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

$(OUTDIR)/%.o: $(SRCDIR)/%.s out
	$(AS) $(ASFLAGS) $< -o $@

$(OUTDIR)/%.o: $(SRCDIR)/%.c out
	$(CC) $(CFLAGS)  $< -o $@

$(OUTDIR)/%.o: $(SRCDIR)/%.cpp out
	$(CXX) $(CXXFLAGS)  $< -o $@

out:
	mkdir -p $(OUTDIR)

clean:
	rm $(OUTDIR)/*
