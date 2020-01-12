#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <csignal>
#include <condition_variable>


/** Decides if server is running. */
inline volatile std::sig_atomic_t isRunning = 0;

/** Condition variable for pinging thread.
 * Release after Server::PING_PERIOD milliseconds. */
inline std::condition_variable cv;

#endif
