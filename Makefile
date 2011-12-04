source = tester.cpp pinger.cpp printer.cpp simple_container.cpp series.cpp buffer.cpp throughput.cpp delay.cpp ensemble_container.cpp stochastic_loss.cpp jitter.cpp isender.cpp channel.cpp diverter.cpp utility_ensemble.cpp embeddable_ensemble.cpp intermittent.cpp timequantize.cpp value_iterator.cpp
objects = pinger.o printer.o simple_container.o series.o buffer.o throughput.o delay.o ensemble_container.o stochastic_loss.o jitter.o isender.o channel.o diverter.o utility_ensemble.o embeddable_ensemble.o intermittent.o timequantize.o value_iterator.o
executables = tester

CXX = g++
CXXFLAGS = -g -O3 -std=c++0x -pedantic -Werror -Wall -Wextra -Weffc++ -fno-default-inline -pipe -D_FILE_OFFSET_BITS=64 -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
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
