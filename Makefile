# Use Clang
#CXX       = clang++
#CXXFLAGS += -stdlib=libc++

CXXFLAGS += -std=c++11 -Wall
TESTLIBS += -lboost_unit_test_framework


all: Ueb3Aufg2.bf

# Example Brainfuck program
Ueb3Aufg2.bf: bfg_Ueb3Aufg2
	./bfg_Ueb3Aufg2

bfg_Ueb3Aufg2: Ueb3Aufg2.o bf/generator.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)


# Tests
test: test_generator
	./test_generator

test_generator: test/generator.o bf/generator.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(TESTLIBS)


clean:
	rm -f bfg_Ueb3Aufg2 test_generator
	rm -f *.o bf/*.o test/*.o
