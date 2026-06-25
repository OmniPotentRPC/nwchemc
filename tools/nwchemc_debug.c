/**
 * Lightweight stack traces via libc backtrace (execinfo). Optional C++
 * backward.hpp path can be added later; this keeps driver/tests pure C/C11.
 */
#define _POSIX_C_SOURCE 200809L
#include "nwchemc_debug.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__linux__) || defined(__APPLE__)
#include <execinfo.h>
#define NWCHEMC_HAVE_BACKTRACE 1
#endif

static void on_fatal_signal(int sig) {
  const char *name = "signal";
  if (sig == SIGSEGV)
    name = "SIGSEGV";
  else if (sig == SIGABRT)
    name = "SIGABRT";
  else if (sig == SIGFPE)
    name = "SIGFPE";
  fprintf(stderr, "\nnwchemc_debug: caught %s (%d)\n", name, sig);
  nwchemc_debug_print_backtrace("fatal signal");
  _exit(128 + sig);
}

void nwchemc_debug_print_backtrace(const char *context) {
  fprintf(stderr, "nwchemc_debug: backtrace (%s)\n",
          context ? context : "unknown");
#if NWCHEMC_HAVE_BACKTRACE
  void *frames[64];
  int n = backtrace(frames, 64);
  if (n <= 0) {
    fprintf(stderr, "  (backtrace unavailable)\n");
    return;
  }
  char **syms = backtrace_symbols(frames, n);
  if (!syms) {
    fprintf(stderr, "  (backtrace_symbols failed, %d frames)\n", n);
    return;
  }
  for (int i = 0; i < n; ++i)
    fprintf(stderr, "  #%02d %s\n", i, syms[i]);
  free(syms);
#else
  fprintf(stderr, "  (execinfo backtrace not supported on this platform)\n");
#endif
}

void nwchemc_debug_install_handlers(void) {
  static int installed = 0;
  if (installed)
    return;
  installed = 1;
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = on_fatal_signal;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESETHAND;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGABRT, &sa, NULL);
  sigaction(SIGFPE, &sa, NULL);
  fprintf(stderr, "nwchemc_debug: signal handlers installed (backtrace on crash)\n");
}
