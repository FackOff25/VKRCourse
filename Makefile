CXX = g++
CXXFLAGS = -w -Wall -Wextra -std=c++17 -I./include
TARGET = main.out
SOURCES = main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean

rebuild: clean all

run: $(TARGET)
	./$(TARGET)

rebuild_and_run: rebuild run