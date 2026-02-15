all: main.out

bus.o: include/interface_bus.hpp include/bus.hpp src/bus.cpp
	g++ -std=c++17 -Wall -c -o bus.o -g src/bus.cpp

main.out: main.cpp bus.o
	g++ -std=c++17 -Wall -o main.out -g main.cpp bus.o

clean:
	rm -f *.out *.o
	
run: main.out
	./main.out

rebuild: clean main.out

rerun: rebuild run