CXXFLAGS += -std=c++14 -Wall
TESTLIBS += -lboost_unit_test_framework

.PHONY: all
all: Ueb3Aufg2.bf

# Example Brainfuck program
Ueb3Aufg2.bf: bfg_Ueb3Aufg2
	./bfg_Ueb3Aufg2

bfg_Ueb3Aufg2: Ueb3Aufg2.o bf/generator.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Tests
.PHONY: test
test: test_generator test_compiler
	@for test in $^; do \
	    echo "----------------------------------------"; \
	    echo "Test module: $$test"; \
	    ./$$test; \
	done

test_generator: test/generator.o bf/generator.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TESTLIBS)

test_compiler: test/compiler.o bf/compiler.o bf/generator.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TESTLIBS)

clean:
	rm -f bfg_Ueb3Aufg2 test_generator test_compiler
	rm -f *.o bf/*.o test/*.o
