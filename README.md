# ENS_sysres
Project for the "Systèmes et réseaux" course at ENS Ulm :

Development of a simple multitask 64 bits operating system named **Super OS**

## Building

You only need a x86_64 version of gcc to build, You can change the varaible CC and CXX in the Makefile if needed.

You can build with `make`

## Testing

You need to have 64 bits QEMU or Bochs installed.

There is two way of testing :

- CD booting : `make run` for Bochs or `make runqemu` for QEMU : depending 
on the code of kinit in src/kmain.cpp, 
the OS may try to use the hard drive and crash.
But for quick testing this is fine
- Hard Drive booting :

To build the virtual hard disk we currently use loopback devices on linux and thus we need superuser right. 
You may read the Makefile to be sure there are no security issues. 
The command is `make builddisk`. Some command like `mkfs` will be needed.

### QEMU

Once the disk is build you can test on QEMU with `make runqemud` if you want tho allow write on disk and `make runqemus` if not

If both case you can run `make connect` in an other terminal to open GDB on the OS and debug.

There is also a unit test system that you can use with `make unittest` one the Hard Drive is generated.

### Bochs

You can do the same with `make rund` and `make runs` with Bochs



## Documentation

If you have doxygen installed, You can run`make doc` to build the documentation.

