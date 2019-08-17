#include "utilities.h"

#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
using namespace std;

#ifdef __APPLE__
#include <mach/mach.h>
#endif

static void signal_handler(int signal_number);

void register_event_handlers() {
  // On exit or when receiving certain signals such as SIGINT (Ctrl-C),
  // print the peak memory usage.
  signal(SIGABRT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGSEGV, signal_handler);
  signal(SIGINT, signal_handler);
}

void signal_handler(int signal_number) {
  // See glibc manual: "Handlers That Terminate the Process"
  static volatile sig_atomic_t handler_in_progress = 0;
  if (handler_in_progress) raise(signal_number);
  handler_in_progress = 1;
  cout << "caught signal " << signal_number << " -- exiting" << endl;
  signal(signal_number, SIG_DFL);
  raise(signal_number);
}
