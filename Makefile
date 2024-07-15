
BUILD_TOP_DIR = .
include $(BUILD_TOP_DIR)/conf/makefile.inc

CSRCS    =
CCSRCS   =
CCSRCS  += cc_tcpcomm.cc
# CCSRCS  += cc_udpcomm.cc
CCSRCS  += cc_misc.cc
CPPSRCS += 
TARGET   = cclib.a

#LDFLAGS   += -lpthread

# ------------------------------------------------------
SUBDIRS = 

build release:
	@echo "========================="
	@echo "=  start release build  ="
	@echo "========================="
#	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i all; done
	make all
#	make install
#	make gtags

debug:
	@echo "======================="
	@echo "=  start debug build  ="
	@echo "======================="
#	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i BUILDTYPE=Debug all; done
	make BUILDTYPE=Debug all
#	make install
	make gtags

install:

gtags:
gtags_clean:

# ------------------------------------------------------
.PHONY: all clean distclean install
.PHONY: unittest_all unittest_clean unittest_distclean



all unittest_all:
	@make mkobjdir
	make $(OBJDIR)/$(TARGET)

$(OBJS): Makefile $(BUILD_TOP_DIR)/conf/makefile.inc

$(OBJDIR)/$(TARGET): $(OBJS)
	$(AR) rcs $@ $(OBJS)

clean unittest_clean distclean unittest_distclean:
#	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done
	@make cleanobjdir
	make gtags_clean
	make doxygen_clean

install:
#	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done

# ------------------------------------------------------ document
doxygen_generate: Doxyfile
	if [ ! -f Doxyfile ] ; then \
		make doxygen_create_doxyfile; \
	fi
	make doxygen_clean
	doxygen Doxyfile
doxygen_create_doxyfile:
	doxygen -g Doxyfile

doxygen_check_doxyfile:
	@doxygen -g Doxyfile_org > /dev/null
	diff Doxyfile_org Doxyfile
	rm -f Doxyfile_org

doxygen_clean:
	(cd doc; rm -rf cclib_document_html)
# ------------------------------------------------------ install packages
install_packages:
	sudo apt update
	sudo apt -y install nlohmann-json3-dev
	sudo apt -y install libwebsocketpp-dev
	sudo apt -y install libboost-all-dev
	sudo apt -y install doxygen graphviz texlive-latex-base texlive-fonts-recommended texlive-fonts-extra texlive-latex-extra

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

# ------------------------------------------------------ sample_message
sample_message:
	make BUILDTYPE=Debug all
	make BUILDTYPE=Debug _sample_message

_sample_message: $(OBJDIR)/sample_message

$(OBJDIR)/sample_message: sample_message.cc $(OBJDIR)/$(TARGET)
	$(CPP) -Wl,--start-group sample_message.cc $(OBJDIR)/$(TARGET) -Wl,--end-group -o $@

sample_message_run:
	$(OBJDIR)/sample_message

# ------------------------------------------------------ pipeexec unittest
test_pipeexec:
	make BUILDTYPE=Debug all
	echo "#define UNIT_TEST_SAMPLE" > unittest.cc
	echo "#include \"cc_pipeexec.h\"" >> unittest.cc

	make BUILDTYPE=Debug run_unittest

# ------------------------------------------------------ unittest
run_unittest:
	-$(CPP) -Wl,--start-group unittest.cc $(OBJDIR)/$(TARGET) -Wl,--end-group -o $(OBJDIR)/unittest
	-$(OBJDIR)/unittest
	rm -f unittest.cc

-include $(OBJDIR)/*/*.d
# ------------------------------------------------------ for me
-include ~/.dotfiles/.makefile.gitbase.inc

