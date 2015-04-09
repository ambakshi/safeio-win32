
UNAME:=$(shell uname -s | sed -e 's/_.*$$//ig' )
VSVER=12
ifeq ($(UNAME),CYGWIN)
DEFINES = /DNDEBUG /D_CRT_SECURE_NO_WARNINGS=1
CXX := cmd.exe /c winsup\\vs$(VSVER).bat cl /nologo 
CXXFLAGS = /favor:AMD64 /volatile:iso /arch:AVX /WX /Wall /wd4820 /wd4668 /wd4710 /wd4700 $(DEFINES) /TP /MT /Fo$@ /c 
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
$1_PROG := $1.exe
ALL += $$($1_PROG)
OBJS += $$($1_OBJ)
all: $$($1_PROG)
$$($1_PROG): $$($1_OBJ)
endef


$(eval $(call PROG,safeio,safeio.c main.c))
$(eval $(call PROG,cpu,cpu.c))
$(eval $(call PROG,wait-chain,wait-chain.c))

ifneq ($(DEBUG),)
$(info $(call PROG,safeio,safeio.c main.c))
$(info $(call PROG,cpu,cpu.c))
$(info $(call PROG,wait-chain,wait-chain.c))
endif

wait-chain_CCFLAGS = /DUNICODE=1 /wd4061 /wd4365 /wd4191 /wd4711 

%.obj: %.cpp
	@echo Cxx $@ ...
	$(QUIET) $(CXX) $(CXXFLAGS) $(CXXOPTS) $<

%.obj: %.c
	@echo Cc $@ ...
	$(QUIET) $(CC) $(CCFLAGS) $(CCOPTS) $($*_CCFLAGS) $< | sed -e 's/Note: including file:[ \t]*//g' > $*.dtmp
	@ (echo "$<:" && tail -n +2 $*.dtmp ) > $*.d  && rm -f $*.dtmp

%.exe:
	@echo Link $@ ...
	$(QUIET) $(LD) $(LDFLAGS) $^

clean:
	rm -f $(ALL) $(OBJS)

debug:
	$(for v,$(V), \
       $(warning $v = $($v)))
