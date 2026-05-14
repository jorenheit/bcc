# Acus - A C++ library for generating Brainfuck programs.
# Copyright (C) 2026 Joren Heit
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Acus build system
#
# Common targets:
#   make              build lib/libacus.a
#   make examples     build every .cc/.cpp file in examples/
#   make tests        build test executables
#   make check        build and run test executables
#   make install      install the static library and public headers
#   make uninstall    remove installed Acus files
#   make clean        remove generated build artifacts

# ---------- Tools ----------
CXX      ?= g++
AR       ?= ar
RANLIB   ?= ranlib
INSTALL  ?= install
RM       ?= rm -f
RMDIR    ?= rm -rf

# ---------- Build configuration ----------
BUILD_DIR ?= build
OBJDIR    := $(BUILD_DIR)/obj
DEPDIR    := $(BUILD_DIR)/dep
BINDIR    ?= bin
LIBDIR_LOCAL ?= lib

LIB_NAME := acus
STATIC_LIB := $(LIBDIR_LOCAL)/lib$(LIB_NAME).a

RUNTIME_TEST_TARGET     := $(BINDIR)/acus-runtime-tests
COMPILETIME_TEST_TARGET := $(BINDIR)/acus-compiletime-tests

CPPFLAGS ?=
CXXFLAGS ?= -std=c++23 -Wall
LDFLAGS  ?=
LDLIBS   ?=

CPPFLAGS += -Iinclude

# ---------- Installation configuration ----------
PREFIX     ?= /usr/local
INCLUDEDIR ?= $(PREFIX)/include
LIBDIR     ?= $(PREFIX)/lib
DESTDIR    ?=

INSTALL_INCLUDEDIR := $(DESTDIR)$(INCLUDEDIR)/acus
INSTALL_LIBDIR     := $(DESTDIR)$(LIBDIR)

# ---------- Sources ----------
# Source files are discovered automatically, but editor backup files are ignored.
LIB_SRCS := $(shell find src -type f \( -name '*.cc' -o -name '*.cpp' \) ! -name '*~' ! -name '#*#' | sort)
RUNTIME_TEST_SRC     := testsuite/testsuite.cc
COMPILETIME_TEST_SRC := testsuite/compiletime_testsuite.cc

LIB_OBJS := $(patsubst %.cc,$(OBJDIR)/%.o,$(filter %.cc,$(LIB_SRCS))) \
            $(patsubst %.cpp,$(OBJDIR)/%.o,$(filter %.cpp,$(LIB_SRCS)))
RUNTIME_TEST_OBJ := $(patsubst %.cc,$(OBJDIR)/%.o,$(RUNTIME_TEST_SRC))
COMPILETIME_TEST_OBJ := $(patsubst %.cc,$(OBJDIR)/%.o,$(COMPILETIME_TEST_SRC))

DEPS := $(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.d,$(LIB_OBJS) $(RUNTIME_TEST_OBJ) $(COMPILETIME_TEST_OBJ))

# Runtime tests are BF-heavy; keep your existing optimization preference.
$(RUNTIME_TEST_OBJ): CXXFLAGS += -O3

# ---------- Main targets ----------
.PHONY: all
all: $(STATIC_LIB)

$(STATIC_LIB): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^
	$(RANLIB) $@

.PHONY: tests runtime-tests compiletime-tests
tests: runtime-tests compiletime-tests

runtime-tests: $(RUNTIME_TEST_TARGET)

compiletime-tests: $(COMPILETIME_TEST_TARGET)

$(RUNTIME_TEST_TARGET): $(RUNTIME_TEST_OBJ) $(STATIC_LIB)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $(RUNTIME_TEST_OBJ) -L$(LIBDIR_LOCAL) -l$(LIB_NAME) $(LDLIBS) -o $@

$(COMPILETIME_TEST_TARGET): $(COMPILETIME_TEST_OBJ) $(STATIC_LIB)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $(COMPILETIME_TEST_OBJ) -L$(LIBDIR_LOCAL) -l$(LIB_NAME) $(LDLIBS) -o $@

.PHONY: check
check: tests
	./$(RUNTIME_TEST_TARGET)
	./$(COMPILETIME_TEST_TARGET)

.PHONY: examples
examples: $(STATIC_LIB)
	$(MAKE) -C examples ACUS_ROOT="$(CURDIR)" LIBDIR="$(CURDIR)/$(LIBDIR_LOCAL)" INCLUDEDIR="$(CURDIR)/include"

# ---------- Compile rules ----------
$(OBJDIR)/%.o: %.cc
	@mkdir -p $(dir $@) $(dir $(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.d,$@))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.d,$@) -MT $@ -c $< -o $@

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@) $(dir $(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.d,$@))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -MF $(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.d,$@) -MT $@ -c $< -o $@

-include $(DEPS)

# ---------- Installation ----------
#
# Public headers are installed below $(INCLUDEDIR)/acus so users can write:
#
#   #include <acus/acus.h>
#
# The source tree currently keeps the umbrella header at include/acus.h and the
# rest of the public header tree at include/acus/.  Installation copies the
# umbrella header to $(INCLUDEDIR)/acus/acus.h and copies the contents of
# include/acus/ underneath the same installed directory.
.PHONY: install
install: $(STATIC_LIB)
	$(INSTALL) -d $(INSTALL_LIBDIR)
	$(INSTALL) -m 644 $(STATIC_LIB) $(INSTALL_LIBDIR)/lib$(LIB_NAME).a
	$(INSTALL) -d $(INSTALL_INCLUDEDIR)
	$(INSTALL) -m 644 include/acus.h $(INSTALL_INCLUDEDIR)/acus.h
	@if [ -d include/acus ]; then \
		find include/acus -type d | while IFS= read -r dir; do \
			rel=$${dir#include/acus}; \
			$(INSTALL) -d "$(INSTALL_INCLUDEDIR)/$$rel"; \
		done; \
		find include/acus -type f \( -name '*.h' -o -name '*.hh' -o -name '*.hpp' -o -name '*.hxx' -o -name '*.tpp' -o -name '*.ipp' \) | while IFS= read -r file; do \
			rel=$${file#include/acus/}; \
			$(INSTALL) -m 644 "$$file" "$(INSTALL_INCLUDEDIR)/$$rel"; \
		done; \
	fi

.PHONY: uninstall
uninstall:
	$(RM) $(INSTALL_LIBDIR)/lib$(LIB_NAME).a
	$(RMDIR) $(INSTALL_INCLUDEDIR)

# ---------- Cleanup ----------
.PHONY: clean
clean:
	$(RMDIR) $(BUILD_DIR) $(BINDIR) $(LIBDIR_LOCAL)
	$(MAKE) -C examples clean ACUS_ROOT="$(CURDIR)" LIBDIR="$(CURDIR)/$(LIBDIR_LOCAL)" INCLUDEDIR="$(CURDIR)/include"
