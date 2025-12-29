#pragma once

#include <sstream>
#include <string>

void LogString(std::string s); // Defined in utils.cpp

// Logger gathers the message parts in a string stream and the destructor logs the accumulated message.
// Automatically prepends a timestamp and appends a newline (via `LogString`, actually).
// Sample usage: Log() << "Hello, " << 42 << "!";
// Equivalent to `log_format("Hello, %d!", 42)` but with stream syntax.
class Log {
public:
    explicit Log() : buffer(std::ostringstream{}) {
    }

    template<typename T>
    Log& operator<<(const T& v) {
        buffer << v;
        return *this;
    }

    ~Log() {
        LogString(buffer.str());
    }

private:
    std::ostringstream buffer;
};
