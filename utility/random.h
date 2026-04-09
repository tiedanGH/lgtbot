// Copyright (c) 2024-present, Chang Liu <github.com/slontia>. All rights reserved.
//
// This source code is licensed under LGPLv2 (found in the LICENSE file).

#pragma once

#include <cstdint>
#include <iterator>
#include <random>
#include <string_view>

// Returns a seeded mt19937 using FNV-1a hash of the seed string.
// Produces identical results across platforms (unlike std::seed_seq).
// If seed is empty, uses std::random_device for non-deterministic output.
inline std::mt19937 MakeRng(const std::string_view seed)
{
    if (seed.empty()) {
        return std::mt19937(std::random_device{}());
    }
    uint64_t hash = 14695981039346656037ULL;
    for (const unsigned char c : seed) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    return std::mt19937(static_cast<uint32_t>(hash ^ (hash >> 32)));
}

// Cross-platform deterministic Fisher-Yates shuffle.
// std::shuffle with uniform_int_distribution produces platform-dependent results;
// this implementation uses simple modulo reduction instead.
template <typename RandomIt>
void SeededShuffle(RandomIt first, RandomIt last, std::mt19937& g)
{
    using diff_t = typename std::iterator_traits<RandomIt>::difference_type;
    for (diff_t i = last - first - 1; i > 0; --i) {
        const diff_t j = static_cast<diff_t>(g() % static_cast<uint32_t>(i + 1));
        std::iter_swap(first + i, first + j);
    }
}

// Cross-platform deterministic uniform integer in [min, max].
// std::uniform_int_distribution produces platform-dependent results.
inline uint32_t RandInt(std::mt19937& g, const uint32_t min, const uint32_t max)
{
    return min + g() % (max - min + 1);
}
