/* Copyright (c) 2025, IP-Solutions AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#pragma once

#include <float.h>
#include <limits.h>
#include <stdint.h>
#ifdef __AVX__
#define USE_SIMD __AVX__
#include <immintrin.h>
#endif

#define MAX_ALG_NAME_LEN 16

enum distance_algorithm_type {
  DA_COSINE = 0,
  DA_DOT = 1,
  DA_EUCLIDEAN = 2
};
static const char *distance_algorithm_names[] = {"COSINE", "DOT", "EUCLIDEAN", NullS};
static const enum distance_algorithm_type default_distance_algorithm = DA_DOT;
static const char *default_distance_algorithm_name = distance_algorithm_names[default_distance_algorithm];

inline bool check_vector_distance_algorithm(const char* algorithm, size_t length)
{
  char alg_name[MAX_ALG_NAME_LEN];
  strncpy(alg_name, algorithm, (length<MAX_ALG_NAME_LEN)?length:MAX_ALG_NAME_LEN);
  char *c = alg_name;
  for (uint i = 0; i++ < length; c++) {
    *c = my_toupper(system_charset_info, *c);
  }
  if (strcmp(algorithm, "COSINE") == 0)
    return true;
  else if (strcmp(algorithm, "DOT") == 0)
    return true;
  else if (strcmp(algorithm, "EUCLIDEAN") == 0)
    return true;
  else
    return false;
}

inline enum distance_algorithm_type get_vector_distance_algorithm(const char* algorithm, size_t length)
{
  char alg_name[MAX_ALG_NAME_LEN];
  strncpy(alg_name, algorithm, (length<MAX_ALG_NAME_LEN)?length:MAX_ALG_NAME_LEN);
  char *c = alg_name;
  for (uint i  = 0; i++ < length; c++) {
    *c = my_toupper(system_charset_info, *c);
  }
  if (strcmp(algorithm, "COSINE") == 0)
    return DA_COSINE;
  else if (strcmp(algorithm, "DOT") == 0)
    return DA_DOT;
  else if (strcmp(algorithm, "EUCLIDEAN") == 0)
    return DA_EUCLIDEAN;
  else
    return default_distance_algorithm;
}

inline const char *get_distance_algorithm_name(const enum distance_algorithm_type alg_type)
{
  return distance_algorithm_names[alg_type];
}

/**
* Cosine distance
*/
#ifndef USE_SIMD
inline double vector_distance_cosine_float(const float* A, const float* B, uint32_t dims)
{
  double dot = 0.0, denom_a = 0.0, denom_b = 0.0;
  double a, b;
  for(uint32_t i = 0; i < dims; ++i) {
    a = static_cast<double>(A[i]);
    b = static_cast<double>(B[i]);
    dot += a * b;
    denom_a += a * a;
    denom_b += b * b;
  }
  return 1 - dot / (sqrt(denom_a) * sqrt(denom_b));
}
#else
// SIMD-accelerated Cosine distance calculation, assuming a float vector
// and assuming the vector is normalized to 16-bit values
inline double vector_distance_cosine_float(const float* A, const float* B, uint32_t dims)
{
  float dot = 0.0, denom_a = 0.0, denom_b = 0.0;
  __m256 _mm256_dot = _mm256_setzero_ps(); // Initialize dotsum to zero
  __m256 _mm256_denom_a = _mm256_setzero_ps(); // Initialize first denominator to zero
  __m256 _mm256_denom_b = _mm256_setzero_ps(); // Initialize second denominator to zero
  __m256 v1, v2, mu, sq_a, sq_b;
  uint32_t i = 0;
  for (; i + 8 <= dims; i += 8) {
    v1 = _mm256_loadu_ps(A + i);                          // Load 8 floats from A
    v2 = _mm256_loadu_ps(B + i);                          // Load 8 floats from B
    mu = _mm256_mul_ps(v1, v2);                           // Multiply the floats from each vector
    _mm256_dot = _mm256_add_ps(_mm256_dot, mu);           // Sum the dot product
    sq_a = _mm256_mul_ps(v1, v1);                         // Square floats from first vector
    _mm256_denom_a = _mm256_add_ps(_mm256_denom_a, sq_a); // Sum the first denominator
    sq_b = _mm256_mul_ps(v2, v2);                         // Square floats from second vector
    _mm256_denom_b = _mm256_add_ps(_mm256_denom_b, sq_b); // Sum the second denominator
  }

  if (i > 0) {
    // Horizontal sum of the 8 float in the SIMD register
    float dot_result[8];
    float denom_a_result[8];
    float denom_b_result[8];
    _mm256_storeu_ps(dot_result, _mm256_dot);
    _mm256_storeu_ps(denom_a_result, _mm256_denom_a);
    _mm256_storeu_ps(denom_b_result, _mm256_denom_b);
    for (int j = 0; j < 8; ++j) {
      dot += dot_result[j];
      denom_a += denom_a_result[j];
      denom_b += denom_b_result[j];
    }
  }

  // Handle the remaining elements
  float a, b;
  for(; i < dims; ++i) {
    a = A[i];
    b = B[i];
    dot += a * b;
    denom_a += a * a;
    denom_b += b * b;
  }

  return static_cast<double>(1 - dot / (sqrt(denom_a) * sqrt(denom_b)));
}
#endif

/**
   vector_distancence_cosine calculates the cosine distance as a measure of simularity.

   Cosine similarity between two vectors corresponds one minus their dot product
   divided by the product of their magnitudes.
   Smaller values indictate more simularity.

   @param vector1
   @param vector2
   @param dims    The dimension of both vectors(checked by caller to be the same)

   @returns cosine distance
 */
inline double vector_distance_cosine(const char *vector1, const char *vector2, uint32_t dims) {
  const float *A = (const float *) vector1, *B = (const float *) vector2;

  return vector_distance_cosine_float(A, B, dims);
}

/**
* Dot distance
*/
#ifndef USE_SIMD
inline double vector_distance_dot_float(const float* A, const float* B, uint32_t dims)
{
  double distance = 0.0;
  double a, b;
  for (uint32_t i = 0; i < dims; ++i) {
    a = static_cast<double>(A[i]);
    b = static_cast<double>(B[i]);
    distance += a * b;
  }

  return distance;
}
#else
// SIMD-accelerated Dot distance calculation, assuming a float vector
// and assuming the vector is normalized to 16-bit values
inline double vector_distance_dot_float(const float* A, const float* B, uint32_t dims)
{
  float distance = 0.0;
  __m256 sum = _mm256_setzero_ps(); // Initialize sum to zero
  __m256 v1, v2, mu;
  uint32_t i = 0;
  for (; i + 8 <= dims; i += 8) {
    v1 = _mm256_loadu_ps(A + i);  // Load 8 floats from A
    v2 = _mm256_loadu_ps(B + i);  // Load 8 floats from B
    mu = _mm256_mul_ps(v1, v2);   // Multiply the floats from each vector
    sum = _mm256_add_ps(sum, mu); // Accumulate the sum
  }

  if (i > 0) {
    // Horizontal sum of the 8 float in the SIMD register
    float result[8];
    _mm256_storeu_ps(result, sum);
    for (int j = 0; j < 8; ++j) {
      distance += result[j];
    }
  }

  // Handle the remaining elements
  float a, b;
  for (; i < dims; ++i) {
    a = A[i];
    b = B[i];
    distance += a * b;
  }

  return static_cast<double>(distance);
}
#endif

/**
   vector_distance_dot uses the dot product as a measure of simularity.

   The dot product of two vectors is the sum of the products of their corresponding elements.
   The dot product can range from -infinity (-DOUBLE_MAX) to +infinity (DOUBLE_MAX)
   Where DOUBLE_MAX is std::numeric_limits<double>::max()
   Larger values indictate more simularity.

   @param vector1
   @param vector2
   @param dims    The dimension of both vectors(checked by caller to be the same)

   @returns dot product
 */
inline double vector_distance_dot(const char *vector1, const char *vector2, uint32_t dims) {
  const float *A = (const float *) vector1, *B = (const float *) vector2;

  return vector_distance_dot_float(A, B, dims);
}

/**
* Euclidean distance
*/
#ifndef USE_SIMD
inline double vector_distance_euclidean_float(const float* A, const float* B, uint32_t dims)
{
  double distance = 0.0;
  double diff;
  for (uint32_t i = 0; i < dims; ++i) {
    diff = static_cast<double>(A[i] - B[i]);
    distance += diff * diff;
  }

  return distance >= 0.0 ? sqrt(distance) : std::numeric_limits<double>::max();
}
#else
// SIMD-accelerated Euclidean distance calculation, assuming a float vector
// and assuming the vector is normalized to 16-bit values
inline double vector_distance_euclidean_float(const float* A, const float* B, uint32_t dims) {
  float distance = 0.0;
  __m256 sum = _mm256_setzero_ps(); // Initialize sum to zero
  __m256 v1, v2, _mm256_diff, sq;
  uint32_t i = 0;
  for (; i + 8 <= dims; i += 8) {
    v1 = _mm256_loadu_ps(A + i);                   // Load 8 floats from A
    v2 = _mm256_loadu_ps(B + i);                   // Load 8 floats from B
    _mm256_diff = _mm256_sub_ps(v1, v2);           // Compute differences
    sq = _mm256_mul_ps(_mm256_diff, _mm256_diff);  // Square the differences
    sum = _mm256_add_ps(sum, sq);                  // Accumulate the sum
  }

  if (i > 0) {
    // Horizontal sum of the 8 float in the SIMD register
    float result[8];
    _mm256_storeu_ps(result, sum);
    for (int j = 0; j < 8; ++j) {
      distance += result[j];
    }
  }

  // Handle the remaining elements
  float diff;
  for (; i < dims; ++i) {
    diff = A[i] - B[i];
    distance += diff * diff;
  }

  return distance >= 0.0 ? static_cast<double>(sqrt(distance)) : std::numeric_limits<double>::max();
}
#endif

/**
   vector_distance_euclidean calculates eucledian distance as a measure of simularity.

   The euclidean distance can be calculated as the square root normalization
   (and checking for numeric overflow) of the difference between each points
   in the two vectors.
   Smaller values indictate more simularity.

   @param vector1
   @param vector2
   @param dims    The dimension of both vectors(checked by caller to be the same)

   @returns euclidean distance
 */
inline double vector_distance_euclidean(const char *vector1, const char *vector2, uint32_t dims) {
  const float *A = (const float *) vector1, *B = (const float *) vector2;

  return vector_distance_euclidean_float(A, B, dims);
}
