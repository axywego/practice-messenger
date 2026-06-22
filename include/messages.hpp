#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <memory.h>

using Bytes = std::vector<uint8_t>;

void pack(Bytes& body, const Bytes& data){
    auto length = htonl(static_cast<uint32_t>(data.size()));
    
    const uint8_t* len_ptr = reinterpret_cast<const uint8_t*>(&length);
    body.insert(body.end(), len_ptr, len_ptr + 4);

    body.insert(body.end(), data.begin(), data.end());
}

std::string unpack(size_t& offset, const Bytes& body) {
    if(offset + 4 > body.size()) 
        throw std::runtime_error("unpack: out of bounds");

    uint32_t length;
    memcpy(&length, body.data() + offset, 4);

    length = ntohl(length);

    offset += 4;

    if(offset + length > body.size()) {
        throw std::runtime_error("unpack: out of bounds reading value");
    }

    std::string value(body.begin() + offset, body.begin() + offset + length);

    offset += value.size();

    return value;
}

Bytes unpackBytes(size_t& offset, const Bytes& body) {
    if(offset + 4 > body.size()) 
        throw std::runtime_error("unpack: out of bounds");

    uint32_t length;
    memcpy(&length, body.data() + offset, 4);

    length = ntohl(length);

    offset += 4;

    if(offset + length > body.size()) 
        throw std::runtime_error("unpack: out of bounds reading value");

    Bytes value(body.begin() + offset, body.begin() + offset + length);

    offset += value.size();

    return value;
}

Bytes stringToBytes(const std::string& str){
    return Bytes(str.begin(), str.end());
}

void pack_i64(Bytes& body, int64_t value) {
    for(int i = 7; i >= 0; --i) {
        body.push_back(static_cast<uint8_t>((value >> (8 * i)) & 0xFF));
    }
}

int64_t unpack_i64(size_t& offset, const Bytes& body) {
    if(offset + 8 > body.size()) 
        throw std::runtime_error("unpack: out of bounds");

    int64_t value = 0;
    for(int i = 0; i < 8; ++i) {
        value = (value << 8) | body[offset + i];
    }
    offset += 8;
    return value;
}


template<typename Derived>
struct Serializable;

template<typename T>
struct Serializer {
    static void serialize(Bytes& body, const T& value) {
        if constexpr(std::is_same_v<T, std::string>) {
            pack(body, Bytes(value.begin(), value.end()));
        }
        else if constexpr(std::is_same_v<T, Bytes>) {
            pack(body, value);
        }
        else if constexpr(std::is_same_v<T, bool>) {
            Serializer<std::string>::serialize(body, value ? "true" : "false");
        }
        else if constexpr(std::is_same_v<T, int64_t>) {
            pack_i64(body, value);
        }
        else if constexpr(std::is_base_of_v<Serializable<T>, T>) {
            auto nested = value.serialize();
            pack(body, nested);
        }
    }
    
    static void deserialize(size_t& offset, const Bytes& body, T& value) {
        if constexpr(std::is_same_v<T, std::string>) {
            value = unpack(offset, body);
        }
        else if constexpr(std::is_same_v<T, Bytes>) {
            value = unpackBytes(offset, body);
        }
        else if constexpr(std::is_same_v<T, bool>) {
            std::string str;
            Serializer<std::string>::deserialize(offset, body, str);
            value = (str == "true");
        }
        else if constexpr(std::is_same_v<T, int64_t>) {
            value = unpack_i64(offset, body);
        }
        else if constexpr(std::is_base_of_v<Serializable<T>, T>) {
            Bytes nested = unpackBytes(offset, body);
            value = T::deserialize(nested);
        }
    }
};

template<typename T>
struct Serializer<std::vector<T>> {
    static void serialize(Bytes& body, const std::vector<T>& vec) {
        if constexpr(std::is_same_v<T, uint8_t>) {
            pack(body, vec);
        }
        else {
            uint32_t size = htonl(static_cast<uint32_t>(vec.size()));
            const uint8_t* pointer = reinterpret_cast<const uint8_t*>(&size);
            body.insert(body.end(), pointer, pointer + 4);
            for(const auto& el : vec) {
                Serializer<T>::serialize(body, el);
            }
        }
    }
    static void deserialize(size_t& offset, const Bytes& body, std::vector<T>& vec) {
        if constexpr(std::is_same_v<T, uint8_t>) {
            vec = unpackBytes(offset, body);
        }
        else {
            uint32_t size;
            memcpy(&size, body.data() + offset, 4);
            size = ntohl(size);
            offset += 4;
            vec.resize(size);
            for(auto& el : vec) {
                Serializer<T>::deserialize(offset, body, el);
            }
        }
    }
};

// args -> элементы tuple. apply распаковывает tuple на args и для каждого такого decltype берет тип, при помощи decay_t 
// убирает ссылочность и константность, и в конечном итогде применяет serialize.
template<typename... Args>
void serialize_tuple(Bytes& body, const std::tuple<Args...>& tuple) {
    std::apply([&body](const auto&... args) {
        (Serializer<std::decay_t<decltype(args)>>::serialize(body, args), ...);
    }, tuple);
}

template<typename... Args>
void deserialize_tuple(size_t& offset, const Bytes& body, std::tuple<Args...>& tuple) {
    std::apply([&offset, &body](auto&... args) {
        (Serializer<std::decay_t<decltype(args)>>::deserialize(offset, body, args), ...);
    }, tuple);
}


template<typename Derived>
struct Serializable {
    Bytes serialize() const {
        Bytes body;
        const Derived* d = static_cast<const Derived*>(this);
        auto fields = d->fields();
        serialize_tuple(body, fields);
        return body;
    }
    
    static Derived deserialize(const Bytes& body) {
        size_t offset = 0;
        Derived obj;
        auto fields = obj.fields();
        deserialize_tuple(offset, body, fields);
        return obj;
    }
};

struct AuthRequest : Serializable<AuthRequest> {
    std::string login;
    std::string password;

    auto fields() { return std::tie(login, password); }
    auto fields() const { return std::tie(login, password); }
};

struct AuthResponse : Serializable<AuthResponse> {
    bool success;
    std::string token;

    auto fields() { return std::tie(success, token); }
    auto fields() const { return std::tie(success, token); }
};

struct ChatSendRequest : Serializable<ChatSendRequest> {
    std::string token;
    std::string message;

    auto fields() { return std::tie(token, message); }
    auto fields() const { return std::tie(token, message); }
};

struct ChatIncoming : Serializable<ChatIncoming> {
    std::string sender_login;
    std::string message;

    auto fields() { return std::tie(sender_login, message); }
    auto fields() const { return std::tie(sender_login, message); }
};

struct ChatResponse : Serializable<ChatResponse> {
    bool success;

    auto fields() { return std::tie(success); }
    auto fields() const { return std::tie(success); }
};

struct FileUploadRequest : Serializable<FileUploadRequest> {
    std::string token;
    std::string file_name;
    Bytes file_data;

    auto fields() { return std::tie(token, file_name, file_data); }
    auto fields() const { return std::tie(token, file_name, file_data); }
};

struct FileUploadResponse : Serializable<FileUploadResponse> {
    bool success;

    auto fields() { return std::tie(success); }
    auto fields() const { return std::tie(success); }
};

struct FileDownloadRequest : Serializable<FileDownloadRequest> {
    std::string token;
    std::string file_name;

    auto fields() { return std::tie(token, file_name); }
    auto fields() const { return std::tie(token, file_name); }
};

struct FileDownloadResponse : Serializable<FileDownloadResponse> {
    bool success;
    Bytes file_data;

    auto fields() { return std::tie(success, file_data); }
    auto fields() const { return std::tie(success, file_data); }
};

// FOR FRIEND_REQUEST FRIEND_ACCEPT FRIEND_REJECT
struct FriendRequest : Serializable<FriendRequest> {
    std::string login;
    std::string target_login;

    auto fields() { return std::tie(login, target_login); }
    auto fields() const { return std::tie(login, target_login); }
};

struct FriendResponse : Serializable<FriendResponse> {
    bool success;
    std::string message;

    auto fields() { return std::tie(success, message); }
    auto fields() const { return std::tie(success, message); }
};

struct FriendListRequest : Serializable<FriendListRequest> {
    std::string token;

    auto fields() { return std::tie(token); }
    auto fields() const { return std::tie(token); }
};

struct FriendListResponse : Serializable<FriendListResponse> {
    std::vector<std::string> friends;
    std::vector<std::string> pending_incoming;

    auto fields() { return std::tie(friends, pending_incoming); }
    auto fields() const { return std::tie(friends, pending_incoming); }
};

struct DirectMessageRequest : Serializable<DirectMessageRequest> {
    std::string token;
    std::string recipient_login;
    std::string message;

    auto fields() { return std::tie(token, recipient_login, message); }
    auto fields() const { return std::tie(token, recipient_login, message); }
};

struct DirectMessageIncoming : Serializable<DirectMessageIncoming> {
    std::string sender_login;
    std::string message;
    int64_t timestamp;

    auto fields() { return std::tie(sender_login, message, timestamp); }
    auto fields() const { return std::tie(sender_login, message, timestamp); }
};

struct DirectMessageResponse : Serializable<DirectMessageResponse> {
    bool success;
    std::string error;

    auto fields() { return std::tie(success, error); }
    auto fields() const { return std::tie(success, error); }
};

struct HistoryRequest : Serializable<HistoryRequest> { 
    std::string token;
    std::string peer_login;

    auto fields() { return std::tie(token, peer_login); }
    auto fields() const { return std::tie(token, peer_login); }
};

struct HistoryResponse : Serializable<HistoryResponse> {
    struct Entry : Serializable<Entry> {
        std::string from_login;
        std::string message;
        int64_t timestamp;

        auto fields() { return std::tie(from_login, message, timestamp); }
        auto fields() const { return std::tie(from_login, message, timestamp); }
    };
    std::vector<Entry> messages;

    auto fields() { return std::tie(messages); }
    auto fields() const { return std::tie(messages); }
};

template<typename T>
const char* getStructName(const T&) {
    if constexpr (std::is_same_v<T, AuthRequest>) return "AuthRequest";
    else if constexpr (std::is_same_v<T, AuthResponse>) return "AuthResponse";
    else if constexpr (std::is_same_v<T, ChatSendRequest>) return "ChatSendRequest";
    else if constexpr (std::is_same_v<T, ChatIncoming>) return "ChatIncoming";
    else if constexpr (std::is_same_v<T, ChatResponse>) return "ChatResponse";
    else if constexpr (std::is_same_v<T, FileUploadRequest>) return "FileUploadRequest";
    else if constexpr (std::is_same_v<T, FileUploadResponse>) return "FileUploadResponse";
    else if constexpr (std::is_same_v<T, FileDownloadRequest>) return "FileDownloadRequest";
    else if constexpr (std::is_same_v<T, FileDownloadResponse>) return "FileDownloadResponse";
    else if constexpr (std::is_same_v<T, FriendRequest>) return "FriendRequest";
    else if constexpr (std::is_same_v<T, FriendResponse>) return "FriendResponse";
    else if constexpr (std::is_same_v<T, FriendListRequest>) return "FriendListRequest";
    else if constexpr (std::is_same_v<T, FriendListResponse>) return "FriendListResponse";
    else if constexpr (std::is_same_v<T, DirectMessageRequest>) return "DirectMessageRequest";
    else if constexpr (std::is_same_v<T, DirectMessageIncoming>) return "DirectMessageIncoming";
    else if constexpr (std::is_same_v<T, DirectMessageResponse>) return "DirectMessageResponse";
    else if constexpr (std::is_same_v<T, HistoryRequest>) return "HistoryRequest";
    else if constexpr (std::is_same_v<T, HistoryResponse>) return "HistoryResponse";
    else return "Unknown";
}