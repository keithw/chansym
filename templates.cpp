#include "simple_container.hpp"
#include "series.hpp"
#include "pinger.hpp"
#include "printer.hpp"
#include "buffer.hpp"
#include "throughput.hpp"
#include "delay.hpp"

#include "series.cpp"
#include "simple_container.cpp"

using namespace std;

template class priority_queue<Event, deque<Event>, Event>;

template class Series<Pinger, Buffer>;
template class Series<Delay, Printer>;
template class Series< Series<Pinger, Buffer>, Series<Delay, Printer> >;
template class SimpleContainer< Series< Series<Pinger, Buffer>, Series<Delay, Printer> > >;
