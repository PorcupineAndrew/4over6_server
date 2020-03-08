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

TAG := cscope.files cscope.in.out cscope.out cscope.po.out tags
.PHONY: clean tags
clean:
	rm -rf $(BINDIR) $(OBJDIR) $(TAG)

tags: 
	rm -f $(TAG)
	find . -type f -name "*.[ch]" >cscope.files
	cscope -bq
	ctags -L cscope.files --fields=+iaS --extra=+q
