#pragma once
#include <cstdint>

// HW 8, part 2 — fast uint64 -> decimal text. Write the digits into out; return the length.
inline int u64toa(std::uint64_t v, char* out) {
    char reversed[20];
    int length = 0;
    do {
        reversed[length++] = static_cast<char>('0' + v % 10);
        v /= 10;
    } while (v != 0);

    for (int i = 0; i < length; ++i) out[i] = reversed[length - i - 1];
    return length;
}
