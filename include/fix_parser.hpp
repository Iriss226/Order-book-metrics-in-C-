#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

// HW 8, part 1 — a high-performance FIX parser (SOH-delimited tag=value; tags 11/55/54/38/44).
struct NewOrder {
    const char* clordid; int clordid_len;   // tag 11
    char symbol[16]; char side;             // tags 55, 54
    uint32_t qty; double price;             // tags 38, 44
};
// Single pass, no per-message allocation. Return false on a malformed message.
inline bool parse_new_order(const char* buf, int len, NewOrder& out) {
    if (buf == nullptr || len <= 0) return false;

    NewOrder parsed{};
    bool has_message_type = false;
    bool has_clordid = false;
    bool has_symbol = false;
    bool has_side = false;
    bool has_qty = false;
    bool has_price = false;

    const auto parse_integer = [](const char* begin, const char* end,
                                  uint64_t& value) {
        if (begin == end) return false;
        value = 0;
        for (const char* p = begin; p != end; ++p) {
            if (*p < '0' || *p > '9') return false;
            const uint64_t digit = static_cast<uint64_t>(*p - '0');
            if (value > (std::numeric_limits<uint64_t>::max() - digit) / 10) return false;
            value = value * 10 + digit;
        }
        return true;
    };

    const auto parse_price = [](const char* begin, const char* end, double& value) {
        if (begin == end) return false;
        long double result = 0.0L;
        bool seen_digit = false;
        bool seen_decimal = false;
        long double fractional_place = 0.1L;
        for (const char* p = begin; p != end; ++p) {
            if (*p == '.' && !seen_decimal) {
                seen_decimal = true;
                continue;
            }
            if (*p < '0' || *p > '9') return false;
            seen_digit = true;
            const unsigned digit = static_cast<unsigned>(*p - '0');
            if (seen_decimal) {
                result += static_cast<long double>(digit) * fractional_place;
                fractional_place *= 0.1L;
            } else {
                result = result * 10.0L + digit;
            }
            if (!std::isfinite(result)) return false;
        }
        if (!seen_digit || result <= 0.0L ||
            result > static_cast<long double>(std::numeric_limits<double>::max())) return false;
        value = static_cast<double>(result);
        return std::isfinite(value);
    };

    const char* cursor = buf;
    const char* const message_end = buf + len;
    while (cursor < message_end) {
        const char* field_end = cursor;
        while (field_end < message_end && *field_end != '\x01') ++field_end;
        if (field_end == message_end || field_end == cursor) return false;

        const char* equals = cursor;
        while (equals < field_end && *equals != '=') ++equals;
        if (equals == cursor || equals == field_end) return false;

        uint64_t tag = 0;
        if (!parse_integer(cursor, equals, tag)) return false;
        const char* value = equals + 1;
        const char* const value_end = field_end;
        const std::size_t value_length = static_cast<std::size_t>(value_end - value);

        switch (tag) {
        case 35:
            if (has_message_type || value_length != 1 || *value != 'D') return false;
            has_message_type = true;
            break;
        case 11:
            if (has_clordid || value_length == 0 ||
                value_length > static_cast<std::size_t>(std::numeric_limits<int>::max())) return false;
            parsed.clordid = value;
            parsed.clordid_len = static_cast<int>(value_length);
            has_clordid = true;
            break;
        case 55:
            if (has_symbol || value_length == 0 || value_length >= sizeof(parsed.symbol)) return false;
            std::memcpy(parsed.symbol, value, value_length);
            parsed.symbol[value_length] = '\0';
            has_symbol = true;
            break;
        case 54:
            if (has_side || value_length != 1 || (*value != '1' && *value != '2')) return false;
            parsed.side = *value;
            has_side = true;
            break;
        case 38: {
            uint64_t quantity = 0;
            if (has_qty || !parse_integer(value, value_end, quantity) ||
                quantity == 0 || quantity > std::numeric_limits<uint32_t>::max()) return false;
            parsed.qty = static_cast<uint32_t>(quantity);
            has_qty = true;
            break;
        }
        case 44:
            if (has_price || !parse_price(value, value_end, parsed.price)) return false;
            has_price = true;
            break;
        default:
            break;
        }

        cursor = field_end + 1;
    }

    if (!has_message_type || !has_clordid || !has_symbol || !has_side ||
        !has_qty || !has_price) return false;
    out = parsed;
    return true;
}
