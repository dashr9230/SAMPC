CXX = g++
RM = rm

CXXFLAGS = -s -O2 -m32
CXXFLAGS_DEBUG = -O0 -g3 -m32

all:
	$(CXX) $(CXXFLAGS) -c *.cpp
	$(CXX) $(CXXFLAGS) -o announce *.o

debug:
	$(CXX) $(CXXFLAGS_DEBUG) -c *.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -o announce *.o

.PHONY: clean

clean:
	$(RM) *.o