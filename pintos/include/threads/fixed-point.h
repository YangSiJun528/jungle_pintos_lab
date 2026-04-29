#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

#include <stdint.h>

#include "../lib/stdint.h"

// 이거 인라인으로 바꾸면 좋을거 같긴 한데...
// 매크로 처럼 잘 못 처리되는거 아닐지 몰라서 일단 냅둠.
// 나중에 바꾸던가 하기

// 그리고 지금 사용에서는 상황은 값 범위가 높지 않아서 괜찮은데,
// 범용적으로 쓰려면 여러가지 고려하거나 알고 써야 할 듯?
// 범위나 unsigned 처리나 그런거?

// 일단 구분을 위해 fp가 항상 앞에 붙음.
// 변환 연산은 이름은 좀 별론거 같은데, 그나마 괜찮은거 붙인거긴 함.
// 사칙연산도 범용적인 이름이 있을거 같은데, 이건 찾아보고 고치던가 하기
// int가 들어간 사칙연산에는 _i를 붙임.

/* mlfqs 구현을 위한 고정소수점 헬퍼 함수, 17.14 고정소수점을 처리한다. */

#define FP_F (1 << 14)
typedef int64_t fp64_t; // 읽기 쉽게 변환

// Convert n to fixed point
fp64_t fp(const int64_t n) {
    return n * FP_F;
}

// Convert x to integer (rounding toward zero, 0 쪽으로 버림)
int64_t fp_int_trunc(const fp64_t x) {
    return x / FP_F;
}

// Convert x to integer (rounding to nearest, 가장 가까운 값으로 반올림)
int64_t fp_int_rnd(const fp64_t x) {
    if (x >= 0) {
        return (x + FP_F / 2) / FP_F;
    } else { // x <= 0
        return (x - FP_F / 2) / FP_F;
    }
}

// Add x and y
fp64_t fp_add(const fp64_t x, const fp64_t y) {
    return x + y;
}

// Subtract y from x
fp64_t fp_sub(const fp64_t x, const fp64_t y) {
    return x - y;
}

// Add x and n
fp64_t fp_add_i(const fp64_t x, const int64_t n) {
    return x + n * FP_F;
}

// Subtract n from x
fp64_t fp_sub_i(const fp64_t x, const int64_t n) {
    return x - n * FP_F;
}

// Multiply x by y
fp64_t fp_mul(const fp64_t x, const fp64_t y) {
    return ((int64_t) x) * y / FP_F;
}

// Multiply x by n
fp64_t fp_mul_i(const fp64_t x, const int64_t n) {
    return x * n;
}

// Divide x by y
fp64_t fp_div(const fp64_t x, const fp64_t y) {
    return ((int64_t) x) * FP_F / y;
}

// Divide x by n
fp64_t fp_div_i(const fp64_t x, const int64_t n) {
    return x / n;
}

#endif /* threads/fixed-point.h */
