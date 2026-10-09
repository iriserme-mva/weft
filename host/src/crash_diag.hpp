// crash_diag.hpp — diagnostic instrumentation for the weft_smoke CLI.
//
// Two helpers, both meant to be cheap and portable across the 3 CI OSes:
//
//   weftSmokeStep("label")  — print a step marker to stderr (goes into the CI
//                             log, NOT the smoke_out.json stdout). The last
//                             marker printed before a crash identifies which
//                             JUCE load stage faulted.
//
//   weftInstallCrashHandler() — install SIGSEGV/SIGABRT/SIGFPE handlers that
//                             dump a native backtrace to stderr, so the CI log
//                             names the exact function/line on a segfault.
//                             POSIX (Linux/macOS) uses <execinfo.h>; Windows
//                             has no execinfo so it installs a plain handler
//                             that at least names the signal.
//
// This file is diagnostic-only: it is included by smoke.cpp / JuceBackend.cpp
// and adds no runtime dependency to the production host (which will drop it
// once the VST3 load is verified working on all 3 OSes).

#pragma once

#include <cstdio>
#include <cstdarg>

#if defined(_WIN32)
// Windows: no execinfo. A minimal handler that names the signal.
#include <csignal>
#include <cstdlib>

namespace weft_smoke_diag {
static void winCrashHandler(int sig) {
    std::fprintf(stderr, "\n[weft-smoke] Windows crash: signal %d (no backtrace on this platform)\n", sig);
    std::fflush(stderr);
    std::_exit(128 + sig);
}
static inline void weftInstallCrashHandler() {
    std::signal(SIGSEGV, winCrashHandler);
    std::signal(SIGABRT, winCrashHandler);
    std::signal(SIGFPE,  winCrashHandler);
}
}  // namespace weft_smoke_diag

#else
// POSIX (Linux / macOS): full backtrace via <execinfo.h>.
#include <execinfo.h>
#include <csignal>
#include <cstdlib>
#include <unistd.h>

namespace weft_smoke_diag {
static void posxCrashHandler(int sig) {
    void* frames[64];
    const int n = backtrace(frames, 64);
    std::fprintf(stderr, "\n[weft-smoke] caught signal %d — backtrace:\n", sig);
    std::fflush(stderr);
    backtrace_symbols_fd(frames, n, STDERR_FILENO);
    std::_exit(128 + sig);
}
static inline void weftInstallCrashHandler() {
    std::signal(SIGSEGV, posxCrashHandler);
    std::signal(SIGABRT, posxCrashHandler);
    std::signal(SIGFPE,  posxCrashHandler);
}
}  // namespace weft_smoke_diag

#endif

// Step marker. Always flushed so it lands in the CI log even on a hard crash.
static inline void weftSmokeStep(const char* label) {
    std::fprintf(stderr, "[weft-smoke] step: %s\n", label);
    std::fflush(stderr);
}
