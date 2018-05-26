CC = gcc
CFLAGS = -O2 -D_FORTIFY_SOURCE=2 -fno-plt -Wl,-z,relro,-z,now -fPIC -pie -Wall -pedantic -fno-omit-frame-pointer -fstack-protector-all -ftrapv -Wstrict-prototypes -mfsgsbase
LDFLAGS = -Wl,-z,relro,-z,now -pie -lm -ldl

BUILD = build

HEADERS = jitmenot.h pmparser.h testEnvVar.h testJITBranchTime.h testJITLibTime.h testNX.h testPagePerm.h testMapname.h testRIPFXSAVE.h testRIPSIGINFO.h testRIPSYSCALL.h testSMC.h testVMLeave.h testFsbase.h testEnter.h testFindConstant.h Makefile
OBJ = main.o pmparser.o testEnvVar.o testJITBranchTime.o testJITLibTime.o testNX.o testPagePerm.o testMapname.o testRIPFXSAVE.o testRIPSIGINFO.o testRIPSYSCALL.o testSMC.o testVMLeave.o testFsbase.o testEnter.o testFindConstant.o
TARGETS = $(patsubst %,$(BUILD)/%,$(OBJ))

.PHONY: all

all: directories $(BUILD)/jitmenot

.PHONY: debug

debug: CFLAGS += -DDEBUG -g
debug: all

$(BUILD)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILD)/jitmenot: $(TARGETS)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: directories
directories:
	@ mkdir -p $(BUILD)

.PHONY: clean

clean:
	@rm -rf $(BUILD)
