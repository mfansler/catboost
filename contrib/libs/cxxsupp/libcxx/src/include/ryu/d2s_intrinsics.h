//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Copyright 2018 Ulf Adams
// Copyright (c) Microsoft Corporation. All rights reserved.

// Boost Software License - Version 1.0 - August 17th, 2003

// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:

// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef _LIBCPP_SRC_INCLUDE_RYU_DS2_INTRINSICS_H
#define _LIBCPP_SRC_INCLUDE_RYU_DS2_INTRINSICS_H

// Avoid formatting to keep the changes with the original code minimal.
// clang-format off

#include "__config"

#include "include/ryu/ryu.h"

_LIBCPP_BEGIN_NAMESPACE_STD

#if defined(_M_X64) && defined(_MSC_VER)
#define _LIBCPP_INTRINSIC128 1
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __ryu_umul128(const uint64_t __a, const uint64_t __b, uint64_t* const __productHi) {
  return _umul128(__a, __b, __productHi);
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __ryu_shiftright128(const uint64_t __lo, const uint64_t __hi, const uint32_t __dist) {
  // For the __shiftright128 intrinsic, the shift value is always
  // modulo 64.
  // In the current implementation of the double-precision version
  // of Ryu, the shift value is always < 64.
  // (The shift value is in the range [49, 58].)
  // Check this here in case a future change requires larger shift
  // values. In this case this function needs to be adjusted.
  _LIBCPP_ASSERT(__dist < 64, "");
  return __shiftright128(__lo, __hi, static_cast<unsigned char>(__dist));
}

// ^^^ intrinsics available ^^^ / vvv __int128 available vvv
#elif defined(__SIZEOF_INT128__) && ( \
    (defined(__clang__) && !defined(_MSC_VER)) || \
    (defined(__GNUC__) && !defined(__clang__) && !defined(__CUDACC__)))
#define _LIBCPP_INTRINSIC128 1
  // We have __uint128 support in clang or gcc
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __ryu_umul128(const uint64_t __a, const uint64_t __b, uint64_t* const __productHi) {
  auto __temp = __a * (unsigned __int128)__b;
  *__productHi = __temp >> 64;
  return static_cast<uint64_t>(__temp);
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __ryu_shiftright128(const uint64_t __lo, const uint64_t __hi, const uint32_t __dist) {
  // In the current implementation of the double-precision version
  // of Ryu, the shift value is always < 64.
  // (The shift value is in the range [49, 58].)
  // Check this here in case a future change requires larger shift
  // values. In this case this function needs to be adjusted.
  _LIBCPP_ASSERT(__dist < 64, "");
  auto __temp = __lo | ((unsigned __int128)__hi << 64);
  // For x64 128-bit shfits using the `shrd` instruction and two 64-bit
  // registers, the shift value is modulo 64.  Thus the `& 63` is free.
  return static_cast<uint64_t>(__temp >> (__dist & 63));
}
#else // ^^^ __int128 available ^^^ / vvv intrinsics unavailable vvv

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline _LIBCPP_ALWAYS_INLINE uint64_t __ryu_umul128(const uint64_t __a, const uint64_t __b, uint64_t* const __productHi) {
  // TRANSITION, VSO-634761
  // The casts here help MSVC to avoid calls to the __allmul library function.
  const uint32_t __aLo = static_cast<uint32_t>(__a);
  const uint32_t __aHi = static_cast<uint32_t>(__a >> 32);
  const uint32_t __bLo = static_cast<uint32_t>(__b);
  const uint32_t __bHi = static_cast<uint32_t>(__b >> 32);

  const uint64_t __b00 = static_cast<uint64_t>(__aLo) * __bLo;
  const uint64_t __b01 = static_cast<uint64_t>(__aLo) * __bHi;
  const uint64_t __b10 = static_cast<uint64_t>(__aHi) * __bLo;
  const uint64_t __b11 = static_cast<uint64_t>(__aHi) * __bHi;

  const uint32_t __b00Lo = static_cast<uint32_t>(__b00);
  const uint32_t __b00Hi = static_cast<uint32_t>(__b00 >> 32);

  const uint64_t __mid1 = __b10 + __b00Hi;
  const uint32_t __mid1Lo = static_cast<uint32_t>(__mid1);
  const uint32_t __mid1Hi = static_cast<uint32_t>(__mid1 >> 32);

  const uint64_t __mid2 = __b01 + __mid1Lo;
  const uint32_t __mid2Lo = static_cast<uint32_t>(__mid2);
  const uint32_t __mid2Hi = static_cast<uint32_t>(__mid2 >> 32);

  const uint64_t __pHi = __b11 + __mid1Hi + __mid2Hi;
  const uint64_t __pLo = (static_cast<uint64_t>(__mid2Lo) << 32) | __b00Lo;

  *__productHi = __pHi;
  return __pLo;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __ryu_shiftright128(const uint64_t __lo, const uint64_t __hi, const uint32_t __dist) {
  // We don't need to handle the case __dist >= 64 here (see above).
  _LIBCPP_ASSERT(__dist < 64, "");
#ifdef _LIBCPP_64_BIT
  _LIBCPP_ASSERT(__dist > 0, "");
  return (__hi << (64 - __dist)) | (__lo >> __dist);
#else // ^^^ 64-bit ^^^ / vvv 32-bit vvv
  // Avoid a 64-bit shift by taking advantage of the range of shift values.
  _LIBCPP_ASSERT(__dist >= 32, "");
  return (__hi << (64 - __dist)) | (static_cast<uint32_t>(__lo >> 32) >> (__dist - 32));
#endif // ^^^ 32-bit ^^^
}

#endif // ^^^ intrinsics unavailable ^^^

#ifndef _LIBCPP_64_BIT

// Returns the high 64 bits of the 128-bit product of __a and __b.
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __umulh(const uint64_t __a, const uint64_t __b) {
  // Reuse the __ryu_umul128 implementation.
  // Optimizers will likely eliminate the instructions used to compute the
  // low part of the product.
  uint64_t __hi;
  (void) __ryu_umul128(__a, __b, &__hi);
  return __hi;
}

// On 32-bit platforms, compilers typically generate calls to library
// functions for 64-bit divisions, even if the divisor is a constant.
//
// TRANSITION, LLVM-37932
//
// The functions here perform division-by-constant using multiplications
// in the same way as 64-bit compilers would do.
//
// NB:
// The multipliers and shift values are the ones generated by clang x64
// for expressions like x/5, x/10, etc.

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __div5(const uint64_t __x) {
  return __umulh(__x, 0xCCCCCCCCCCCCCCCDu) >> 2;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __div10(const uint64_t __x) {
  return __umulh(__x, 0xCCCCCCCCCCCCCCCDu) >> 3;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __div100(const uint64_t __x) {
  return __umulh(__x >> 2, 0x28F5C28F5C28F5C3u) >> 2;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __div1e8(const uint64_t __x) {
  return __umulh(__x, 0xABCC77118461CEFDu) >> 26;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __div1e9(const uint64_t __x) {
  return __umulh(__x >> 9, 0x44B82FA09B5A53u) >> 11;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint32_t __mod1e9(const uint64_t __x) {
  // Avoid 64-bit math as much as possible.
  // Returning static_cast<uint32_t>(__x - 1000000000 * __div1e9(__x)) would
  // perform 32x64-bit multiplication and 64-bit subtraction.
  // __x and 1000000000 * __div1e9(__x) are guaranteed to differ by
  // less than 10^9, so their highest 32 bits must be identical,
  // so we can truncate both sides to uint32_t before subtracting.
  // We can also simplify static_cast<uint32_t>(1000000000 * __div1e9(__x)).
  // We can truncate before multiplying instead of after, as multiplying
  // the highest 32 bits of __div1e9(__x) can't affect the lowest 32 bits.
  return static_cast<uint32_t>(__x) - 1000000000 * static_cast<uint32_t>(__div1e9(__x));
}

#else // ^^^ 32-bit ^^^ / vvv 64-bit vvv

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __div5(const uint64_t __x) {
  return __x / 5;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __div10(const uint64_t __x) {
  return __x / 10;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __div100(const uint64_t __x) {
  return __x / 100;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __div1e8(const uint64_t __x) {
  return __x / 100000000;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint64_t __div1e9(const uint64_t __x) {
  return __x / 1000000000;
}

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint32_t __mod1e9(const uint64_t __x) {
  return static_cast<uint32_t>(__x - 1000000000 * __div1e9(__x));
}

#endif // ^^^ 64-bit ^^^

[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline uint32_t __pow5Factor(uint64_t __value) {
  uint32_t __count = 0;
  for (;;) {
    _LIBCPP_ASSERT(__value != 0, "");
    const uint64_t __q = __div5(__value);
    const uint32_t __r = static_cast<uint32_t>(__value) - 5 * static_cast<uint32_t>(__q);
    if (__r != 0) {
      break;
    }
    __value = __q;
    ++__count;
  }
  return __count;
}

// Returns true if __value is divisible by 5^__p.
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline bool __multipleOfPowerOf5(const uint64_t __value, const uint32_t __p) {
  // I tried a case distinction on __p, but there was no performance difference.
  return __pow5Factor(__value) >= __p;
}

// Returns true if __value is divisible by 2^__p.
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI inline bool __multipleOfPowerOf2(const uint64_t __value, const uint32_t __p) {
  _LIBCPP_ASSERT(__value != 0, "");
  _LIBCPP_ASSERT(__p < 64, "");
  // __builtin_ctzll doesn't appear to be faster here.
  return (__value & ((1ull << __p) - 1)) == 0;
}

_LIBCPP_END_NAMESPACE_STD

// clang-format on

#endif // _LIBCPP_SRC_INCLUDE_RYU_DS2_INTRINSICS_H