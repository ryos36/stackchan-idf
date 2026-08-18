// SPDX-FileCopyrightText: 2026 Kenta IDA <fuga@fugafuga.org>
// SPDX-License-Identifier: BSL-1.0

#include "base64.hpp"

#include <array>

#include <mbedtls/base64.h>

namespace stackchan::conversation::base64 {

std::vector<std::uint8_t> encode(std::span<const std::uint8_t> input)
{
    std::vector<std::uint8_t> out(encoded_size(input.size()));
    std::size_t written = 0;
    const int rc = mbedtls_base64_encode(out.data(), out.size(), &written, input.data(), input.size());
    if (rc != 0) {
        return {};
    }
    out.resize(written);
    return out;
}

tl::expected<std::size_t, ConversationError> encode_into(std::span<const std::uint8_t> input, std::span<char> out)
{
    std::size_t written = 0;
    const int rc = mbedtls_base64_encode(reinterpret_cast<unsigned char*>(out.data()), out.size(), &written,
                                         input.data(), input.size());
    if (rc != 0) {
        return tl::unexpected{ConversationError::OutOfMemory};
    }
    return written;
}

namespace {

// Base64 の 1 文字を 6 bit の値に戻す表。使えない文字は 0xFF。
constexpr std::array<std::uint8_t, 256> make_decode_table()
{
    std::array<std::uint8_t, 256> t{};
    t.fill(0xFF);
    constexpr std::string_view chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (std::size_t i = 0; i < chars.size(); ++i) {
        t[static_cast<unsigned char>(chars[i])] = static_cast<std::uint8_t>(i);
    }
    return t;
}
constexpr auto kDecodeTable = make_decode_table();

} // namespace

tl::expected<std::vector<std::uint8_t>, ConversationError> decode(std::string_view input)
{
    std::vector<std::uint8_t> out;
    out.reserve(input.size() / 4 * 3 + 3);

    std::uint32_t acc = 0;
    int bits = 0;
    for (const char ch : input) {
        const auto c = static_cast<unsigned char>(ch);
        if (c == '=') { break; }
        if (c == '\r' || c == '\n' || c == ' ' || c == '\t') { continue; }
        const std::uint8_t v = kDecodeTable[c];
        if (v == 0xFF) { return tl::unexpected{ConversationError::ProtocolError}; }
        acc = (acc << 6) | v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<std::uint8_t>((acc >> bits) & 0xFF));
        }
    }
    return out;
}

} // namespace stackchan::conversation::base64
