CXX       = clang++
CXXFLAGS += -stdlib=libc++ -std=c++1y -Wall

all: Ueb3Aufg2.bf

Ueb3Aufg2.bf: bfg_Ueb3Aufg2
	./bfg_Ueb3Aufg2

bfg_Ueb3Aufg2: Ueb3Aufg2.o bf/generator.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f bfg_Ueb3Aufg2
	rm -f *.o
	rm -f bf/*.o
