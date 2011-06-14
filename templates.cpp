#include "simple_container.hpp"
#include "series.hpp"
#include "pinger.hpp"
#include "printer.hpp"

#include "series.cpp"
#include "simple_container.cpp"

using namespace std;

template class SimpleContainer< Series<Pinger, Printer> >;
template class Series< Pinger, Printer >;
template class priority_queue<Event, deque<Event>, Event>;
