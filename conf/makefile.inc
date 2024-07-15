
#COMMONCFLAGS  = `pkg-config --cflags $(GTKMM3_PKGNAME)`
#COMMONCFLAGS += `pkg-config --cflags $(JSONLIB_PKGNAME)`
#COMMONCFLAGS += `pkg-config --cflags $(PYTHON_PKGNAME)`

# default value
BUILDTYPE ?= Release

CPP        = g++
CPPFLAGS   =
ifeq ($(BUILDTYPE),Release)
CPPFLAGS  += -O3
CPPFLAGS  += -D_RELEASE
else
CPPFLAGS  += -D_DEBUG
endif
CPPFLAGS  += -g -MD
CPPFLAGS  += -Wall
CPPFLAGS  += -Werror
#CPPFLAGS  += -std=gnu++11
CPPFLAGS  += -std=gnu++17
CPPFLAGS  += $(COMMONCFLAGS)

CC         = gcc
CFLAGS     = 
CFLAGS    += -g -MD
ifeq ($(BUILDTYPE),Release)
CPPFLAGS  += -O3
else
CFLAGS    += -D_DEBUG
endif
CFLAGS    += -Wall
CFLAGS    += -Werror
CFLAGS    += $(COMMONCFLAGS)

LDFLAGS   += `pkg-config --libs $(JSONLIB_PKGNAME)`
##LDFLAGS   += `pkg-config --libs $(PYTHON_PKGNAME)`
LDFLAGS   += -lpython3.10

LDFLAGS_GUI += `pkg-config --libs $(GTKMM3_PKGNAME)`

OBJDIR    = .obj
CPPOBJDIR = $(OBJDIR)/cppobjs_$(BUILDTYPE)
CCOBJDIR  = $(OBJDIR)/ccobjs_$(BUILDTYPE)
COBJDIR   = $(OBJDIR)/cobjs_$(BUILDTYPE)

COBJS     = $(addprefix $(COBJDIR)/, $(CSRCS:.c=.o))
CCOBJS    = $(addprefix $(CCOBJDIR)/, $(CCSRCS:.cc=.o))
CPPOBJS   = $(addprefix $(CPPOBJDIR)/, $(CPPSRCS:.cpp=.o))

OBJS      = $(COBJS) $(CCOBJS) $(CPPOBJS)

AR        = ar

$(CPPOBJDIR)/%.o: %.cpp
	@[ -d $(CPPOBJDIR) ]
	$(CPP) -o $@ -c $< $(CPPFLAGS)

$(CCOBJDIR)/%.o: %.cc
	@[ -d $(CCOBJDIR) ]
	$(CPP) -o $@ -c $< $(CPPFLAGS)

$(COBJDIR)/%.o: %.c
	@[ -d $(COBJDIR) ]
	$(CC) -o $@ -c $< $(CFLAGS)

target_default:
	@make release

mkobjdir:
ifeq ($(BUILDTYPE),Debug)
	rm -rf .obj/*objs_Release .obj/*objs_
else
	rm -rf .obj/*objs_Debug .obj/*_
endif
	@mkdir -p $(COBJDIR) $(CCOBJDIR) $(CPPOBJDIR)

cleanobjdir:
	rm -rf .obj

exec_with_localpythonenv:
	@echo "#!/bin/bash" > .sh_script
	@echo "source $(TARGETDIR)/bin/activate" >> .sh_script
	@echo "export PYTHONPATH=$(TARGETDIR)/python/etest" >> .sh_script
	@echo "$(COMMAND)" >> .sh_script

	@chmod +x .sh_script
	@./.sh_script
	@rm .sh_script


