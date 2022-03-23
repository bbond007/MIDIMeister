CC=arm-linux-gnueabihf-
CC_X86=
BIN=MIDIMeister
BIN_DBG=$(BIN)-debug
BIN_X86=$(BIN_DBG)-x86
LDFLAGS=-lasound 
CCFLAGS_DBG=-DINCLUDE_UDP -DINCLUDE_DEBUG -DINCLUDE_SERIAL -Ialsa/include -Lalsa/lib -Ofast -mcpu=cortex-a9 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -funsafe-math-optimizations   
CCFLAGS=-DINCLUDE_UDP -DINCLUDE_SERIAL -Ialsa/include -Lalsa/lib -Ofast -mcpu=cortex-a9 -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -funsafe-math-optimizations   
CCFLAGS_X86=-DINCLUDE_DEBUG -DPOLL_SEQ_HAND -DINCLUDE_UDP

all:
	$(CC)gcc $(CCFLAGS) $(LDFLAGS) src/main.c src/misc.c src/udpsock.c src/serial.c -o $(BIN) 
	$(CC)strip $(BIN)
	
debug:
	$(CC)gcc $(CCFLAGS_DBG) $(LDFLAGS) src/main.c src/misc.c src/udpsock.c src/serial.c -o $(BIN_DBG) 
	$(CC)strip $(BIN_DBG)

x86:
	$(CC_X86)gcc $(CCFLAGS_X86) $(LDFLAGS) src/main.c src/misc.c src/udpsock.c src/serial.c -o $(BIN_X86) 
	$(CC_X86)strip $(BIN_X86)

clean:
	rm -f $(BIN) $(BIN_DBG) $(BIN_X86) 