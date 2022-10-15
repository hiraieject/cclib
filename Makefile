
TARGET   = yos_code_template
CSRCS    = cmain.c
CCSRCS   = main.cc cc_udpcomm.cc cc_thread.cc
LIBS     = -lpthread

CPPFLAGS  += -g -MD
CPPFLAGS  += -Wall
CPPFLAGS  += -Werror

CPPFLAGS  += -DCC_UDPCOMM_ENB_DBGPR
CPPFLAGS  += -DCC_THREAD_ENB_DBGPR
CPPFLAGS  += -DCC_FIFO_ENB_DBGPR

CFLAGS  += -g -MD
CFLAGS  += -Wall
CFLAGS  += -Werror

CPPOBJDIR   = objs_cpp
CPPOBJS     = $(addprefix $(CPPOBJDIR)/, $(CCSRCS:.cc=.o))

COBJDIR   = objs_c
COBJS     = $(addprefix $(COBJDIR)/, $(CSRCS:.c=.o))

CPP       = g++
CPPFLAGS   += -std=gnu++11

CC       = g++
#CFLAGS   += -std=c99


.PHONY: all clean kill test archive server client objdir diff

all: objdir $(TARGET)
	make gtags

objdir:
	@mkdir -p $(CPPOBJDIR) $(COBJDIR)

$(TARGET): $(COBJS) $(CPPOBJS)
	$(CPP) -o $(TARGET) $(COBJS) $(CPPOBJS) $(LDFLAGS) $(LIBS)

	@echo -n '### source code total line count = '
#	@cat *.c *.cc *.h | wc -l
	@cat *.cc *.h | wc -l
	@echo ------------------------------------------------------------

$(COBJS) $(CPPOBJS): Makefile

$(CPPOBJDIR)/%.o: %.cc
	@[ -d $(CPPOBJDIR) ]
	$(CPP) -o $@ -c $< $(CPPFLAGS)

$(COBJDIR)/%.o: %.c
	@[ -d $(COBJDIR) ]
	$(CC) -o $@ -c $< $(CFLAGS)

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

run:
	./$(TARGET)

-include $(COBJDIR)/*.d
-include $(CPPOBJDIR)/*.d

