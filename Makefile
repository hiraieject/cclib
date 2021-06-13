
TARGET   = yos_code_template
CSRCS    = 
CCSRCS   = main.cc cc_udpcomm.cc cc_thread.cc
LIBS     = -lpthread

CFLAGS  += -g -MD
CFLAGS  += -Wall
CFLAGS  += -Werror

CFLAGS  += -DCC_UDPCOMM_DBGPR
CFLAGS  += -DCC_THREAD_DBGPR
CFLAGS  += -DCC_FIFO_DBGPR

OBJDIR   = objs
OBJS     = $(addprefix $(OBJDIR)/, $(CCSRCS:.cc=.o)) $(addprefix $(OBJDIR)/, $(CSRCS:.c=.o))

CPP       = g++
CFLAGS   += -std=gnu++11


.PHONY: all clean kill test archive server client objdir diff

all: objdir $(TARGET)
	make gtags

objdir:
	@mkdir -p $(OBJDIR)

$(TARGET): $(OBJS)
	$(CPP) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

	@echo -n '### source code total line count = '
#	@cat *.c *.cc *.h | wc -l
	@cat *.cc *.h | wc -l
	@echo ------------------------------------------------------------

$(OBJS): Makefile

$(OBJDIR)/%.o: %.cc
	@[ -d $(OBJDIR) ]
	$(CPP) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: gtags clean_gtags
gtags:
	@echo "do gtags"
	@-gtags
clean_gtags:
	rm -f GPATH GRTAGS GTAGS


test:
	gdb $(TARGET)

