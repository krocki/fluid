CC=gcc
LD=gcc

.SUFFIXES:

TARGETS=gl sim.so
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

%.so: %.c $(DEPS)
	$(CC) $(CFLAGS) $< -shared -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

fluid: fluid.o gl_utils.o
	$(CC) $^ $(LFLAGS) -o $@

gl: gl.o gl_utils.o sim.so
	$(CC) $^ $(LFLAGS) -o $@

clean :
	rm -rf $(TARGETS) *.o
