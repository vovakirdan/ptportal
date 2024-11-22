print-%:; @echo $($*)

CC = gcc
LD = gcc

# library paths
PATH_LIB = lib
PATH_SDL = $(PATH_LIB)/SDL

INCFLAGS = -iquotesrc

CCFLAGS  = -std=c2x
CCFLAGS += -O2
CCFLAGS += -g
CCFLAGS += -fbracket-depth=1024
CCFLAGS += -fmacro-backtrace-limit=0
CCFLAGS += -Wall
CCFLAGS += -Wextra
CCFLAGS += -Wpedantic
CCFLAGS += -Wfloat-equal
CCFLAGS += -Wstrict-aliasing
CCFLAGS += -Wswitch-default
CCFLAGS += -Wformat=2
CCFLAGS += -Wno-newline-eof
CCFLAGS += -Wno-unused-parameter
CCFLAGS += -Wno-strict-prototypes
CCFLAGS += -Wno-fixed-enum-extension
CCFLAGS += -Wno-int-to-void-pointer-cast
CCFLAGS += -Wno-gnu-statement-expression
CCFLAGS += -Wno-gnu-compound-literal-initializer
CCFLAGS += -Wno-gnu-zero-variadic-macro-arguments
CCFLAGS += -Wno-gnu-empty-struct
CCFLAGS += -Wno-gnu-auto-type
CCFLAGS += -Wno-gnu-empty-initializer
CCFLAGS += -Wno-gnu-pointer-arith
CCFLAGS += -Wno-c99-extensions
CCFLAGS += -Wno-c11-extensions

LDFLAGS = -lm

BIN = bin
SRC = $(shell find src -name "*.c")
OBJ = $(SRC:%.c=$(BIN)/%.o)
DEP = $(SRC:%.c=$(BIN)/%.d)
OUT = $(BIN)/game

-include $(DEP)

# UNAME := $(shell uname -s)
BREW := $(shell which brew)
ifeq ($(BREW),)
	LDFLAGS += -lSDL2
else
	CC = $(shell brew --prefix llvm)/bin/clang
	LD = $(shell brew --prefix llvm)/bin/clang

	INCFLAGS += -I$(PATH_SDL)/include
	LDFLAGS += $(shell $(BIN)/sdl/sdl2-config --prefix=$(BIN) --static-libs)
	LDFLAGS += -lSDL2
endif
# ifeq ($(UNAME),Linux)
# 	CC = $(shell brew --prefix llvm)/bin/clang
# 	LD = $(shell brew --prefix llvm)/bin/clang

# 	INCFLAGS += -I$(PATH_SDL)/include
# 	LDFLAGS += $(shell $(BIN)/sdl/sdl2-config --prefix=$(BIN) --static-libs)
# 	LDFLAGS += -lSDL2
# else ifeq ($(UNAME),Darwin)
# 	LDFLAGS += -lSDL2
# endif

$(BIN):
	mkdir -p $@

dirs: $(BIN)
	rsync -a --include '*/' --exclude '*' "src" "bin"

lib-sdl:
	mkdir -p $(BIN)/sdl
	cmake -S $(PATH_SDL) -B $(BIN)/sdl
	cd $(BIN)/sdl && make -j 10
	chmod +x $(BIN)/sdl/sdl2-config
	mkdir -p $(BIN)/lib
	cp $(BIN)/sdl/libSDL2.a $(BIN)/lib

libs: lib-sdl

$(OBJ): $(BIN)/%.o: %.c
	$(CC) -o $@ -MMD -c $(CCFLAGS) $(INCFLAGS) $<

doom: dirs $(BIN)/src/main_doom.o
	$(LD) -o bin/main_doom $(BIN)/src/main_doom.o $(LDFLAGS)

portal: dirs $(BIN)/src/main_portal.o
	$(LD) -o bin/portal $(BIN)/src/main_portal.o $(LDFLAGS)

all: dirs doom portal

clean:
	rm -rf bin