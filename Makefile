# Basic Makefile structure
CXX = g++
CXXFLAGS = -std=c++17 `wx-config --cxxflags`
LIBS = `wx-config --libs`

# The target executable
filemanager: MainFrame.o App.o
	$(CXX) MainFrame.o App.o $(LIBS) -o filemanager

MainFrame.o: MainFrame.cpp MainFrame.h
	$(CXX) $(CXXFLAGS) -c MainFrame.cpp

App.o: App.cpp MainFrame.h
	$(CXX) $(CXXFLAGS) -c App.cpp

clean:
	rm -f *.o filemanager