JACK_STUFF += jack_simpleprocessor jack_dynamic_inputs jack_dynamic_outputs
JACK_STUFF += jack_minimal

SNDFILE_STUFF += audiofile_simpleprocessor

EXECUTABLES += $(SNDFILE_STUFF) $(JACK_STUFF)

MEX_FILES = mex_simpleprocessor.mex

OPT ?= -O3

CXXFLAGS += $(OPT)
CXXFLAGS += -g

LDLIBS += -lpthread

# show all warnings
CXXFLAGS += -Wall -Wextra
CXXFLAGS += -pedantic
# warnings are errors
CXXFLAGS += -pedantic-errors
CXXFLAGS += -Werror
# even more warnings:
CXXFLAGS += -Wpointer-arith
CXXFLAGS += -Wcast-align
CXXFLAGS += -Wwrite-strings
CXXFLAGS += -Wredundant-decls
CXXFLAGS += -Wconversion
CXXFLAGS += -Wshadow
CXXFLAGS += -Wold-style-cast
CXXFLAGS += -Wlong-long
#CXXFLAGS += -Wno-long-long
CXXFLAGS += -Wconversion
#CXXFLAGS += -Winline

CPPFLAGS += -I../.. -D_REENTRANT

#### no more setting below here ####

FLEXTPATH ?= /usr/local/src/flext

# without this, intermediate .o files are generated:
.SUFFIXES:
.SUFFIXES: .cpp .o

no-debug: CPPFLAGS += -DNDEBUG
no-debug: all

# this adds (very slow) runtime checks for many STL functions:
debug: CPPFLAGS += -D_GLIBCXX_DEBUG
debug: all

all: $(EXECUTABLES)

.PHONY: debug no-debug all

$(JACK_STUFF): LDLIBS += -ljack

$(SNDFILE_STUFF): LDLIBS += -lsndfile

PD_EXT_DIR = pd-linux/release-single

PD_EXTERNALS = $(PD_EXT_DIR)/simpleprocessor~.pd_linux

pd: CPPFLAGS += -DNDEBUG
pd: $(PD_EXTERNALS)

.PHONY: pd

$(PD_EXT_DIR)/simpleprocessor~.pd_linux: flext_simpleprocessor.cpp simpleprocessor.h

$(PD_EXTERNALS): $(FLEXTPATH) package.txt
	CFLAGS=-I../.. $(FLEXTPATH)/build.sh pd gcc

$(FLEXTPATH):
	@test -d $(FLEXTPATH) || \
		( echo \"$(FLEXTPATH)\" not found! Set FLEXTPATH! ; false )

mex: CPPFLAGS += -DNDEBUG
mex: $(MEX_FILES)

mex-double: CPPFLAGS += -DMEX_USE_DOUBLE
mex-double: mex

.PHONY: mex mex-double

%.mex: %.cpp
	mkoctfile --mex $< $(CPPFLAGS)
	$(RM) $*.o

clean:
	$(RM) $(EXECUTABLES) $(OBJECTS)
	test -d $(FLEXTPATH) && $(FLEXTPATH)/build.sh pd gcc clean || true
	rmdir pd-linux || true
	$(RM) $(MEX_FILES)

.PHONY: clean

# rebuild everything when Makefile changes
$(OBJECTS) $(EXECUTABLES) $(PD_EXTERNALS) $(MEX_FILES): Makefile

DEPENDENCIES = $(EXECUTABLES) $(OBJECTS)

include ../Makefile.dependencies
