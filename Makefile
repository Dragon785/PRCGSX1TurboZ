# makefile for nmake

CC = zcc
CFLAGS = -c -O2
PLATFORM = cpm

TARGET = look.com

SRCDIR = src
SRCS=$(SRCDIR)\lowgraph.c $(SRCDIR)\main.c 

OBJDIR = obj

OBJ_TMP = $(subst $(SRCDIR),$(OBJDIR),$(SRCS))
OBJS = $(OBJ_TMP:.c=.o)


.SUFFIXES :

.SUFFIXES : .c .asm .o

$(TARGET): $(OBJS)
	$(CC) +$(PLATFORM) -o $@ $(OBJS)

{$(SRCDIR)}.c{$(OBJDIR)}.o:
	@if not exist $(OBJDIR) mkdir $(OBJDIR)
	$(CC) +$(PLATFORM) $(CFLAGS) -o $@ $<

