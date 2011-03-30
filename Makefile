source = channel.cpp time.cpp tester.cpp pinger.cpp delay_channel.cpp
objects = time.o pinger.o delay_channel.o
executables = tester

CXX = g++
CXXFLAGS = -g --std=c++0x -pedantic -Werror -Wall -Wextra -Weffc++ -fno-default-inline -pipe -D_FILE_OFFSET_BITS=64 -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
LIBS =

all: $(executables)

tester: tester.o $(objects)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include depend

depend: $(source)
	$(CXX) $(INCLUDES) -MM $(source) > depend

.PHONY: clean
clean:
	-rm -f $(executables) depend *.o *.rpo
