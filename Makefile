CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
LDLIBS := -lncurses
TARGET := expenses
SOURCES := $(shell find src -name '*.cpp')
OBJECTS := $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean
