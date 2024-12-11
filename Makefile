SHELL := bash
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

.RECIPEPREFIX = >


CC = gcc
GDB = gdb
PROJ = klondike

SRCDIR = .
OBJDIR = .objs
DEPDIR = .deps
LIBDIR = libs
LIBLINEARDIR = liblinear
STBDIR = stb
SHRDIR = shaders
TOLDIR = tinyobjloader-c
GLTFDIR = cgltf
KTXDIR = ktx

#SRCS = main.c glad.c glfw.c app_alloc.c vulk.c vulk_debug.c files.c xvect.c
#SRCS += mesh.c vulk_texture.c vulk_buffer.c
#OBJS = $(SRCS:.c=.o)

SRCS = main.c game.c debug.c
TEST_SRCS = test.c game.c debug.c

OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.c=.o))
DEPS = $(addprefix $(DEPDIR)/,$(SRCS:.c=.d))

T_OBJS = $(addprefix $(OBJDIR)/,$(TEST_SRCS:.c=.o))
T_DEPS = $(addprefix $(DEPDIR)/,$(TEST_SRCS:.c=.d))

BIN = $(SRCDIR)/$(PROJ)
TESTBIN = $(SRCDIR)/run_test

#vpath %.a $(LIBDIR)

ROOT=$(shell pwd)

CFLAGS = -Wall -std=gnu17
CFLAGS += -g3 -ggdb #-DDBUG_GL

#LIBS = -ldl -lvulkan -lGL -lX11 -lm -lpthread -lglfw -lzstd
#LIBS += -lktx -lktx_read -L $(LIBDIR)

.PHONY: all clean test


all: $(BIN)

#CFLAGS += -I$(LIBLINEARDIR)
#CFLAGS += -I$(STBDIR)
#CFLAGS += -I$(TOLDIR)
#CFLAGS += -I$(GLTFDIR)
#CFLAGS += -I$(KTXDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
> mkdir -p $(@D)
> mkdir -p $(DEPDIR)
> $(CC) $(CFLAGS) -c -o $@ $< -MMD -MF $(DEPDIR)/$(*F).d

$(OBJDIR)/%.o: $(SRCDIR)/%.c
> mkdir -p $(@D)
> mkdir -p $(DEPDIR)
> $(CC) $(CFLAGS) -c -o $@ $< -MMD -MF $(DEPDIR)/$(*F).d

$(BIN): $(OBJS)
> $(CC) $(CFLAGS) $^ -o $@ $(LIBS)

$(TESTBIN): $(T_OBJS)
> $(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
> $(RM) *.o $(OBJDIR)/*.o $(DEPDIR)/*.d $(BIN) $(TESTBIN)

#reallyclean:
#> $(MAKE) -C ktx clean

-include $(DEPS)
