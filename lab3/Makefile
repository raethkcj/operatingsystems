all: main

main: main.cpp Process.cpp
	$(CXX) $(CXXFLAGS) -o main main.cpp Process.cpp -std=c++11

debug: CXXFLAGS += -DDEBUG
debug: main

clean:
	rm -i main
