CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = main
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

test:
	@echo "No tests defined yet. Add your test commands here."

clean:
	rm -f $(TARGET)
