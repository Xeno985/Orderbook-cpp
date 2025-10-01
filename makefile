# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread \
           -Wno-template-id-cdtor \
           -Wno-unused-parameter \
           -Wno-unused-variable \
           -Wno-reorder
LDFLAGS = -lboost_system -lboost_thread -lssl -lcrypto -pthread
INCLUDES = -I./FTXUI/include -I./websocketpp -I./json
LIB_DIRS = -L./FTXUI/build
LIBS = -lftxui-component -lftxui-dom -lftxui-screen
SOURCES = orderbook.cpp Authenticator.cpp OrderManager.cpp Order.cpp menu.cpp
TARGET = trading_app

# Default target
all: build-ftxui $(TARGET)

# Build the target (depends on FTXUI being built first)
$(TARGET): $(SOURCES) | build-ftxui
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(INCLUDES) $(LIB_DIRS) $(LDFLAGS) $(LIBS)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Run the application (ensure FTXUI is built first)
run: $(TARGET)
	. ./.env && ./$(TARGET)

# Build the FTXUI libraries (required before building your app)
build-ftxui:
	@if [ ! -f "./FTXUI/build/libftxui-component.a" ]; then \
		echo "Building FTXUI libraries..."; \
		cd FTXUI && mkdir -p build && cd build && cmake .. -DCMAKE_CXX_STANDARD=17 && make -j4; \
	else \
		echo "FTXUI libraries already built."; \
	fi

.PHONY: all clean run build-ftxui