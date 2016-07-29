CXXFLAGS += -std=c++14 -Wall
TESTLIBS += -lboost_unit_test_framework

# Build compiler
bin/bfc: bf/frontend.o
	@test -d bin || mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Build generator example
bin/bfg_example: generator_example.o bf/generator.o
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

bin/test_generator: test/generator_tests.o bf/generator.o
	@test -d bin || mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TESTLIBS)

bin/test_compiler: test/compiler_tests.o bf/compiler.o bf/generator.o
	@test -d bin || mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TESTLIBS)

clean:
	rm -f *.o bf/*.o test/*.o
