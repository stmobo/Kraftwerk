# supports x86 only for now
ARCH := x86-64

#
# Source directories and stuff:
MAIN_SRC	:= ./os
MAIN_OBJ    := ./obj
INCLUDE_DIR := $(MAIN_SRC)/include
AUX_INCLUDE_DIR := ./auxlib

ISO_DIR     := ./isodir

ARCH_SRC	:= $(MAIN_SRC)/arch/$(ARCH)
BOOT_SRC	:= $(MAIN_SRC)/boot/$(ARCH)
CORE_SRC	:= $(MAIN_SRC)/core
DEVICE_SRC	:= $(MAIN_SRC)/device
#FS_SRC     := $(MAIN_SRC)/fs
#LIB_SRC 	:= $(MAIN_SRC)/lib
SRC_DIRS    := $(ARCH_SRC) $(BOOT_SRC) $(CORE_SRC) $(FS_SRC) $(DEVICE_SRC) $(LIB_SRC)
OBJ_DIRS    := $(subst $(MAIN_SRC),$(MAIN_OBJ),$(SRC_DIRS))

# find all files of a certain type
HDR_FILES    := $(shell find $(INCLUDE_DIR) -type f -name *.h)
C_FILES      := $(shell find $(SRC_DIRS) -type f -name *.c)
CPP_FILES    := $(shell find $(SRC_DIRS) -type f -name *.cpp)
ASM_FILES    := $(shell find $(SRC_DIRS) -type f -name *.s)

# a few other things
C_OBJ_FILES   :=  $(subst os/,obj/,$(patsubst %.c, %.o, $(C_FILES)))
CPP_OBJ_FILES :=  $(subst os/,obj/,$(patsubst %.cpp, %.o, $(CPP_FILES)))
ASM_OBJ_FILES :=  $(subst os/,obj/,$(patsubst %.s, %.o, $(ASM_FILES)))
OBJ_FILES     :=  $(ASM_OBJ_FILES) $(C_OBJ_FILES) $(CPP_OBJ_FILES)
DEP_FILES     :=  $(patsubst %.o, %.d, $(CPP_OBJ_FILES) $(C_OBJ_FILES))
CRTBEGIN_OBJ  :=  $(shell $(HOME)/opt/x64-cross/bin/x86_64-elf-gcc -print-file-name=crtbegin.o)
CRTEND_OBJ    :=  $(shell $(HOME)/opt/x64-cross/bin/x86_64-elf-gcc -print-file-name=crtend.o)

# generate a link order
LINK_ORDER_FIRST := 
LINK_ORDER_LAST  := 
LINK_ORDER_MID   := $(filter-out $(LINK_ORDER_FIRST) $(LINK_ORDER_LAST), $(OBJ_FILES))

# remap stuff to use our cross-compiler
AS       := $(HOME)/opt/x64-cross/bin/x86_64-elf-as
CC       := $(HOME)/opt/x64-cross/bin/x86_64-elf-gcc
CCFLAGS  := -I$(INCLUDE_DIR) -MMD -MP -std=gnu99 -ffreestanding -g -O2 -Wall -Wextra -mcmodel=large -Wno-unused-parameter -fno-omit-frame-pointer -fno-strict-aliasing
CXX      := $(HOME)/opt/x64-cross/bin/x86_64-elf-g++
CXXFLAGS := -MMD -MP -I$(INCLUDE_DIR) -I$(AUX_INCLUDE_DIR) -D__$(ARCH)__ -ffreestanding -g -O2 -Wall -Wextra -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -fno-exceptions -fno-rtti -fno-omit-frame-pointer -std=c++14
LD       := $(HOME)/opt/x64-cross/bin/x86_64-elf-gcc
LDFLAGS  := -z max-page-size=0x1000 -nostdlib -ffreestanding -g -O2 -L./lib/x86_64-elf/lib
LDLIBS   := -lgcc

all: kraftwerk.iso

kraftwerk.iso: kraftwerk.elf
		@cp kraftwerk.elf $(ISO_DIR)/boot/kraftwerk.elf
		@cp kraftwerk_embedded_debug.elf $(ISO_DIR)/kraftwerk_embedded_debug.elf
		@grub-mkrescue -o kraftwerk.iso $(ISO_DIR) > /dev/null 2>&1

kraftwerk.elf: kraftwerk_embedded_debug.elf
		@$(HOME)/opt/x64-cross/bin/x86_64-elf-objcopy -S kraftwerk_embedded_debug.elf kraftwerk.elf

debug: kraftwerk_embedded_debug.elf
		@$(HOME)/opt/x64-cross/bin/x86_64-elf-objcopy --only-keep-debug kraftwerk_embedded_debug.elf kraftwerk.sym
		@$(HOME)/opt/x64-cross/bin/x86_64-elf-readelf -s kraftwerk.sym | sort -k 2,2 > ./symbols

kraftwerk_embedded_debug.elf: $(OBJ_FILES)
		@$(LD) -T $(MAIN_SRC)/linker.ld $(LDFLAGS) -o ./kraftwerk_embedded_debug.elf $(LINK_ORDER_FIRST) $(LINK_ORDER_MID) $(LINK_ORDER_LAST) $(LDLIBS)

clean:
		@$(RM) $(OBJ_FILES)
		@$(RM) $(DEP_FILES)
		@$(RM) kraftwerk.iso
		@$(RM) kraftwerk.elf

$(C_OBJ_FILES): $(MAIN_OBJ)/%.o : $(MAIN_SRC)/%.c
		@$(CC) $(CCFLAGS) -c $< -o $@

$(CPP_OBJ_FILES): $(MAIN_OBJ)/%.o : $(MAIN_SRC)/%.cpp
		@$(CXX) $(CXXFLAGS) -c $< -o $@

$(ASM_OBJ_FILES): $(MAIN_OBJ)/%.o : $(MAIN_SRC)/%.s
		@$(AS) -c $< -o $@

-include $(DEP_FILES)

.PHONY: clean debug
