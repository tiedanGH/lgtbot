#pragma once

#include <cstdio>
#include <string>

// Length-prefixed frames: 4-byte big-endian length + payload (UTF-8 JSON).
bool WriteJsonFrame(FILE* out, const std::string& payload);

// Returns false on EOF or malformed length.
bool ReadJsonFrame(FILE* in, std::string& payload_out);
