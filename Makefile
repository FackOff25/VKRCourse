all: main.out

main.out: main.cpp
	g++ -std=c++17 -Wall -o main.out -g main.cpp

clean:
	rm -f *.out
	
run: main.out
	./main.out

rebuild: clean main.out

rerun: rebuild run