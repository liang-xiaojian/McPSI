#pragma once

// #include "gmp.h"
#include "mcpsi/utils/uint256.h"
#include "yacl/base/int128.h"

namespace mcpsi {

using uint256_t = uint256;

constexpr static uint64_t Prime64 = 2305843009213697249;  // safe prime

// prime order for secp128r2
constexpr static uint128_t Prime128 =
    yacl::MakeUint128(0x3FFFFFFF7FFFFFFF, 0xBE0024720613B5A3);  //

// constexpr static uint256_t Prime256 = uint256_t(
//     absl::MakeUint128(1152921504606846976, 0),
//     absl::MakeUint128(1503914060200516822, 6346243789798364141));  // ed25519

constexpr static uint256_t Prime256 =
    uint256_t(absl::MakeUint128(uint64_t(11764505149049458u),
                                uint64_t(17317351579400803557u)),
              absl::MakeUint128(uint64_t(16122042576031152537u),
                                uint64_t(3436901888089820391u)));  // fourQ

// static mpz_t GMP_Prime64;

// bool inline GlobalInit() {
//   mpz_init(GMP_Prime64);
//   mpz_set_ui(GMP_Prime64, Prime64);
//   return true;
// }

// const static bool init_flag = GlobalInit();

};  // namespace mcpsi
