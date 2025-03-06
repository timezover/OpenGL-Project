//----------------------------------------------------------------------------------------
/**
 * \file	spline.cpp
 * \author	Yahor Byaliauski
 * \date	27.04.2024
 * \brief	Implementation and initialization of animation of objects in the scene. Film-texture coordinates
 *          initialization.
*/
//----------------------------------------------------------------------------------------

#include "spline.h"

const size_t curveButterflySize = 10;

glm::vec3 curveButterflyData[] = {
    glm::vec3(0.0f, -1.09f, 0.15f),
    glm::vec3(0.41f, -0.79f, 0.05f),
    glm::vec3(0.68f, -0.38f, 0.09f),
    glm::vec3(0.72f, 0.07f, 0.03f),
    glm::vec3(0.54f, 0.57f, 0.09f),
    glm::vec3(0.14f, 0.84f, 0.12f),
    glm::vec3(-0.32f, 0.75f, 0.09f),
    glm::vec3(-0.59f, 0.34f, 0.06f),
    glm::vec3(-0.63f, -0.2f, 0.12f),
    glm::vec3(-0.45f, -0.7f, 0.15f)
};

const int particleNumVertices = 4;
const float particleVertices[] = {
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f
};

glm::mat4 allignObject(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up) {
    glm::vec3 z = -glm::normalize(front);
    if (!z.x && !z.y && !z.z) {
        z = glm::vec3(0.0, 0.0, 1.0);
    }
    glm::vec3 x = glm::normalize(glm::cross(up, z));
    if (!x.x && !x.y && !x.z) {
        x = glm::vec3(1.0, 0.0, 0.0);
    }
    glm::vec3 y = glm::cross(z, x);
    glm::mat4 matrix = glm::mat4(
        x.x, x.y, x.z, 0.0,
        y.x, y.y, y.z, 0.0,
        z.x, z.y, z.z, 0.0f,
        pos.x, pos.y, pos.z, 1.0
    );
    return matrix;
}

glm::vec3 evaluateCurveSegment(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3, const float t) {
    glm::vec3 res(0.0, 0.0, 0.0);
    float t2 = t * t;
    float t3 = t * t2;
    res = P0 * (-t3 + 2.0f * t2 - t) +
        P1 * (3.0f * t3 - 5.0f * t2 + 2.0f) +
        P2 * (-3.0f * t3 + 4.0f * t2 + t) +
        P3 * (t3 - t2);
    res *= 0.5f;
    return res;
}

glm::vec3 evaluateCurveSegment_1stDerivative(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2, const glm::vec3& P3, const float t) {
    glm::vec3 res(1.0, 0.0, 0.0);
    float t2 = t * t;
    res = P0 * (-3.0f * t2 + 4.0f * t - 1.0f) + 
          P1 * (9.0f * t2 - 10.0f * t) + 
          P2 * (-9.0f * t2 + 8.0f * t + 1.0f) + 
          P3 * (3.0f * t2 - 2.0f * t);
    res *= 0.5f;
    return res;
}

glm::vec3 evaluateClosedCurve(const glm::vec3 points[], const size_t count, const float t) {
    glm::vec3 res(0.0, 0.0, 0.0);
    float param = cyclic_clamp(t, 0.0f, float(count));
    size_t inx = size_t(param);
    res = evaluateCurveSegment(
        points[(inx - 1 + count) % count],
        points[(inx) % count],
        points[(inx + 1) % count],
        points[(inx + 2) % count],
        param - floor(param)
    );
    return res;
}

glm::vec3 evaluateClosedCurve_1stDerivative(const glm::vec3 points[], const size_t count, const float t) {
    glm::vec3 res(1.0, 0.0, 0.0);
    float param = cyclic_clamp(t, 0.0f, float(count));
    size_t inx = size_t(param);
    res = evaluateCurveSegment_1stDerivative(
        points[(inx - 1 + count) % count],
        points[(inx) % count],
        points[(inx + 1) % count],
        points[(inx + 2) % count],
        param - floor(param)
    );
    return res;
}

