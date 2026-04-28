# ---------- Config ----------
CXX      := g++
CXXFLAGS := -std=c++23 -O3 -Wall -Iinclude
LDFLAGS  :=
LDLIBS   :=

TARGET      := bcc
TEST_TARGET := bcc_tests
OBJDIR      := obj

# Shared library/application sources, without app/main.cc.
COMMON_SRCS := \
	src/api/api.cc \
	src/ir/primitive.cc \
	src/types/typesystem.cc \
	src/types/type_rules.cc \
	src/types/operators.cc \
	src/types/literal.cc \
	src/core/proxy.cc \
	src/core/slot.cc \
	src/builder/builder_program.cc \
	src/builder/builder_struct.cc \
	src/builder/builder_access.cc \
	src/builder/builder_calls_control.cc \
	src/builder/builder_pointers_memory.cc \
	src/builder/builder_algorithms.cc \
	src/builder/builder_codeblocks.cc \
	src/builder/builder_framenav.cc \
	src/builder/builder_diag.cc \
	src/builder/builder_codegen.cc \
	src/builder/builder_globals.cc \
	src/builder/builder_memory.cc \
	src/builder/builder_rlvalue.cc \
	src/builder/builder_add.cc \
	src/builder/builder_sub.cc \
	src/builder/builder_divmod.cc \
	src/builder/builder_mul.cc \
	src/builder/builder_logical.cc \
	src/builder/builder_comparisons.cc \
	src/builder/builder_binop_general.cc \

MAIN_SRC := app/main.cc
TEST_SRC := testsuite/testsuite.cc

COMMON_OBJS := $(patsubst %.cc,$(OBJDIR)/%.o,$(COMMON_SRCS))
MAIN_OBJ    := $(patsubst %.cc,$(OBJDIR)/%.o,$(MAIN_SRC))
TEST_OBJ    := $(patsubst %.cc,$(OBJDIR)/%.o,$(TEST_SRC))
DEPS        := $(COMMON_OBJS:.o=.d) $(MAIN_OBJ:.o=.d) $(TEST_OBJ:.o=.d)

.PHONY: all
all: $(TARGET)

$(TARGET): $(COMMON_OBJS) $(MAIN_OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

.PHONY: tests
tests: $(TEST_TARGET)

$(TEST_TARGET): $(COMMON_OBJS) $(TEST_OBJ)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJDIR)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -MF $(@:.o=.d) -MT $@ -c $< -o $@

-include $(DEPS)

.PHONY: clean
clean:
	$(RM) -r $(OBJDIR) $(TARGET) $(TEST_TARGET)
