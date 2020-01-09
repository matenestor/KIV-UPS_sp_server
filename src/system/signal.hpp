#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <csignal>


inline volatile std::sig_atomic_t isRunning = 0;

#endif
