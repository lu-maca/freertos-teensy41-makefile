# Teensyduino Core Library Makefile (fixed includes)

MCU = IMXRT1062
MCU_LD = imxrt1062_t41.ld
MCU_DEF = ARDUINO_TEENSY41

BUILDDIR = $(abspath $(CURDIR)/build)
TARGET = $(notdir $(CURDIR))

# Toolchain
CC  = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Teensy paths
TOOLSPATH = $(CURDIR)/tools
COREPATH  = teensy4
LIBRARYPATH = libs

# Compiler options
OPTIONS = -DF_CPU=600000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -DUSING_MAKEFILE
OPTIONS += -D__$(MCU)__ -DARDUINO=10607 -DTEENSYDUINO=159 -D$(MCU_DEF) -D__NEWLIB__=1
CPUOPTIONS = -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb

CPPFLAGS = -Wall -g -O2 $(CPUOPTIONS) -MMD $(OPTIONS) \
           -Isrc -I$(COREPATH) -ffunction-sections -fdata-sections

CXXFLAGS = -c -O2 -g -Wall -MMD -std=gnu++20 -fno-exceptions -fpermissive \
           -fno-rtti -fno-threadsafe-statics -felide-constructors -Wno-error=narrowing -mthumb \
		   -specs=nano.specs -DALTERNATIVE_MUTEX_IMPL

CFLAGS = -std=gnu99 -c -O2 -g -Wall -ffunction-sections -fdata-sections -MMD -mthumb

# Linker
LD_PATH = $(COREPATH)/$(MCU_LD)
LDFLAGS = -Os -Wl,--gc-sections,--relax $(SPECS) $(CPUOPTIONS) -T$(LD_PATH) -specs=nano.specs
LIBS = -lm -lstdc++ 

# ----------------------------------------------------------------------
# Source files
# ----------------------------------------------------------------------
SUBDIRS := $(shell find $(LIBRARYPATH) -type d)
SUBDIRS := $(filter-out %/example %/example/%,$(SUBDIRS))

LC_FILES   := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c))
LCPP_FILES := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cpp))
TC_FILES   := $(wildcard $(COREPATH)/*.c)
TCPP_FILES := $(wildcard $(COREPATH)/*.cpp)
C_FILES    := $(wildcard src/*.c)
CPP_FILES  := $(wildcard src/*.cpp)

# ----------------------------------------------------------------------
# Include paths
# ----------------------------------------------------------------------
FREERTOS_CPP_INC  = libs/freertos/lib/cpp/include

L_INC := $(foreach lib,$(SUBDIRS),-I$(lib) -I$(lib)/src) \
		 -I$(COREPATH) -I$(LIBRARYPATH) -Isrc -I$(FREERTOS_CPP_INC)

# ----------------------------------------------------------------------
# Build objects
# ----------------------------------------------------------------------
SOURCES := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o) \
           $(TC_FILES:.c=.o) $(TCPP_FILES:.cpp=.o) \
           $(LC_FILES:.c=.o) $(LCPP_FILES:.cpp=.o)

OBJS := $(foreach src,$(SOURCES), $(BUILDDIR)/$(src))

# ----------------------------------------------------------------------
# Targets
# ----------------------------------------------------------------------
all: $(TARGET).hex

build: $(TARGET).elf

hex: $(TARGET).hex

post_compile: $(TARGET).hex
	@$(abspath $(TOOLSPATH))/teensy_post_compile -file="$(basename $<)" -path=$(CURDIR) -tools="$(abspath $(TOOLSPATH))"

reboot:
	@-$(abspath $(TOOLSPATH))/teensy_reboot

upload: $(TARGET).hex
	@-$(abspath $(TOOLSPATH))/teensy_loader_cli -w -s -v --mcu=TEENSY41 $(basename $<).hex

# ----------------------------------------------------------------------
# Compile rules
# ----------------------------------------------------------------------
$(BUILDDIR)/%.o: %.c
	@echo -e "[CC]\t$<"
	@mkdir -p "$(dir $@)"
	$(CC) $(CPPFLAGS) $(CFLAGS) $(L_INC) -o "$@" -c "$<" 

$(BUILDDIR)/%.o: %.cpp
	@echo -e "[CXX]\t$<"
	@mkdir -p "$(dir $@)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(L_INC) -o "$@" -c "$<"

# ----------------------------------------------------------------------
# Linking
# ----------------------------------------------------------------------
$(TARGET).elf: $(OBJS) $(LD_PATH)
	@echo -e "[LD]\t$@"
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.hex: %.elf
	$(SIZE) $<
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# ----------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------
-include $(OBJS:.o=.d)

clean:
	@echo Cleaning...
	@rm -rf "$(BUILDDIR)"
	@rm -f "$(TARGET).elf" "$(TARGET).hex"
