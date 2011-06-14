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
template class Series<Throughput, Delay>;
template class Series< Series<Pinger, Buffer>, Series<Throughput, Delay> >;
template class SimpleContainer< Series< Series<Pinger, Buffer>, Series<Throughput, Delay> > >;
