CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-format-truncation -Wno-stringop-truncation

SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = code

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean