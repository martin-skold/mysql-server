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
  double dot = 0.0, denom_a = 0.0, denom_b = 0.0;
  const float *A = (const float *) vector1, *B = (const float *) vector2;
  float a, b;
  for(uint32_t i = 0; i < dims; ++i) {
    a = A[i];
    b = B[i];
    dot += a * b;
    denom_a += a * a;
    denom_b += b * b;
  }
  return 1 - dot / (sqrt(denom_a) * sqrt(denom_b));
}

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
  double result = 0.0;
  const float *A = (const float *) vector1, *B = (const float *) vector2;
  float a, b;
  for (uint32_t i = 0; i < dims; ++i) {
    a = A[i];
    b = B[i];
    result += a * b;
  }
  return result;
}

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
  double result = 0.0;
  const float *A = (const float *) vector1, *B = (const float *) vector2;
  float a, b, dist;
  for (uint32_t i = 0; i < dims; ++i) {
    a = A[i];
    b = B[i];
    dist = a - b;
    result += dist * dist;
  }
  return result >= 0.0 ? sqrt(result) : std::numeric_limits<double>::max();
}
