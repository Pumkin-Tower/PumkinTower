
# toolchain
CC      = ../../../z64rom/tools/mips64-binutils/bin/mips64-gcc
LD      = ../../../z64rom/tools/mips64-binutils/bin/mips64-ld
OBJCOPY = ../../../z64rom/tools/mips64-binutils/bin/mips64-objcopy
OBJDUMP = ../../../z64rom/tools/mips64-binutils/bin/mips64-objdump
PUT     = z64put      # this utility generates the cloudpatch
TARGET  = ../../code1.txt

# entry point
CODE_RAM = 0x800A5AC0
CODE_ROM = 0x00B3C000
ADDRESS_RAM = 0x8014ADBC # this is Message_DrawTextDefault
ADDRESS_ROM := $(shell printf "%#x\n" $$(($(ADDRESS_RAM) - $(CODE_RAM) + $(CODE_ROM))))

# the original addresses of each function, transformed from ram addresses to rom addresses
Scene_ExecuteCommands_HOOK = $(shell printf "%#x\n" $$((0x801306E8 - $(CODE_RAM) + $(CODE_ROM))))
Entrance_GetTableEntry_HOOK = $(shell printf "%#x\n" $$((0x80169F78 - $(CODE_RAM) + $(CODE_ROM))))
Play_SpawnScene_HOOK = $(shell printf "%#x\n" $$((0x801693D4 - $(CODE_RAM) + $(CODE_ROM))))
func_80169EFC_HOOK = $(shell printf "%#x\n" $$((0x80169EFC - $(CODE_RAM) + $(CODE_ROM))))
Magic_Update_HOOK = $(shell printf "%#x\n" $$((0x80116348 - $(CODE_RAM) + $(CODE_ROM))))
Actor_Init_HOOK = $(shell printf "%#x\n" $$((0x800B6834 - $(CODE_RAM) + $(CODE_ROM))))
QuestHint_GetTatlTextId_HOOK = $(shell printf "%#x\n" $$((0x800F05C0 - $(CODE_RAM) + $(CODE_ROM))))

# default compilation flags
CFLAGS = -G 0 -nostdinc -DNDEBUG -I../../../z64rom/include/z64hdr/mm -Os -s --std=gnu99 -march=vr4300 -mfix4300 -mabi=32 -mno-abicalls -mdivide-breaks -fno-zero-initialized-in-bss -fno-toplevel-reorder -ffreestanding -fno-common -fno-merge-constants -mno-explicit-relocs -mno-split-addresses -funsigned-char -mno-memcpy -Wall -Wextra -Wno-unknown-pragmas
LDFLAGS = -L ../../../z64rom/include/z64hdr/common -L../../../z64rom/include/z64hdr/mm_u10 -T z64hdr.ld --emit-relocs

default: mod.bin cloudpatch

mod.bin:
	@echo -n "ENTRY_POINT = " > entry.ld
	@echo -n $(ADDRESS_RAM) >> entry.ld
	@echo ";" >> entry.ld
	@$(CC) -c mod.c $(CFLAGS) -Os
	@$(LD) -o mod.elf mod.o $(LDFLAGS)
	@$(OBJCOPY) -R .MIPS.abiflags -O binary mod.elf mod.bin
	$(OBJDUMP) -t mod.elf | grep -i Custom_

cloudpatch:
	@rm $(TARGET)
	$(PUT) $(TARGET) --file $(ADDRESS_ROM) mod.bin
	$(PUT) $(TARGET) --jump $(Scene_ExecuteCommands_HOOK) $(shell $(OBJDUMP) -t mod.elf | grep Custom_Scene_ExecuteCommands | head -c 8)
	$(PUT) $(TARGET) --jump $(Entrance_GetTableEntry_HOOK) $(shell $(OBJDUMP) -t mod.elf | grep custom_func_80169F78 | head -c 8)
	$(PUT) $(TARGET) --jump $(Play_SpawnScene_HOOK) $(shell $(OBJDUMP) -t mod.elf | grep Custom_Play_SpawnScene | head -c 8)
	$(PUT) $(TARGET) --jump $(func_80169EFC_HOOK) $(shell $(OBJDUMP) -t mod.elf | grep Custom_func_80169EFC | head -c 8)
	$(PUT) $(TARGET) --jump $(Magic_Update_HOOK) $(shell $(OBJDUMP) -t mod.elf | grep Custom_Magic_Update | head -c 8)
	$(PUT) $(TARGET) --jump $(Actor_Init_HOOK) $(shell $(OBJDUMP) -t mod.elf | grep Custom_Actor_Init | head -c 8)
	$(PUT) $(TARGET) --jump $(QuestHint_GetTatlTextId_HOOK) $(shell $(OBJDUMP) -t mod.elf | grep Custom_QuestHint_GetTatlTextId | head -c 8)

clean:
	rm -f *.bin *.elf *.ovl

