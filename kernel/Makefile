NAME           := kernel
SRCS           := framework/main.c $(shell find -L ./src/ -name "*.c")
INC_PATH       := include/ framework/

export AM_HOME := $(PWD)/../abstract-machine
ifeq ($(ARCH),)
export ARCH    := x86_64-qemu
endif

include $(AM_HOME)/Makefile
include ../oslabs.mk
image: git

check: checker
	@build/checker < test/log.txt

checker:
	@g++ test/checker.cc -o build/checker

build-test: git
	@gcc $(shell find src/ -name "*.c") \
		 $(shell find test/ -name "*.c") \
		 -Iinclude -Iframework -Itest -DTEST -lpthread \
		 -o build/test -ggdb

test: build-test
	@build/test