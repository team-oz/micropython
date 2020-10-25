# The Propeller 2 native port

This MicroPython port runs natively on the Propeller 2 MCU.

It should build under common OS hosts including Linux, Cygwin, Darwin (OS X).

## Prerequisites

This port requires some tools installed and available in the working path.

### Propeller GCC Compiler

You will require propeller-elf-gcc and its include files.

This is a GCC Compiler for Propeller 1 chip, code will be translated for the P2.

This build was tested with GCC version 4.6.1

You may find information here:
https://github.com/parallaxinc/propgcc

Note: Under MAC OS X (Yosemite) this compiler can sometimes randomly segfault with an
internal compiler error.  If this happens, just re-invoke make to continue 
where the build left off.

### P2GCC

These tools translate the compiled P1 code into P2 code and assemble and link.
This is available as a submodule of the P2 port and should be automatically 
included the first time Make is invoked.  If not, you can try:

   git submodule init lib/p2gcc
   git submodule update lib/p2gcc

### LOADP2

An older version is included with the P2GCC library, but the newer versions work
better and offer more capabililities such as higher baud rates and automatic
port detection.  

It is available on github here:

https://github.com/totalspectrum/loadp2

You may also use PropTool to download the binary.

### Other tools used in the P2 Makefile

perl, sed, gcc (native C compiler on host OS, used to build P2GCC tools)


## Building and running MicroPython on a P2

By default the port will be built for the P2 machine:

    $ make


### Verbose output

To enable verbose output include the V=1 setting with make:

    $ make V=1

## Build information

### Target image

The default target binary is:

    build/python.bin

### Debug files

Other files are created during the build and are available for debugging:

    build/python.linker - detailed linker debug output
    build/python.map - image symbol information
    build/python.lss - disassembled output of binary image

## Downloading the binary directly to the Propeller 2 MCU

To download and run the executable image and get a basic working REPL do this:

    $ make PORT=<port> run

    where <port> is your OS serial port, e.g /dev/cu.usbserial-XYZABCDE etc

This will attempt to use the loadp2 tool shipped with P2GCC which is limited,
with a recent loadp2 tool nominated you can download to an automatically detected port:

    $ make LOADP2=<file_path_to_loadp2> run

       OR, once built

    $ loadp2 -t build/python.bin

The default operating serial port baud setting used by this image is 115200 N81.

