#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace signaling {

struct Endpoint {
    std::string ip;
    std::uint16_t port{};
};

struct RoomState {
    std::optional<Endpoint> host;
    std::vector<Endpoint> clients;
    std::optional<std::uint64_t> created_at_ms;
};

class SignalingError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class SignalingClient {
public:
    // Throws SignalingError on invalid URL or WinHTTP init failure.
    explicit SignalingClient(std::string_view base_url_utf8);
    explicit SignalingClient(std::wstring_view base_url_wide);
    ~SignalingClient();

    SignalingClient(SignalingClient&&) noexcept;
    SignalingClient& operator=(SignalingClient&&) noexcept;

    SignalingClient(const SignalingClient&) = delete;
    SignalingClient& operator=(const SignalingClient&) = delete;

    // Throws SignalingError on transport / HTTP / parse failure.
    void register_host(std::uint64_t room_id, std::string_view ip, std::uint16_t port);

    // Returns the host endpoint if present, std::nullopt if host is null/not yet registered.
    // Throws SignalingError on transport / HTTP / parse failure.
    [[nodiscard]]
    std::optional<Endpoint> join_room(std::uint64_t room_id, std::string_view ip, std::uint16_t port);

    // Mirrors GET /poll/:roomId
    // Throws SignalingError on transport / HTTP / parse failure.
    [[nodiscard]]
    RoomState poll_room(std::uint64_t room_id) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace signaling
