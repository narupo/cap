GIT := git
CD := cd
ifeq ($(OS), Windows_NT)
	RM := del
	RMDIR := rmdir /s /q
	SEP := \\
	CPR := echo D | xcopy /E /H /Y
	CP := copy
else
	RM := rm
	RMDIR := rm -rf
	SEP := /
	CPR := cp -r
	CP := cp
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
# $(warning $(wildcard cap/*.c))

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
	build$(SEP)lang \
	build$(SEP)lang$(SEP)builtin
	$(CPR) tests_env build$(SEP)tests_env

.PHONY: cc
cc:
	$(CC) -v

SRCS := \
	build/core/config.c \
	build/core/util.c \
	build/core/alias_manager.c \
	build/core/alias_info.c \
	build/core/symlink.c \
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
	build/make/make.c \
	build/cook/cook.c \
	build/sh/sh.c \
	build/find/find.c \
	build/find/arguments_manager.c \
	build/bake/bake.c \
	build/insert/insert.c \
	build/clone/clone.c \
	build/replace/replace.c \
	build/lang/importer.c \
	build/lang/builtin/functions.c

OBJS := $(SRCS:.c=.o)

pad: build/$(LIBPAD)

build/$(LIBPAD):
	$(CD) build && \
		$(GIT) clone https://github.com/narupo/pad && \
		$(CD) pad && \
		make init && \
		make && \
		$(CD) ..$(SEP).. && \
		$(CP) build$(SEP)pad$(SEP)build$(SEP)$(LIBPAD) build$(SEP)

cap: build/app.o build/$(LIBPAD) $(OBJS)
	$(CC) $(CFLAGS) -o build/cap build/app.o $(OBJS) -lpad

tests: build/tests.o build/$(LIBPAD) $(OBJS)
	$(CC) $(CFLAGS) -o build/tests build/tests.o $(OBJS) -lpad

build/app.o: cap/app.c cap/app.h cap/core/constant.h
	$(CC) $(CFLAGS) -c $< -o $@
build/tests.o: cap/tests.c cap/tests.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/config.o: cap/core/config.c cap/core/config.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/util.o: cap/core/util.c cap/core/util.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/alias_manager.o: cap/core/alias_manager.c cap/core/alias_manager.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/alias_info.o: cap/core/alias_info.c cap/core/alias_info.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/symlink.o: cap/core/symlink.c cap/core/symlink.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/error_stack.o: cap/core/error_stack.c cap/core/error_stack.h
	$(CC) $(CFLAGS) -c $< -o $@
build/core/args.o: cap/core/args.c cap/core/args.h
	$(CC) $(CFLAGS) -c $< -o $@
build/home/home.o: cap/home/home.c cap/home/home.h
	$(CC) $(CFLAGS) -c $< -o $@
build/cd/cd.o: cap/cd/cd.c cap/cd/cd.h
	$(CC) $(CFLAGS) -c $< -o $@
build/pwd/pwd.o: cap/pwd/pwd.c cap/pwd/pwd.h
	$(CC) $(CFLAGS) -c $< -o $@
build/ls/ls.o: cap/ls/ls.c cap/ls/ls.h
	$(CC) $(CFLAGS) -c $< -o $@
build/cat/cat.o: cap/cat/cat.c cap/cat/cat.h
	$(CC) $(CFLAGS) -c $< -o $@
build/run/run.o: cap/run/run.c cap/run/run.h
	$(CC) $(CFLAGS) -c $< -o $@
build/exec/exec.o: cap/exec/exec.c cap/exec/exec.h
	$(CC) $(CFLAGS) -c $< -o $@
build/alias/alias.o: cap/alias/alias.c cap/alias/alias.h
	$(CC) $(CFLAGS) -c $< -o $@
build/edit/edit.o: cap/edit/edit.c cap/edit/edit.h
	$(CC) $(CFLAGS) -c $< -o $@
build/editor/editor.o: cap/editor/editor.c cap/editor/editor.h
	$(CC) $(CFLAGS) -c $< -o $@
build/mkdir/mkdir.o: cap/mkdir/mkdir.c cap/mkdir/mkdir.h
	$(CC) $(CFLAGS) -c $< -o $@
build/rm/rm.o: cap/rm/rm.c cap/rm/rm.h
	$(CC) $(CFLAGS) -c $< -o $@
build/mv/mv.o: cap/mv/mv.c cap/mv/mv.h
	$(CC) $(CFLAGS) -c $< -o $@
build/cp/cp.o: cap/cp/cp.c cap/cp/cp.h
	$(CC) $(CFLAGS) -c $< -o $@
build/touch/touch.o: cap/touch/touch.c cap/touch/touch.h
	$(CC) $(CFLAGS) -c $< -o $@
build/snippet/snippet.o: cap/snippet/snippet.c cap/snippet/snippet.h
	$(CC) $(CFLAGS) -c $< -o $@
build/link/link.o: cap/link/link.c cap/link/link.h
	$(CC) $(CFLAGS) -c $< -o $@
build/hub/hub.o: cap/hub/hub.c cap/hub/hub.h
	$(CC) $(CFLAGS) -c $< -o $@
build/make/make.o: cap/make/make.c cap/make/make.h
	$(CC) $(CFLAGS) -c $< -o $@
build/cook/cook.o: cap/cook/cook.c cap/cook/cook.h
	$(CC) $(CFLAGS) -c $< -o $@
build/sh/sh.o: cap/sh/sh.c cap/sh/sh.h
	$(CC) $(CFLAGS) -c $< -o $@
build/find/find.o: cap/find/find.c cap/find/find.h
	$(CC) $(CFLAGS) -c $< -o $@
build/bake/bake.o: cap/bake/bake.c cap/bake/bake.h
	$(CC) $(CFLAGS) -c $< -o $@
build/insert/insert.o: cap/insert/insert.c cap/insert/insert.h
	$(CC) $(CFLAGS) -c $< -o $@
build/clone/clone.o: cap/clone/clone.c cap/clone/clone.h
	$(CC) $(CFLAGS) -c $< -o $@
build/replace/replace.o: cap/replace/replace.c cap/replace/replace.h
	$(CC) $(CFLAGS) -c $< -o $@
build/find/arguments_manager.o: cap/find/arguments_manager.c cap/find/arguments_manager.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/importer.o: cap/lang/importer.c cap/lang/importer.h
	$(CC) $(CFLAGS) -c $< -o $@
build/lang/builtin/functions.o: cap/lang/builtin/functions.c cap/lang/builtin/functions.h
	$(CC) $(CFLAGS) -c $< -o $@
