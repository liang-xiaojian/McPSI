#pragma once

#include "field.h"
#include "test/utils/field.h"
#include "yacl/base/int128.h"

namespace test {

constexpr static uint64_t Prime64 = 2305843009213697249;    // safe prime
constexpr static uint128_t Prime128 = 4611686018427394499;  // Prime64 * 2 + 1

};  // namespace test
