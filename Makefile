CC=gcc
CFLAGS=-I./include -Wall
SRCDIR=src
OBJDIR=obj
BINDIR=bin

SOURCES=$(wildcard $(SRCDIR)/*.c)
OBJECTS=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))
TARGET=$(BINDIR)/keylogger.exe

$(TARGET): $(OBJECTS)
    @mkdir -p $(BINDIR)
    $(CC) $^ -o $@ -luser32

$(OBJDIR)/%.o: $(SRCDIR)/%.c
    @mkdir -p $(OBJDIR)
    $(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
    rm -rf $(OBJDIR) $(BINDIR)