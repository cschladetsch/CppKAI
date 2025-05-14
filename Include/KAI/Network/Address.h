#pragma once

#include <KAI/Network/Config.h>

#include <functional>
#include <sstream>
#include <string>

KAI_NET_BEGIN

struct MacAddress {
    MacAddress() = default;
    MacAddress(const std::string& text) : text_(text) {}

    const std::string& Text() const { return text_; }

    std::string ToString() const { return text_; }

    friend bool operator==(MacAddress const& A, MacAddress const& B) {
        return A.text_ == B.text_;
    }

    friend bool operator!=(MacAddress const& A, MacAddress const& B) {
        return !(A == B);
    }

    size_t Hash() const {
        // Use std::hash instead of boost::hash for simplicity
        return std::hash<std::string>()(text_);
    }

   private:
    std::string text_;
};

struct IpAddress {
    IpAddress() = default;
    IpAddress(const std::string& text) : text_(text) {}

    const std::string& Text() const { return text_; }

    // Convert to string for use with RakNet
    std::string ToString() const { return text_; }

    // Parse IP address from string
    static IpAddress FromString(const std::string& str) {
        return IpAddress(str);
    }

    // Create a localhost address
    static IpAddress Localhost() { return IpAddress("127.0.0.1"); }

    // Create a broadcast address
    static IpAddress Broadcast() { return IpAddress("255.255.255.255"); }

    // Get port from combined address:port string
    static int GetPort(const std::string& addressWithPort,
                       int defaultPort = 0) {
        size_t pos = addressWithPort.find(":");
        if (pos != std::string::npos) {
            std::string portStr = addressWithPort.substr(pos + 1);
            try {
                return std::stoi(portStr);
            } catch (...) {
                return defaultPort;
            }
        }
        return defaultPort;
    }

    // Get address part from combined address:port string
    static std::string GetAddress(const std::string& addressWithPort) {
        size_t pos = addressWithPort.find(":");
        if (pos != std::string::npos) {
            return addressWithPort.substr(0, pos);
        }
        return addressWithPort;
    }

    // Comparison operators
    friend bool operator==(IpAddress const& A, IpAddress const& B) {
        return A.text_ == B.text_;
    }

    friend bool operator!=(IpAddress const& A, IpAddress const& B) {
        return !(A == B);
    }

    // Hash function for unordered containers
    size_t Hash() const { return std::hash<std::string>()(text_); }

   private:
    std::string text_;
};

// Hash functions for STL containers
inline std::size_t hash_value(const IpAddress& addr) { return addr.Hash(); }

inline std::size_t hash_value(const MacAddress& addr) { return addr.Hash(); }

KAI_NET_END

// STL hash specialization
namespace std {
template <>
struct hash<kai::net::IpAddress> {
    std::size_t operator()(const kai::net::IpAddress& addr) const {
        return addr.Hash();
    }
};

template <>
struct hash<kai::net::MacAddress> {
    std::size_t operator()(const kai::net::MacAddress& addr) const {
        return addr.Hash();
    }
};
}  // namespace std
