CC := gcc
CFLAGS := -Wall -O3

SRCDIR := src
OBJDIR := obj
BINDIR := bin
$(shell [ ! -d $(OBJDIR) ] && mkdir $(OBJDIR))
$(shell [ ! -d $(BINDIR) ] && mkdir $(BINDIR))

TARGET := $(BINDIR)/4over6_server
SRCS = $(sort $(wildcard $(SRCDIR)/*.c))
HEADS = $(sort $(wildcard $(SRCDIR)/*.h))
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(sort $(wildcard $(SRCDIR)/*.c)))
DLIBS = -lpthread

$(TARGET):$(OBJS) $(HEADS)
	$(CC) $(CFLAGS) -o $@ $^ $(DLIBS)

$(OBJDIR)/%.o:$(SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(BINDIR) $(OBJDIR)
