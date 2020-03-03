CC=gcc
LD=gcc

.SUFFIXES:

TARGETS=fluid
DEPS:=$(wildcard *.h) Makefile

CFLAGS = -g -std=c99 -Wfatal-errors -Wall -pedantic -DGL_SILENCE_DEPRECATION
LFLAGS = -g

OS:=$(shell uname)
ifeq ($(OS),Darwin) # Mac OS
  GL_FLAGS=-lglfw -framework Cocoa -framework OpenGL -lpthread
  CFLAGS:=$(CFLAGS) -DAPPLE
else # Linux or other
  GL_FLAGS=-lglfw -lGL -lpthread
endif

LFLAGS:=$(LFLAGS) $(GL_FLAGS)

all : $(TARGETS) $(DEPS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

% : %.o gl_utils.o
	$(LD) $^ $(LFLAGS) -o $@

clean :
	rm -rf $(TARGETS) *.o
