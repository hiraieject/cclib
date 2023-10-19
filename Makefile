
BUILD_TOP_DIR = .
include $(BUILD_TOP_DIR)/conf/makefile.inc

CSRCS    =
CSRCS   += c_message.c
CCSRCS   =
CCSRCS  += cc_message.cc
CCSRCS  += cc_thread.cc
CCSRCS  += cc_tcpcomm.cc
# CCSRCS  += cc_udpcomm.cc
CPPSRCS += 
TARGET   = cclib.a

LDFLAGS   += -lpthread

# ------------------------------------------------------
.PHONY: all clean distclean install
.PHONY: unittest_all unittest_clean unittest_distclean

all unittest_all:
	@make mkobjdir
	make $(OBJDIR)/$(TARGET)

$(OBJS): Makefile ../../conf/makefile.inc

$(OBJDIR)/$(TARGET): $(OBJS)
	$(AR) rcs $@ $(OBJS)

clean unittest_clean distclean unittest_distclean:
	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done
	@make cleanobjdir

install:
	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done

# ------------------------------------------------------ sample_tcpserver/client
SAMPLETCP_PORT = 5000
SAMPLETCP_CMAX = 10
SAMPLETCP_IP   = 127.0.0.1

sampletcp:
	make BUILDTYPE=Debug all
	make BUILDTYPE=Debug _sampletcp
_sampletcp: $(OBJDIR)/sample_tcpserver $(OBJDIR)/sample_tcpclient

sample_tcpserver_run:
	make sample_tcpserver_kill
	sudo $(OBJDIR)/sample_tcpserver -p$(SAMPLETCP_PORT) -m$(SAMPLETCP_CMAX) &
sample_tcpserver_kill:
	-sudo killall sample_tcpserver
	sleep 1

sample_tcpclient_run:
	$(OBJDIR)/sample_tcpclient -p$(SAMPLETCP_PORT) -i$(SAMPLETCP_IP)

$(OBJDIR)/sample_tcpserver: sample_tcpserver.cc sample_tcpserver_main.cc $(OBJDIR)/$(TARGET)
	$(CPP) -Wl,--start-group sample_tcpserver.cc sample_tcpserver_main.cc $(OBJDIR)/$(TARGET) -Wl,--end-group -o $@

$(OBJDIR)/sample_tcpclient: sample_tcpclient.cc sample_tcpclient_main.cc $(OBJDIR)/$(TARGET)
	$(CPP) -Wl,--start-group sample_tcpclient.cc sample_tcpclient_main.cc $(OBJDIR)/$(TARGET) -Wl,--end-group -o $@

# ------------------------------------------------------ sample_signalhandler
sample_signalhandler:
	make BUILDTYPE=Debug all
	make BUILDTYPE=Debug _sample_signalhandler
_sample_signalhandler: $(OBJDIR)/sample_signalhandler

sample_signalhandler_run: $(OBJDIR)/sample_signalhandler
	$(OBJDIR)/sample_signalhandler

$(OBJDIR)/sample_signalhandler : sample_signalhandler.cc
	$(CPP) -Wl,--start-group sample_signalhandler.cc $(OBJDIR)/$(TARGET) -Wl,--end-group -o $@

# ------------------------------------------------------ sample_try_exception
sample_try_exception:
	make BUILDTYPE=Debug all
	make BUILDTYPE=Debug _sample_try_exception
_sample_try_exception: $(OBJDIR)/sample_try_exception

$(OBJDIR)/sample_try_exception : sample_try_exception.cc
	$(CPP) -Wl,--start-group sample_try_exception.cc $(OBJDIR)/$(TARGET) -Wl,--end-group -o $@

sample_try_exception_run:
	-$(OBJDIR)/sample_try_exception
	-$(OBJDIR)/sample_try_exception -r
	-$(OBJDIR)/sample_try_exception -l
	-$(OBJDIR)/sample_try_exception -b
	-$(OBJDIR)/sample_try_exception -i
	-$(OBJDIR)/sample_try_exception -o
	-$(OBJDIR)/sample_try_exception -L
	-$(OBJDIR)/sample_try_exception -O
	-$(OBJDIR)/sample_try_exception -u
	-$(OBJDIR)/sample_try_exception -d
	-$(OBJDIR)/sample_try_exception -R
	-$(OBJDIR)/sample_try_exception -c
	-$(OBJDIR)/sample_try_exception -C


# ------------------------------------------------------ for me
gcommit:
	(cd ../; make gcommit)
gpull:
	(cd ../; make gpull)


-include $(OBJDIR)/*/*.d

