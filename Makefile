CXXFLAGS += -std=c++11

all: Ueb3Aufg2.bf

Ueb3Aufg2.bf: bfg_Ueb3Aufg2
	./bfg_Ueb3Aufg2

bfg_Ueb3Aufg2: Ueb3Aufg2.o bf/generator.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	rm -f bfg_Ueb3Aufg2
	rm -f *.o
	rm -f bf/*.o
