CXXFLAGS += -std=c++14 -Wall
TESTLIBS += -lboost_unit_test_framework

.PHONY: all
all: bfg_example.bf

# Run generator example
bfg_example.bf: bfg_example
	./bfg_example

# Build generator example
bfg_example: generator_example.o bf/generator.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Tests
.PHONY: test
test: test_generator test_compiler
	@for test in $^; do \
	    echo "----------------------------------------"; \
	    echo "Test module: $$test"; \
	    ./$$test; \
	done

test_generator: test/generator_tests.o bf/generator.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TESTLIBS)

test_compiler: test/compiler_tests.o bf/compiler.o bf/generator.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TESTLIBS)

clean:
	rm -f bfg_example test_generator test_compiler
	rm -f *.o bf/*.o test/*.o
