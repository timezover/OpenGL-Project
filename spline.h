//----------------------------------------------------------------------------------------
/**
 * \file	spline.h
 * \author	Yahor Byaliauski
 * \date	27.04.2024
 * \brief	Implementation and initialization of animation of objects in the scene. Film-texture coordinates
 *          initialization.
*/
//----------------------------------------------------------------------------------------

#pragma once
#ifndef __SPLINE_H
#define __SPLINE_H

#include "pgr.h" 

extern const size_t curveButterflySize; // Curve data for object animation
extern glm::vec3 curveButterflyData[];

extern const int particleNumVertices; // Interpolation for size for film and animated textures
extern const float particleVertices[];

/**
    This function works similarly to \ref gluLookAt, however, it is used for object transform
    rather than for view transform. Therefore, this function does not compute the computed matrix inverse.
    The current coordinate system is moved in such a way that the origin is moved
    to the \a position. Object's local front (-Z) direction is rotated to the \a front and
    object's local up (+Y) direction is rotated so that angle between its local up direction and
    \a up vector is minimum.

    * @param[in]  position           Position of the origin.
    * @param[in]  front              Front direction.
    * @param[in]  up                 Up vector.
 */
glm::mat4 allignObject(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up);

/**
    Makes sure that \a value is not outside the interval [\a minBound, \a maxBound].
    If \a value is outside the interval it is treated as a periodic value with the period equal to the size
    of the interval. A necessary number of periods are added/subtracted to fit the value to the interval.

    * @param[in]  value              Value to be clamped.
    * @param[in]  minBound           Minimum bound of value.
    * @param[in]  maxBound           Maximum bound of value.
    * @return                        Value within range [minBound, maxBound].
    * @pre                           \a minBound is not greater that \maxBound.
*/
template <typename T>
T cyclic_clamp(const T value, const T minBound, const T maxBound) {
    T amp = maxBound - minBound;
    T val = fmod(value - minBound, amp);
    if (val < T(0)) {
        val += amp;
    }
    return val + minBound;
}

/**
    * @brief Evaluates a position on Catmull-Rom curve segment.
    * @param[in] P0       First control point of the curve segment.
    * @param[in] P1       Second control point of the curve segment.
    * @param[in] P2       Third control point of the curve segment.
    * @param[in] P3       Fourth control point of the curve segment.
    * @param[in] t        Curve segment parameter. Must be within range [0, 1].
    * @return             Position on the curve for parameter \a t.
*/
glm::vec3 evaluateCurveSegment(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3, const float t);

/**
    * @brief  Evaluates a first derivative of Catmull-Rom curve segment.
    * @param[in] P0       First control point of the curve segment.
    * @param[in] P1       Second control point of the curve segment.
    * @param[in] P2       Third control point of the curve segment.
    * @param[in] P3       Fourth control point of the curve segment.
    * @param[in] t        Curve segment parameter. Must be within range [0, 1].
    * @return             Position on the curve for parameter \a t.
*/
glm::vec3 evaluateCurveSegment_1stDerivative(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3, const float t);

/**
    * @brief Evaluates a position on Catmull-Rom closed curve segment.
    * @param[in] P0       First control point of the curve segment.
    * @param[in] P1       Second control point of the curve segment.
    * @param[in] P2       Third control point of the curve segment.
    * @param[in] P3       Fourth control point of the curve segment.
    * @param[in] t        Curve segment parameter. Must be within range [0, 1].
    * @return             Position on the curve for parameter \a t.
*/
glm::vec3 evaluateClosedCurve(const glm::vec3 points[], const size_t count, const float t);

/**
    * @brief  Evaluates a first derivative of Catmull-Rom closed curve segment.
    * @param[in] P0       First control point of the curve segment.
    * @param[in] P1       Second control point of the curve segment.
    * @param[in] P2       Third control point of the curve segment.
    * @param[in] P3       Fourth control point of the curve segment.
    * @param[in] t        Curve segment parameter. Must be within range [0, 1].
    * @return             Position on the curve for parameter \a t.
*/
glm::vec3 evaluateClosedCurve_1stDerivative(const glm::vec3 points[], const size_t count, const float t);

#endif