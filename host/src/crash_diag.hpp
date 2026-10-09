// crash_diag.hpp — diagnostic instrumentation for the weft_smoke CLI.
//
// Two helpers, both meant to be cheap and portable across the 3 CI OSes:
//
//   weft_smoke_diag::weftSmokeStep("label") — print a step marker to stderr
//       (goes into the CI log, NOT the smoke_out.json stdout). The last
//       marker printed before a crash identifies which JUCE load stage faulted.
//
//   weft_smoke_diag::weftInstallCrashHandler() — install SIGSEGV/SIGABRT/SIGFPE
//       handlers that dump a native backtrace to stderr, so the CI log names
//       the exact function/line on a segfault. POSIX (Linux/macOS) uses
//       <execinfo.h>; Windows has no execinfo so it installs a plain handler
//       that at least names the signal.
//
// This file is diagnostic-only: it is included by smoke.cpp / JuceBackend.cpp
// and adds no runtime dependency to the production host (which will drop it
// once the VST3 load is verified working on all 3 OSes).
//
// Portability notes:
//   - std::_Exit (from <cstdlib>) is the async-signal-safe process terminator;
//     std::_exit does not exist in <cstdlib> (POSIX _exit lives in <unistd.h>,
//     and is not in namespace std on any platform we build).
//   - weftSmokeStep lives INSIDE namespace weft_smoke_diag; call sites use the
//     qualified name.

#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>

#if defined(_WIN32)
// Windows: no execinfo. A minimal handler that names the signal.
#include <csignal>

namespace weft_smoke_diag {

static void winCrashHandler(int sig) {
    std::fprintf(stderr,
                 "\n[weft-smoke] Windows crash: signal %d "
                 "(no backtrace on this platform)\n", sig);
    std::fflush(stderr);
    std::_Exit(128 + sig);
}

static inline void weftInstallCrashHandler() {
    std::signal(SIGSEGV, winCrashHandler);
    std::signal(SIGABRT, winCrashHandler);
    std::signal(SIGFPE,  winCrashHandler);
}

}  // namespace weft_smoke_diag

#else
// POSIX (Linux / macOS): full backtrace via <execinfo.h>.
#include <csignal>
#include <execinfo.h>
#include <unistd.h>

namespace weft_smoke_diag {

static void posxCrashHandler(int sig) {
    void* frames[64];
    const int n = backtrace(frames, 64);
    // Not strictly async-signal-safe, but this is diagnostic-only and we
    // terminate immediately after.
    std::fprintf(stderr, "\n[weft-smoke] caught signal %d — backtrace:\n", sig);
    std::fflush(stderr);
    backtrace_symbols_fd(frames, n, STDERR_FILENO);
    std::_Exit(128 + sig);
}

static inline void weftInstallCrashHandler() {
    std::signal(SIGSEGV, posxCrashHandler);
    std::signal(SIGABRT, posxCrashHandler);
    std::signal(SIGFPE,  posxCrashHandler);
}

}  // namespace weft_smoke_diag

#endif

namespace weft_smoke_diag {

// Step marker. Always flushed so it lands in the CI log even on a hard crash.
static inline void weftSmokeStep(const char* label) {
    std::fprintf(stderr, "[weft-smoke] step: %s\n", label);
    std::fflush(stderr);
}

// std::string overload for dynamically built step labels.
static inline void weftSmokeStep(const std::string& label) {
    std::fprintf(stderr, "[weft-smoke] step: %s\n", label.c_str());
    std::fflush(stderr);
}

}  // namespace weft_smoke_diag
