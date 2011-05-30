source = time.cpp tester.cpp pinger.cpp delay_channel.cpp printer_channel.cpp throughput_channel.cpp buffer_channel.cpp stats_channel.cpp series_channel.cpp
objects = time.o pinger.o delay_channel.o printer_channel.o throughput_channel.o buffer_channel.o stats_channel.o series_channel.o
executables = tester

CXX = g++
CXXFLAGS = -g -O2 --std=c++0x -pedantic -Werror -Wall -Wextra -fno-default-inline -pipe -D_FILE_OFFSET_BITS=64 -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
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
