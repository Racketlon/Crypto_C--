COMPILER = g++
CXXFLAGS = -Wall -Iinclude

TARGET = crypto
SOURCES = main.cpp src/Interpreter.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(COMPILER) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run:
	./$(TARGET) tests/hello.crypto

clean:
	rm -rf $(TARGET)
