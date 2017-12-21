ifdef COVERALLS_REPO_TOKEN
CXXFLAGS += -std=c++14 -Wall -O0 -g --coverage
else
CXXFLAGS += -std=c++14 -Wall -O2
endif

BFC_LIBS := -lboost_program_options
TESTLIBS := -lboost_unit_test_framework

COMP_OBJ := bf/compiler.o \
            bf/expression_visitor.o \
            bf/generator.o \
            bf/instruction_visitor.o
GEN_OBJ  := bf/generator.o

BFC_PREFIX ?= ~/.local/bin

# Build compiler
bin/bfc: bf/frontend.o $(COMP_OBJ)
	@test -d bin || mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(BFC_LIBS)

# Build generator example
bin/bfg_example: generator_example.o $(GEN_OBJ)
	@test -d bin || mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Tests
.PHONY: test
test: bin/test_generator bin/test_compiler
	@for test in $^; do \
	    echo "----------------------------------------"; \
	    echo "Test module: $$test"; \
	    ./$$test; \
	done

bin/test_generator: test/generator_tests.o $(GEN_OBJ)
	@test -d bin || mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TESTLIBS)

bin/test_compiler: test/compiler_tests.o $(COMP_OBJ)
	@test -d bin || mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TESTLIBS)

.PHONY: install
install: bin/bfc
	@test -d $(BFC_PREFIX) || mkdir -p $(BFC_PREFIX)
	cp bin/bfc $(BFC_PREFIX)

clean:
	rm -f *.o bf/*.o test/*.o
	rm -f bf/*.gcno test/*.gcno
	rm -f bf/*.gcda test/*.gcda
