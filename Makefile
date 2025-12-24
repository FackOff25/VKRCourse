all: main.out

main.out: main.cpp
	g++ -std=c++17 -Wall -o main.out -g main.cpp `pkg-config vips-cpp --cflags --libs`

run: main.out
	./main.out