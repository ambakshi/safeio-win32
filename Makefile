SHELL:=/bin/bash
UNAME:=$(shell uname -s | sed -e 's/_.*$$//ig' )
VSVER=14
ifeq ($(UNAME),CYGWIN)
DEFINES = /DNDEBUG /D_CRT_SECURE_NO_WARNINGS=1
CXX := cmd.exe /c winsup\\vs$(VSVER).bat cl /nologo 
CXXFLAGS = /favor:AMD64 /Wv:17 /volatile:iso /arch:AVX /WX /Wall /wd4820 /wd4668 /wd4710 /wd4700 /wd4777 /wd4711 $(DEFINES) /TP /MT /Fo$@ /c 
CXXOPTS = /O2 /Ox
CC := $(CXX)
CCFLAGS := /showIncludes $(CXXFLAGS)
CCOPTS := $(CXXOPTS)
LD := cmd.exe /c winsup\\vs$(VSVER).bat link /nologo 
LDFLAGS = /RELEASE /SUBSYSTEM:console /OUT:$@
else ifeq ($(UNAME),Linux)
Linux=1
else ifeq ($(UNAME),MINGW)
Mingw=1
endif
QUIET = @

#PROGRAMS = safeio cpu wait-chain
#safeio_OBJS = safeio.o

define PROG
$1_OBJ := $$(patsubst %.c,%.obj,$2)
$1_D := $$(patsubst %.c,%.d,$2)
$1_PROG := $1.exe
ALL += $$($1_PROG)
OBJS += $$($1_OBJ)
DEPS += $$($1_D)
all: $$($1_PROG)
$$($1_OBJ): $2
$$($1_PROG): $$($1_OBJ)
endef


$(eval $(call PROG,safeio,safeio.c main.c))
$(eval $(call PROG,cpu,cpu.c))
$(eval $(call PROG,wait-chain,wait-chain.c))
$(eval $(call PROG,hardlink,hardlink.c))
$(eval $(call PROG,fsctl_filesystem_get_statistics,fsctl_filesystem_get_statistics.c))

ifneq ($(DEBUG),)
$(info $(call PROG,safeio,safeio.c main.c))
$(info $(call PROG,cpu,cpu.c))
$(info $(call PROG,wait-chain,wait-chain.c))
$(eval $(call PROG,hardlink,hardlink.c))
$(eval $(call PROG,fsctl_filesystem_get_statistics,fsctl_filesystem_get_statistics.c))
endif

wait-chain_CCFLAGS = /DUNICODE=1 /wd4061 /wd4365 /wd4191 /wd4777 

cpu_CCFLAGS = /wd4477

#-include $(DEPS)


%.obj: %.cpp
	@echo Cxx $@ ...
	$(QUIET) $(CXX) $(CXXFLAGS) $(CXXOPTS) $<

%.obj: %.c
	@echo CC $@ ...
	$(QUIET) $(CC) $(CCFLAGS) $(CCOPTS) $($*_CCFLAGS) $< | sed -e 's/Note: including file:[ \t]*//g' > $*.dtmp
	@(echo "$<:" && tail -n +2 $*.dtmp ) > $*.d  && rm -f $*.dtmp
	@test -f $@ || { cat $*.d | grep -E ': (warning|error)' >&2 ; rm -f $*.d; exit 1; }

%.exe:
	@echo Link $@ ...
	$(QUIET) $(LD) $(LDFLAGS) $^

clean:
	rm -f $(ALL) $(OBJS) $(DEPS)

debug:
	$(for v,$(V), \
       $(warning $v = $($v)))
