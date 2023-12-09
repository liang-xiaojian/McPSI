#pragma once

#include "field.h"
// #include "gmp.h"
#include "mcpsi/utils/field.h"
#include "yacl/base/int128.h"

namespace mcpsi {

constexpr static uint64_t Prime64 = 2305843009213697249;  // safe prime
// TODO:
constexpr static uint128_t Prime128 = 4611686018427394499;  // Prime64 * 2 + 1

// static mpz_t GMP_Prime64;

// bool inline GlobalInit() {
//   mpz_init(GMP_Prime64);
//   mpz_set_ui(GMP_Prime64, Prime64);
//   return true;
// }

// const static bool init_flag = GlobalInit();

};  // namespace mcpsi
