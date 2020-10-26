#BUILD_VERBOSE = 1
include ../../py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

#MICROPY_VFS_LFS2 = 1

ifeq '$(findstring ;,$(PATH))' ';'
    detected_OS := Windows
else
    detected_OS := $(shell uname 2>/dev/null || echo Unknown)
    detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
    detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
    detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif

ifeq ($(detected_OS),Cygwin)
    EXE = .exe
else ifeq ($(detected_OS),MSYS)
    EXE = .exe
else
    EXE = 
endif

# include py core make definitions
include $(TOP)/py/py.mk

GIT_SUBMODULES = lib/p2gcc

CROSS_COMPILE = propeller-elf-

ifeq ($(BUILD_VERBOSE), 0)
SILENT = > /dev/null
else
SILENT = 
endif

# Tools
PERL = perl

# P2GCC directories
P2GCC = $(TOP)/lib/p2gcc/bin

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

#DFU = $(TOP)/tools/dfu.py
#PYDFU = $(TOP)/tools/pydfu.py
CFLAGS = $(INC) -Wall -std=c99  $(COPT) -mcog -S -mno-fcache $(CFLAGS_MOD) -m32bit-doubles -DP2GCC_COMPILER
LD = propeller-elf-gcc
LDFLAGS = -mxmm -Wl,-Map=$@.map -Wl,--gc-sections

# Tune for Debugging or Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
endif

LIBS =

SRC_C = \
    main.c \
    mphalport.c \
    moduos.c \
    modmachine.c \
    modutime.c \
    machine_pin.c \
    machine_cpu.c \
    sdcard.c \
    math.c \
    gpio.c \
    lib/p2gcc/lib/longlong.c \
    lib/p2gcc/lib/float.c \
    lib/p2gcc/lib/p2start.c \
    lib/utils/printf.c \
    lib/utils/stdout_helpers.c \
    lib/utils/pyexec.c \
    lib/libc/string0.c \
    lib/mp-readline/readline.c \
    lib/oofatfs/ff.c \
    lib/oofatfs/ffunicode.c \
    lib/timeutils/timeutils.c \
    lib/libm/asinfacosf.c \
    lib/libm/atanf.c \
    lib/libm/kf_cos.c \
    lib/libm/kf_sin.c \
    lib/libm/kf_tan.c \
    lib/libm/atan2f.c \
    lib/libm/math.c \
    lib/libm/fmodf.c \
    lib/libm/sf_frexp.c \
    lib/libm/sf_modf.c \
    lib/libm/nearbyintf.c \
    lib/libm/sf_sin.c \
    lib/libm/sf_cos.c \
    lib/libm/sf_tan.c \
    lib/libm/ef_sqrt.c \
    lib/libm/ef_rem_pio2.c \
    lib/libm/kf_rem_pio2.c \
    lib/libm/sf_ldexp.c \
    $(BUILD)/_frozen_mpy.c \
    $(SRC_MOD) \

# List of SPIN2 library source files
SRC_SPIN2 = \
    lib/p2gcc/lib/intrinsics.spin2 \
    lib/p2gcc/lib/cognew2.spin2 \
    lib/p2gcc/lib/setjmp.spin2 \

# Special single SPIN2 prefix file that contains the VM startup code and must be linked first
SRC_PREFIX = prefix.spin2
    
PREFIX_O = $(addprefix $(BUILD)/, $(SRC_PREFIX:.spin2=.o))
SPIN_O = $(addprefix $(BUILD)/, $(SRC_SPIN2:.spin2=.o))

OBJ = $(PREFIX_O) $(SPIN_O) $(PY_CORE_O) $(PY_EXTMOD_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o))

P2GCCTOOLS = \
    $(TOP)/lib/p2gcc/bin/p2asm$(EXE) \
    $(TOP)/lib/p2gcc/bin/s2pasm$(EXE) \
    $(TOP)/lib/p2gcc/bin/p2link$(EXE) \
    $(TOP)/lib/p2gcc/bin/p2dump$(EXE) \
    $(TOP)/lib/p2gcc/bin/loadp2$(EXE) \

# add some default loadp2 options if not specified
LOADP2_OPTS ?= -b 115200
LOADP2 ?= $(TOP)/lib/p2gcc/bin/loadp2$(EXE)

ifneq ($(PORT),)
    LOADP2_OPTS += -p $(PORT)
endif

LOADP2TOOL = $(LOADP2)

$(OBJ) : $(SRC_PREFIX) $(P2GCCTOOLS)

all: $(TOP)/lib/p2gcc/readme.txt $(BUILD)/python.bin

$(BUILD)/_frozen_mpy.c: frozentest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
	$(ECHO) "MISC freezing bytecode"
	$(Q)$(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=mpz $< > $@


vpath %.c . $(TOP) $(USER_C_MODULES)

# For convenience, automatically fetch required submodule(s) if they don't exist 
$(TOP)/lib/p2gcc/readme.txt : 
	$(ECHO) "p2gcc submodule not found, fetching it now..."
	(cd $(TOP) && git submodule update --init lib/p2gcc)
    
# Unfortunately p2gcc doesn't use Makefiles and just has these build scripts
# to make everything, so we will use those for now.
$(P2GCCTOOLS) : $(TOP)/lib/p2gcc/readme.txt
ifeq ($(detected_OS), Darwin)
	(cd $(TOP)/lib/p2gcc && sh build_macos)
endif
ifeq ($(detected_OS), Linux)
	(cd $(TOP)/lib/p2gcc && sh build_linux)
endif
ifeq ($(detected_OS), Cygwin)
	(cd $(TOP)/lib/p2gcc && sh build_cygwin)
endif
ifeq ($(detected_OS), MSYS)
	(cd $(TOP)/lib/p2gcc && sh build_mingw)
endif


$(BUILD)/python.bin: $(OBJ)
	$(ECHO) "Detected OS is $(detected_OS)"
	$(ECHO) "Linking $^"
	$(Q)$(P2GCC)/p2link -d -v -o $@ -m $(@:.bin=.map) $^ >$(@:.bin=.linker)
	$(Q)$(P2GCC)/p2dump -dis -data -asc $@ > $(@:.bin=.lss)
ifeq ($(detected_OS), Darwin)
	$(ECHO) "Image size of $@ is `stat -L -f %z $@` bytes"
endif

#TODO: add image flashing tool for P2 port
#deploy: $(BUILD)/python.bin
#	$(ECHO) "Writing $< to the board"
#	$(Q)$(PYTHON) $(PYDFU) -u $<

run: $(BUILD)/python.bin 
	$(LOADP2TOOL) $(LOADP2_OPTS) -t $^

test: $(BUILD)/python.bin
	/bin/echo -e "print('hello world!', list(x+1 for x in range(10)), end='eol\\\\n')\\r\\n\\x04" | $(LOADP2TOOL) $(LOADP2_OPTS) -t $^ | tail -n2 | grep "^hello world! \\[1, 2, 3, 4, 5, 6, 7, 8, 9, 10\\]eol"

# List of sources for qstr extraction
SRC_QSTR += moduos.c sdcard.c machine_cpu.c modmachine.c modutime.c machine_pin.c
# Append any auto-generated sources that are needed by sources listed in
# SRC_QSTR
SRC_QSTR_AUTO_DEPS +=


include $(TOP)/py/mkrules.mk

# Redefine C compilation to translate to P2 objects from P1 assembly using P2GCC tools
define compile_c
    $(ECHO) "Compiling $<"
    $(Q)$(CC) $(CFLAGS) -S -MD -o $(@:.o=.spin1) $<
    @$(CP) $(@:.o=.d) $(@:.o=.P); \
      $(SED) -e 's/#.*//' -e 's/^.*:  *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P); \
      $(RM) -f $(@:.o=.d)
    $(Q)$(CP) $(@:.o=.spin1) $(@:.o=.spin0)
    $(Q)$(ECHO) "Optimizing prologs/epilogs for $(@:.o=.spin1)" $(SILENT)
    $(Q)$(PERL) -i -0p optimizer/prolog8 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/prolog9 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/prolog10 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/prolog11 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/prolog12 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/prolog13 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/prolog14 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/epilog8 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/epilog9 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/epilog10 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/epilog11 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/epilog12 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/epilog13 $(@:.o=.spin0)
    $(Q)$(PERL) -i -0p optimizer/epilog14 $(@:.o=.spin0)
    $(Q)$(SED) -f optimizer/transforms_rev.sed $(@:.o=.spin0) > $(@:.o=.spin1)
    $(Q)$(RM) -f $(@:.o=.spin0)
    $(Q)$(ECHO) "Translating $(@:.o=.spin1) into SPIN2" $(SILENT)
    $(Q)$(P2GCC)/s2pasm -p $(SRC_PREFIX) $(@:.o=.spin1)
    $(Q)$(ECHO) "Optimizing constants for $(@:.o=.spin2)" $(SILENT)
    $(Q)optimizer/optimize $(@:.o=.spin2)
    $(Q)$(ECHO) "Assembling $(@:.o=.spin2)" $(SILENT)
    $(Q)$(P2GCC)/p2asm -c -o -hub $(@:.o=.spin2)
    $(Q)$(RM) -f $(@:.o=.lst)
    $(Q)$(RM) -f $(@:.o=.bin)
    $(Q)$(RM) -f $(@:.o=.spin1)
    $(Q)$(RM) -f $(@:.o=.spin2)
endef

# Define how to build SPIN2 files
define compile_spin
    $(ECHO) "Compiling $<"
    $(Q)$(CP) $< $(@:.o=.spin2)
    $(Q)$(ECHO) "Assembling $(@:.o=.spin2)" $(SILENT)
    $(Q)$(P2GCC)/p2asm -o $(1) $(@:.o=.spin2)
    $(Q)$(RM) -f $(@:.o=.spin2)
    $(Q)$(RM) -f $(@:.o=.lst)
    $(Q)$(RM) -f $(@:.o=.bin)
endef


# Prefix file is special and gets compiled without -hub option
$(PREFIX_O) : $(SRC_PREFIX)
	$(call compile_spin)

vpath %.spin2 . $(TOP)

# Other spin files are compiled with -hub option
$(BUILD)/%.o: %.spin2
	$(call compile_spin, -hub)

