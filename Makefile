# Makefile for cParser

CC = clang

CFLAGS = -g

OBJDIR = ./obj
SRCDIR = ./
BINDIR = ./


SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all : $(OBJECTS) cparser

cparser : $(OBJECTS)
	$(CC) $(CFLAGS) -o cparser $(OBJECTS)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -v $(OBJECTS)
	rm -v cparser