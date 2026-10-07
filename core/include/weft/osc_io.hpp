#pragma once
#include <string>

#include "weft/osc.hpp"

namespace weft {

// UDP OSC transport (OSC over UDP — same framing over TCP works too).
//
// Bind() listens on host:port (use "0.0.0.0" or "127.0.0.1");
// openOut() only sends to a remote host:port. poll() is non-blocking
// (0 ms timeout) — the host calls it from its timer/heartbeat.
class OscUdp {
public:
    OscUdp();
    ~OscUdp();

    bool bind(const std::string& host, int port, std::string* err);
    bool openOut(const std::string& host, int port, std::string* err);

    bool send(const OscMsg& msg);
    // Returns false if nothing available.
    bool poll(OscMsg& out);
    void close();

    int localPort() const { return localPort_; }

private:
    struct Impl;
    Impl* impl_ = nullptr;
    int localPort_ = 0;
};

}  // namespace weft
