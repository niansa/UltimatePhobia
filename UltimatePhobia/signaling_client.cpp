#include "signaling_client.hpp"
#include "json.hpp"

#include <windows.h>
#include <winhttp.h>

#include <limits>
#include <string>
#include <utility>

namespace signaling {

namespace {

class InternetHandle {
public:
    InternetHandle() noexcept = default;
    explicit InternetHandle(HINTERNET handle) noexcept : handle_(handle) {}

    ~InternetHandle() { reset(); }

    InternetHandle(const InternetHandle&) = delete;
    InternetHandle& operator=(const InternetHandle&) = delete;

    InternetHandle(InternetHandle&& other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}

    InternetHandle& operator=(InternetHandle&& other) noexcept {
        if (this != &other) {
            reset();
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }

    void reset(HINTERNET handle = nullptr) noexcept {
        if (handle_ != nullptr)
            WinHttpCloseHandle(handle_);
        handle_ = handle;
    }

    [[nodiscard]]
    HINTERNET get() const noexcept {
        return handle_;
    }

    [[nodiscard]]
    explicit operator bool() const noexcept {
        return handle_ != nullptr;
    }

private:
    HINTERNET handle_ = nullptr;
};

struct HttpResponse {
    DWORD status_code{};
    std::string body;
};

std::wstring utf8_to_wide(std::string_view input) {
    if (input.empty())
        return {};

    const int required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(), static_cast<int>(input.size()), nullptr, 0);

    if (required <= 0)
        throw SignalingError("Invalid UTF-8 string");

    std::wstring out(static_cast<std::size_t>(required), L'\0');

    const int written = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(), static_cast<int>(input.size()), out.data(), required);

    if (written != required)
        throw SignalingError("UTF-8 to UTF-16 conversion failed");

    return out;
}

std::string wide_to_utf8(std::wstring_view input) {
    if (input.empty())
        return {};

    const int required = WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr);

    if (required <= 0)
        return "WideCharToMultiByte failed";

    std::string out(static_cast<std::size_t>(required), '\0');

    const int written = WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), out.data(), required, nullptr, nullptr);

    if (written != required)
        return "WideCharToMultiByte failed";

    return out;
}

std::string format_win32_error(DWORD error) {
    LPWSTR buffer = nullptr;

    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

    const DWORD size = FormatMessageW(flags, nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

    if (size == 0 || buffer == nullptr)
        return "Win32 error " + std::to_string(error);

    std::wstring message(buffer, size);
    LocalFree(buffer);

    while (!message.empty() && (message.back() == L'\r' || message.back() == L'\n'))
        message.pop_back();

    auto utf8 = wide_to_utf8(message);
    if (utf8.empty())
        return "Win32 error " + std::to_string(error);
    return utf8;
}

[[noreturn]]
void throw_last_error(std::string_view where) {
    const DWORD error = GetLastError();
    throw SignalingError(std::string(where) + " failed: " + format_win32_error(error));
}

[[noreturn]]
void throw_http_error(DWORD status_code, std::string_view body) {
    std::string message = "HTTP ";
    message += std::to_string(status_code);

    if (!body.empty()) {
        message += ": ";
        constexpr std::size_t max_len = 256;
        if (body.size() > max_len) {
            message.append(body.substr(0, max_len));
            message += "...";
        } else {
            message.append(body);
        }
    }

    throw SignalingError(message);
}

std::wstring normalize_base_path(std::wstring path) {
    if (path == L"/")
        return {};

    while (!path.empty() && path.back() == L'/')
        path.pop_back();

    return path;
}

std::wstring join_path(const std::wstring& base, std::wstring_view relative) {
    if (base.empty())
        return std::wstring(relative);

    std::wstring out = base;
    if (!relative.empty() && relative.front() != L'/')
        out.push_back(L'/');
    out.append(relative.data(), relative.size());
    return out;
}

void append_hex_escape(std::string& out, unsigned char ch) {
    static constexpr char hex[] = "0123456789ABCDEF";
    out += "\\u00";
    out.push_back(hex[(ch >> 4) & 0x0F]);
    out.push_back(hex[ch & 0x0F]);
}

void append_json_string(std::string& out, std::string_view value) {
    out.push_back('"');

    for (unsigned char ch : value) {
        switch (ch) {
        case '\"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (ch < 0x20)
                append_hex_escape(out, ch);
            else
                out.push_back(static_cast<char>(ch));
            break;
        }
    }

    out.push_back('"');
}

std::string make_room_payload(std::uint64_t room_id, std::string_view ip, std::uint16_t port) {
    std::string body;
    body.reserve(ip.size() + 64);

    // Intentionally serialize roomId as a JSON string.
    // This preserves full uint64_t precision with a JS backend and also fits the Worker code.
    body += "{\"roomId\":\"";
    body += std::to_string(room_id);
    body += "\",\"ip\":";
    append_json_string(body, ip);
    body += ",\"port\":";
    body += std::to_string(port);
    body += '}';

    return body;
}

DWORD query_status_code(HINTERNET request) {
    DWORD status_code = 0;
    DWORD size = sizeof(status_code);

    if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status_code, &size,
                             WINHTTP_NO_HEADER_INDEX))
        throw_last_error("WinHttpQueryHeaders");

    return status_code;
}

std::string read_response_body(HINTERNET request) {
    std::string body;

    for (;;) {
        DWORD available = 0;
        if (!WinHttpQueryDataAvailable(request, &available))
            throw_last_error("WinHttpQueryDataAvailable");

        if (available == 0)
            break;

        const std::size_t old_size = body.size();
        body.resize(old_size + available);

        DWORD read = 0;
        if (!WinHttpReadData(request, body.data() + old_size, available, &read))
            throw_last_error("WinHttpReadData");

        body.resize(old_size + read);
    }

    return body;
}

static std::optional<std::uint64_t> try_get_u64(const nlohmann::json& j) {
    if (j.is_number_unsigned()) {
        return j.get<std::uint64_t>();
    }

    if (j.is_number_integer()) {
        const auto v = j.get<std::int64_t>();
        if (v >= 0) {
            return static_cast<std::uint64_t>(v);
        }
    }

    return std::nullopt;
}

std::optional<Endpoint> try_parse_endpoint(const nlohmann::json& obj) {
    if (!obj.is_object())
        return std::nullopt;

    const auto ip_it = obj.find("ip");
    const auto port_it = obj.find("port");

    if (ip_it == obj.end() || port_it == obj.end())
        return std::nullopt;

    if (!ip_it->is_string())
        return std::nullopt;

    const auto port_u64 = try_get_u64(*port_it);
    if (!port_u64)
        return std::nullopt;

    if (*port_u64 > std::numeric_limits<std::uint16_t>::max())
        return std::nullopt;

    Endpoint ep;
    ep.ip = ip_it->get<std::string>();
    ep.port = static_cast<std::uint16_t>(*port_u64);
    return ep;
}

std::optional<Endpoint> parse_optional_endpoint_json(const std::string& json) {
    const auto obj = nlohmann::json::parse(json, nullptr, false);
    if (obj.is_discarded())
        return std::nullopt;

    return try_parse_endpoint(obj);
}

RoomState parse_room_state_json(const std::string& json) {
    RoomState state;

    const auto obj = nlohmann::json::parse(json, nullptr, false);
    if (obj.is_discarded() || !obj.is_object())
        return state;

    if (const auto host_it = obj.find("host"); host_it != obj.end())
        state.host = try_parse_endpoint(*host_it);

    if (const auto clients_it = obj.find("clients"); clients_it != obj.end() && clients_it->is_array()) {
        state.clients.reserve(clients_it->size());

        for (const auto& client_el : *clients_it)
            if (auto endpoint = try_parse_endpoint(client_el))
                state.clients.push_back(std::move(*endpoint));
    }

    if (const auto created_it = obj.find("createdAt"); created_it != obj.end())
        if (auto created = try_get_u64(*created_it))
            state.created_at_ms = *created;

    return state;
}

} // namespace

struct SignalingClient::Impl {
    explicit Impl(std::wstring base_url) {
        URL_COMPONENTSW parts{};
        parts.dwStructSize = sizeof(parts);
        parts.dwSchemeLength = static_cast<DWORD>(-1);
        parts.dwHostNameLength = static_cast<DWORD>(-1);
        parts.dwUrlPathLength = static_cast<DWORD>(-1);
        parts.dwExtraInfoLength = static_cast<DWORD>(-1);

        if (!WinHttpCrackUrl(base_url.c_str(), 0, 0, &parts))
            throw_last_error("WinHttpCrackUrl");

        if (parts.nScheme != INTERNET_SCHEME_HTTP && parts.nScheme != INTERNET_SCHEME_HTTPS)
            throw SignalingError("Only http and https base URLs are supported");

        if (parts.lpszHostName == nullptr || parts.dwHostNameLength == 0)
            throw SignalingError("Base URL is missing a host");

        if (parts.dwExtraInfoLength != 0)
            throw SignalingError("Base URL must not contain a query string or fragment");

        host_.assign(parts.lpszHostName, parts.dwHostNameLength);
        port_ = parts.nPort;
        secure_ = (parts.nScheme == INTERNET_SCHEME_HTTPS);

        if (parts.lpszUrlPath != nullptr && parts.dwUrlPathLength != 0) {
            base_path_.assign(parts.lpszUrlPath, parts.dwUrlPathLength);
            base_path_ = normalize_base_path(std::move(base_path_));
        }

        session_.reset(WinHttpOpen(L"signaling-client/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));

        if (!session_)
            throw_last_error("WinHttpOpen");

        connect_.reset(WinHttpConnect(session_.get(), host_.c_str(), port_, 0));
        if (!connect_)
            throw_last_error("WinHttpConnect");
    }

    [[nodiscard]]
    HttpResponse request(std::wstring_view method, std::wstring_view path, std::string_view body = {}) const {
        const std::wstring method_w(method);
        const std::wstring path_w(path);

        InternetHandle request_handle(WinHttpOpenRequest(connect_.get(), method_w.c_str(), path_w.c_str(), nullptr, WINHTTP_NO_REFERER,
                                                         WINHTTP_DEFAULT_ACCEPT_TYPES, secure_ ? WINHTTP_FLAG_SECURE : 0));

        if (!request_handle)
            throw_last_error("WinHttpOpenRequest");

        std::wstring headers;
        LPCWSTR headers_ptr = WINHTTP_NO_ADDITIONAL_HEADERS;
        DWORD headers_len = 0;
        LPVOID body_ptr = WINHTTP_NO_REQUEST_DATA;
        DWORD body_len = 0;

        if (!body.empty()) {
            headers = L"Content-Type: application/json\r\n";
            headers_ptr = headers.c_str();
            headers_len = static_cast<DWORD>(headers.size());
            body_ptr = const_cast<char *>(body.data());
            body_len = static_cast<DWORD>(body.size());
        }

        if (!WinHttpSendRequest(request_handle.get(), headers_ptr, headers_len, body_ptr, body_len, body_len, 0))
            throw_last_error("WinHttpSendRequest");

        if (!WinHttpReceiveResponse(request_handle.get(), nullptr))
            throw_last_error("WinHttpReceiveResponse");

        HttpResponse response;
        response.status_code = query_status_code(request_handle.get());
        response.body = read_response_body(request_handle.get());

        if (response.status_code < 200 || response.status_code >= 300)
            throw_http_error(response.status_code, response.body);

        return response;
    }

    void register_host(std::uint64_t room_id, std::string_view ip, std::uint16_t port) {
        const auto payload = make_room_payload(room_id, ip, port);
        const auto path = join_path(base_path_, L"/register");
        (void)request(L"POST", path, payload);
    }

    [[nodiscard]]
    std::optional<Endpoint> join_room(std::uint64_t room_id, std::string_view ip, std::uint16_t port) {
        const auto payload = make_room_payload(room_id, ip, port);
        const auto path = join_path(base_path_, L"/join");
        const auto response = request(L"POST", path, payload);
        return parse_optional_endpoint_json(response.body);
    }

    [[nodiscard]]
    RoomState poll_room(std::uint64_t room_id) const {
        std::wstring relative = L"/poll/";
        relative += std::to_wstring(room_id);

        const auto path = join_path(base_path_, relative);
        const auto response = request(L"GET", path);
        return parse_room_state_json(response.body);
    }

    InternetHandle session_;
    InternetHandle connect_;
    std::wstring host_;
    INTERNET_PORT port_{};
    bool secure_{false};
    std::wstring base_path_;
};

SignalingClient::SignalingClient(std::string_view base_url_utf8) : impl_(std::make_unique<Impl>(utf8_to_wide(base_url_utf8))) {}

SignalingClient::SignalingClient(std::wstring_view base_url_wide) : impl_(std::make_unique<Impl>(std::wstring(base_url_wide))) {}

SignalingClient::~SignalingClient() = default;
SignalingClient::SignalingClient(SignalingClient&&) noexcept = default;
SignalingClient& SignalingClient::operator=(SignalingClient&&) noexcept = default;

void SignalingClient::register_host(std::uint64_t room_id, std::string_view ip, std::uint16_t port) { impl_->register_host(room_id, ip, port); }

std::optional<Endpoint> SignalingClient::join_room(std::uint64_t room_id, std::string_view ip, std::uint16_t port) {
    return impl_->join_room(room_id, ip, port);
}

RoomState SignalingClient::poll_room(std::uint64_t room_id) const { return impl_->poll_room(room_id); }

} // namespace signaling
