# Makefile for unit tests

TESTS += test_math
TESTS += test_stringtools
TESTS += test_accumulating_iterator
TESTS += test_cast_iterator
TESTS += test_circular_iterator
TESTS += test_function_input_iterator
TESTS += test_index_iterator
TESTS += test_stride_iterator
TESTS += test_iterator_combinations

OBJECTS = $(TESTS:=.o)

#CXXFLAGS += -O3
CXXFLAGS += -g

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

CPPFLAGS += -I../..

# this adds (very slow) runtime checks for many STL functions:
CPPFLAGS += -D_GLIBCXX_DEBUG

#CPPFLAGS += -DNDEBUG

all: run_tests

run_tests: build_tests
	./main

build_tests: main

main: $(OBJECTS)

DEPENDENCIES = main $(OBJECTS)

clean:
	$(RM) $(DEPENDENCIES)

.PHONY: all build_tests run_tests clean

# rebuild everything when Makefile changes
$(DEPENDENCIES): Makefile

include ../Makefile.dependencies
