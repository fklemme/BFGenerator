CXXFLAGS += -std=c++14 -O2 -Wall
BFC_LIBS := -lboost_program_options
TESTLIBS := -lboost_unit_test_framework

COMP_OBJ := bf/compiler.o \
            bf/expression_visitor.o \
            bf/generator.o \
            bf/instruction_visitor.o
GEN_OBJ  := bf/generator.o

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

clean:
	rm -f *.o bf/*.o test/*.o
