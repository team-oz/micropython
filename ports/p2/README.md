# The Propeller 2 native port of MicroPython

This MicroPython port now runs natively on the Propeller 2 MCU.

It should build under common OS hosts including Linux, Cygwin, MSYS, Darwin (OS X).

## Prerequisites

This port requires some tools to be installed and available in the working path before an image can be built.

### Propeller GCC Compiler

You will require propeller-elf-gcc and its include files.

This is a GCC Compiler for the Propeller 1 chip, the assembly code generated will be translated for the P2.

This build was tested with GCC version 4.6.1

You may find information here regarding this tool:
https://github.com/parallaxinc/propgcc

Note: Under MAC OS X (Yosemite) this compiler can sometimes randomly segfault with an internal compiler error.  If this happens, just re-invoke make to continue where the build left off.

### P2GCC

These tools translate the compiled P1 code into P2 code and then assemble and link.  This tool and library is available as a submodule of the P2 port and should be automatically included the first time make is invoked if it is not already installed.  

To manually install the p2gcc submodule prior to running make, you can issue this (once):

    $ git submodule update --init lib/p2gcc

Or you can use make itself to do this step (once) prior to building the MicroPython binary:

    $ make submodules

Attempting to make the image without P2GCC submodule installed can result in an error the first time make is called due to not having all files present when rules are determined, however this can be fixed by issuing a second make.

### LOADP2 (optional)

An older version of this download utility is included with the P2GCC library, but the newer version works better and offers more capabilities such as supporting higher baud rates and providing automatic port detection for MAC platforms.  In time this newer tool may be added into the MicroPython build as another submodule but in the meantime if required it is available on GitHub for separate installation here:

https://github.com/totalspectrum/loadp2

You may also use PropTool to download and/or flash the binary independently if desired, or just boot the image directly from SPI flash if you have installed it there after building.

### Other tools used in the P2 Makefile

git, python, perl, sed, sh, stat, gcc (native C compiler on host OS, used to build P2GCC tools)


## Building and running MicroPython on a P2

By default when running make with no targets the port will be built for the P2 machine when make is invoved from the ports/p2 folder:

    $ make

### Target image

The default target binary generated is:

    build/python.bin

### Verbose output

To enable verbose build output include the V=1 setting when issuing make:

    $ make V=1

### Downloading the binary directly to the Propeller 2 MCU

To download and run the executable image and get a basic working REPL you can do this, where PORT defines your OS serial port connected to the P2 device, e.g. something like /dev/cu.usbserial-XYZABCDE:

    $ make PORT=<port> run

This will attempt to use the loadp2 tool shipped with P2GCC which is somewhat limited, however with a more recent loadp2 tool nominated you can download to an automatically detected port.   For that you just need to pass the LOADP2 variable into make and assign it to the loadp2 file installed on your system as <file_path_to_loadp2>:

    $ make LOADP2=<file_path_to_loadp2> PORT=<port> run

You can also specify the options sent to loadp2 with LOADP2_OPTS, overriding the PORT option if that is also specified.

    $ make LOADP2=<file_path_to_loadp2> LOADP2_OPTS=<all_options> run

OR, once built, you can manually download using your loadp2 application and its built-in terminal emulator:

    $ loadp2 -t build/python.bin

The default operating serial port baud setting used by this image is 115200 N81.

## Build information

### Debug files

Other files are created during the build and are available for debugging:

    build/python.linker - detailed linker debug output
    build/python.map - image symbol information
    build/python.lss - disassembled output of binary image

### Customizing MicroPython

The mpconfigport.h file allows further customization of MicroPython features.

### Baud rate, initial stack location, default operating frequency

Start up settings are defined in prefix.spin2 prefix file in the ports/p2 folder.  Right now the processor frequency and baud rate is not adjustable after boot, however this could be added in the future.

### MicroPython Heap Size

The size of the MicroPython heap is currently defined in main.c using the HEAP_SIZE_KB parameter.  Care must be taken to not make it too large such that it exceeds the Propeller 2 MCU 512kB internal memory limit after accounting for a sufficient MicroPython stack and any other HUB memory use outside of MicroPython.

