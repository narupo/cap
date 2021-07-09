# get rm and rmdir and sep
ifeq ($(OS), Windows_NT)
	RM := del
	RMDIR := rmdir /s /q
	SEP := \\
else
	RM := rm
	RMDIR := rm -rf
	SEP := /
endif

# windows's mkdir not has -p option :/
MKDIR := mkdir
CC := gcc

ifeq ($(OS), Windows_NT)
	CFLAGS := -Wall \
		-g \
		-O0 \
		-std=c11 \
		-Wno-unused-function \
		-Wno-unused-result \
		-D_DEBUG \
		-I . \
		-I build/pad \
		-L build
	LIBPAD := libpad.dll
else
	CFLAGS := -Wall \
		-g \
		-O0 \
		-std=c11 \
		-Wno-unused-function \
		-Wno-unused-result \
		-D_DEBUG \
		-I . \
		-I build/pad \
		-L build
	LIBPAD := libpad.so
endif

# this is benri tool
# $(warning $(wildcard src/*.c))

all: pad cap tests 

.PHONY: clean
clean:
	$(RMDIR) build

.PHONY: init
init:
	$(MKDIR) \
	build \
	build$(SEP)core \
	build$(SEP)home \
	build$(SEP)cd \
	build$(SEP)pwd \
	build$(SEP)ls \
	build$(SEP)cat \
	build$(SEP)run \
	build$(SEP)exec \
	build$(SEP)alias \
	build$(SEP)edit \
	build$(SEP)editor \
	build$(SEP)mkdir \
	build$(SEP)rm \
	build$(SEP)mv \
	build$(SEP)cp \
	build$(SEP)touch \
	build$(SEP)snippet \
	build$(SEP)link \
	build$(SEP)make \
	build$(SEP)cook \
	build$(SEP)sh \
	build$(SEP)find \
	build$(SEP)bake \
	build$(SEP)insert \
	build$(SEP)clone \
	build$(SEP)replace \
	build$(SEP)lang

.PHONY: cc
cc:
	$(CC) -v

SRCS := build/lib/error.c \
	build/core/config.c \
	build/core/util.c \
	build/core/alias_manager.c \
	build/core/alias_info.c \
	build/core/symlink.c \
	build/core/args.c \
	build/core/error_stack.c \
	build/home/home.c \
	build/cd/cd.c \
	build/pwd/pwd.c \
	build/ls/ls.c \
	build/cat/cat.c \
	build/run/run.c \
	build/exec/exec.c \
	build/alias/alias.c \
	build/edit/edit.c \
	build/editor/editor.c \
	build/mkdir/mkdir.c \
	build/rm/rm.c \
	build/mv/mv.c \
	build/cp/cp.c \
	build/touch/touch.c \
	build/snippet/snippet.c \
	build/link/link.c \
	build/hub/hub.c \
	build/make/make.c \
	build/cook/cook.c \
	build/sh/sh.c \
	build/find/find.c \
	build/find/arguments_manager.c \
	build/bake/bake.c \
	build/insert/insert.c \
	build/clone/clone.c \
	build/replace/replace.c \
	build/lang/importer.c

OBJS := $(SRCS:.c=.o)

pad: build/$(LIBPAD)

build/$(LIBPAD):
	cd build && \
		git clone https://github.com/narupo/pad && \
		cd pad && \
		make init && \
		make && \
		cd .. && \
		cp pad/build/$(LIBPAD) .

cap: build/app.o build/$(LIBPAD) $(OBJS)
	$(CC) $(CFLAGS) -o build/cap build/app.o $(OBJS) -lpad

tests: build/tests.o build/$(LIBPAD) $(OBJS)
	$(CC) $(CFLAGS) -o build/tests build/tests.o $(OBJS) -lpad

build/app.o: src/app.c src/app.h
	$(CC) $(CFLAGS) -c $< -o $@
build/tests.o: src/tests.c src/tests.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/config.o: src/core/config.c src/core/config.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/util.o: src/core/util.c src/core/util.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/alias_manager.o: src/core/alias_manager.c src/core/alias_manager.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/alias_info.o: src/core/alias_info.c src/core/alias_info.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/symlink.o: src/core/symlink.c src/core/symlink.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/error_stack.o: src/core/error_stack.c src/core/error_stack.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/args.o: src/core/args.c src/core/args.h
	$(CC) $(CFLAGS) -c $< -o $@
build/home/home.o: src/home/home.c src/home/home.h
	$(CC) $(CFLAGS) -c $< -o $@
build/cd/cd.o: src/cd/cd.c src/cd/cd.h
	$(CC) $(CFLAGS) -c $< -o $@
build/pwd/pwd.o: src/pwd/pwd.c src/pwd/pwd.h
	$(CC) $(CFLAGS) -c $< -o $@
build/ls/ls.o: src/ls/ls.c src/ls/ls.h
	$(CC) $(CFLAGS) -c $< -o $@
build/cat/cat.o: src/cat/cat.c src/cat/cat.h
	$(CC) $(CFLAGS) -c $< -o $@
build/run/run.o: src/run/run.c src/run/run.h
	$(CC) $(CFLAGS) -c $< -o $@
build/exec/exec.o: src/exec/exec.c src/exec/exec.h
	$(CC) $(CFLAGS) -c $< -o $@
build/alias/alias.o: src/alias/alias.c src/alias/alias.h
	$(CC) $(CFLAGS) -c $< -o $@
build/edit/edit.o: src/edit/edit.c src/edit/edit.h
	$(CC) $(CFLAGS) -c $< -o $@
build/editor/editor.o: src/editor/editor.c src/editor/editor.h
	$(CC) $(CFLAGS) -c $< -o $@
build/mkdir/mkdir.o: src/mkdir/mkdir.c src/mkdir/mkdir.h
	$(CC) $(CFLAGS) -c $< -o $@
build/rm/rm.o: src/rm/rm.c src/rm/rm.h
	$(CC) $(CFLAGS) -c $< -o $@
build/mv/mv.o: src/mv/mv.c src/mv/mv.h
	$(CC) $(CFLAGS) -c $< -o $@
build/cp/cp.o: src/cp/cp.c src/cp/cp.h
	$(CC) $(CFLAGS) -c $< -o $@
build/touch/touch.o: src/touch/touch.c src/touch/touch.h
	$(CC) $(CFLAGS) -c $< -o $@
build/snippet/snippet.o: src/snippet/snippet.c src/snippet/snippet.h
	$(CC) $(CFLAGS) -c $< -o $@
build/link/link.o: src/link/link.c src/link/link.h
	$(CC) $(CFLAGS) -c $< -o $@
build/hub/hub.o: src/hub/hub.c src/hub/hub.h
	$(CC) $(CFLAGS) -c $< -o $@
build/make/make.o: src/make/make.c src/make/make.h
	$(CC) $(CFLAGS) -c $< -o $@
build/cook/cook.o: src/cook/cook.c src/cook/cook.h
	$(CC) $(CFLAGS) -c $< -o $@
build/sh/sh.o: src/sh/sh.c src/sh/sh.h
	$(CC) $(CFLAGS) -c $< -o $@
build/find/find.o: src/find/find.c src/find/find.h
	$(CC) $(CFLAGS) -c $< -o $@
build/bake/bake.o: src/bake/bake.c src/bake/bake.h
	$(CC) $(CFLAGS) -c $< -o $@
build/insert/insert.o: src/insert/insert.c src/insert/insert.h
	$(CC) $(CFLAGS) -c $< -o $@
build/clone/clone.o: src/clone/clone.c src/clone/clone.h
	$(CC) $(CFLAGS) -c $< -o $@
build/replace/replace.o: src/replace/replace.c src/replace/replace.h
	$(CC) $(CFLAGS) -c $< -o $@
build/find/arguments_manager.o: src/find/arguments_manager.c src/find/arguments_manager.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/importer.o: src/lang/importer.c src/lang/importer.h
	$(CC) $(CFLAGS) -c $< -o $@
