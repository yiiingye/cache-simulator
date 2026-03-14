CC = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
TARGET = cache
SRCS = cache.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) a.out