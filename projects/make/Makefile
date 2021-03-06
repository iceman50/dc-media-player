# This is a rudimentary Makefile that compiles files from the pluginsdk & src directories into a
# shared library. Adapt to your needs.

TARGET = MediaPlayer

CC = gcc
CXX = g++

CPPFLAGS += -Wall -Wextra -g -pipe -O3 -I../../
CXXFLAGS += -std=gnu++0x
LINKFLAGS += -static-libgcc -g -O3 -shared -Wl,--add-stdcall-alias
LINKXXFLAGS += -static-libstdc++

VPATH = ../../

OUTPUT_DIR = build

OBJS = \
	pluginsdk/Config.o \
	pluginsdk/Connections.o \
	pluginsdk/Core.o \
	pluginsdk/Hooks.o \
	pluginsdk/Hubs.o \
	pluginsdk/Logger.o \
	pluginsdk/Queue.o \
	pluginsdk/Tagger.o \
	pluginsdk/UI.o \
	pluginsdk/Util.o \
	src/Dialog.o \
	src/main.o \
	src/MediaPlayers.o \
	src/MediaUtil.o \
	src/Plugin.o \
	src/stdafx.o

COMPILER_SPEC = $(shell $(CC) -dumpmachine)

ifeq ($(findstring mingw, $(COMPILER_SPEC)),)
	BINEXT =
	LIBEXT = .so
else
	BINEXT = .exe
	CPPFLAGS += -D_WIN32_WINNT=0x502 -DWINVER=0x502 -D_WIN32_IE=0x600 \
				-DNOMINMAX -DSTRICT -DWIN32_LEAN_AND_MEAN \
				-DWIN32 -D_WIN32 -DUNICODE -D_UNICODE
	LIBEXT = .dll
	OBJS += src/resource.o
	OUTPUT_DIR := $(OUTPUT_DIR)-mingw
endif

ifeq ($(findstring x86_64, $(COMPILER_SPEC)),)
	CPPFLAGS += -march=i686
	OUTPUT_DIR := $(OUTPUT_DIR)-x86
else
	OUTPUT_DIR := $(OUTPUT_DIR)-x64
endif

ifeq ($(OS), Windows_NT)
	ifeq ($(findstring Cygwin, $(SHELL)),)
		MKDIR = if not exist $(subst /,\,$1) md $(subst /,\,$1)
		RMDIR = if exist $(subst /,\,$1) rd /s /q $(subst /,\,$1)
	else
		MKDIR = mkdir -p $1
		RMDIR = $(RM) -r $1
	endif
else
	MKDIR = mkdir -p $1
	RMDIR = $(RM) -r $1
endif


ifeq ($(findstring mingw, $(COMPILER_SPEC)),)
	PDB =
else
	PDB := $(OUTPUT_DIR)/$(TARGET).pdb
endif

TARGET := $(OUTPUT_DIR)/$(TARGET)$(LIBEXT)
OBJS := $(addprefix $(OUTPUT_DIR)/, $(OBJS))

PACKAGER = $(OUTPUT_DIR)/packaging/packager$(BINEXT)
PACK_INFO = ../../packaging/info_generated.xml

all: $(TARGET) $(PDB) $(PACK_INFO)

$(TARGET): $(OBJS)
	$(CXX) $^ $(LINKFLAGS) $(LINKXXFLAGS) $(OUTPUT_OPTION)

ifneq ($(findstring mingw, $(COMPILER_SPEC)),)
$(PDB): $(TARGET)
	strip --only-keep-debug $< $(OUTPUT_OPTION) && strip $<
endif

$(PACKAGER): $(OUTPUT_DIR)/packaging/packager/packager.o
	$(CXX) $^ $(subst -shared,,$(LINKFLAGS) $(LINKXXFLAGS)) $(OUTPUT_OPTION)

$(PACK_INFO): $(PACKAGER)
	$(PACKAGER) $(PACK_INFO)

$(OUTPUT_DIR)/%.o: %.c
	$(COMPILE.c) $< $(OUTPUT_OPTION)

$(OUTPUT_DIR)/%.o: %.cpp
	$(COMPILE.cpp) $< $(OUTPUT_OPTION)

$(OUTPUT_DIR)/%.o: %.rc
	windres $< $(OUTPUT_OPTION)

$(OBJS): | $(OUTPUT_DIR)

$(PACKAGER): | $(OUTPUT_DIR)

$(OUTPUT_DIR):
	$(call MKDIR,$(OUTPUT_DIR)/packaging/packager)
	$(call MKDIR,$(OUTPUT_DIR)/pluginsdk)
	$(call MKDIR,$(OUTPUT_DIR)/src)

clean:
	$(call RMDIR,$(OUTPUT_DIR))
