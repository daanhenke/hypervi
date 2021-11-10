CONFIG ?= debug

DIR_MK := $(realpath $(dir $(current_mk)))
DIR_SOURCE := $(dir $(DIR_MK))
DIR_BUILD := $(DIR_SOURCE)build/$(CONFIG)
DIR_OBJ := $(DIR_BUILD)/obj
DIR_DIST := $(DIR_BUILD)/dist

NASM=nasm
CC=clang
CXX=clang++

FREESTANDING_CFLAGS := \
	-I $(DIR_SOURCE) \
	-fno-stack-protector \
	-fshort-wchar \
	-mno-red-zone