#ifndef __STENCILS_STENCIL_FUNCTIONS_HPP__
#define __STENCILS_STENCIL_FUNCTIONS_HPP__

#include "Parameters.hpp"
#include "Definitions.hpp"

#include <math.h>

namespace NSEOF::Stencils {

// Load the local velocity cube with relevant velocities of the 2D plane
inline void loadLocalVelocity2D(FlowField& flowField, FLOAT* const localVelocity, int i, int j) {
    for (int row = -1; row <= 1; row++) {
        for (int column = -1; column <= 1; column ++) {
            const FLOAT* const point = flowField.getVelocity().getVector(i + column, j + row);

            localVelocity[39 + 9 * row + 3 * column]     = point[0]; // x-component
            localVelocity[39 + 9 * row + 3 * column + 1] = point[1]; // y-component
        }
    }
}

// Load the local velocity cube with surrounding velocities
inline void loadLocalVelocity3D(FlowField& flowField, FLOAT* const localVelocity, int i, int j, int k) {
    for (int layer = -1; layer <= 1; layer ++) {
        for (int row = -1; row <= 1; row++) {
            for (int column = -1; column <= 1; column++) {
                const FLOAT* const point = flowField.getVelocity().getVector(i + column, j + row, k + layer);

                localVelocity[39 + 27 * layer + 9 * row + 3 * column    ] = point[0]; // x-component
                localVelocity[39 + 27 * layer + 9 * row + 3 * column + 1] = point[1]; // y-component
                localVelocity[39 + 27 * layer + 9 * row + 3 * column + 2] = point[2]; // z-component
            }
        }
    }
}

// Load local meshsize for 2D -> same as loadLocalVelocity2D, but invoking call to meshsize-ptr
inline void loadLocalMeshsize2D(const Parameters& parameters, FLOAT* const localMeshsize, int i, int j) {
    for (int row = -1; row <= 1; row++) {
        for (int column = -1; column <= 1; column++) {
            localMeshsize[39 + 9 * row + 3 * column]     = parameters.meshsize->getDx(i + column, j + row);
            localMeshsize[39 + 9 * row + 3 * column + 1] = parameters.meshsize->getDy(i + column, j + row);
        }
    }
}

// Load local meshsize for 3D
inline void loadLocalMeshsize3D(const Parameters& parameters, FLOAT* const localMeshsize, int i, int j, int k) {
    for (int layer = -1; layer <= 1; layer++) {
        for (int row = -1; row <= 1; row++) {
            for (int column = -1; column <= 1; column++) {
                localMeshsize[39 + 27 * layer + 9 * row + 3 * column    ] = parameters.meshsize->getDx(i + column, j + row, k + layer);
                localMeshsize[39 + 27 * layer + 9 * row + 3 * column + 1] = parameters.meshsize->getDy(i + column, j + row, k + layer);
                localMeshsize[39 + 27 * layer + 9 * row + 3 * column + 2] = parameters.meshsize->getDz(i + column, j + row, k + layer);
            }
        }
    }
}

// Load the local viscosity (normal + eddy) cube with relevant viscosities of the 2D plane
// in all three dimension the same value
inline void loadLocalViscosity2D(const Parameters& parameters, FlowField& flowField, FLOAT* const localViscosity, int i, int j) {
    for (int row = -1; row <= 1; row++) {
        for (int column = -1; column <= 1; column ++) {
        	// changed from: const FLOAT* const point
            const FLOAT point = flowField.getEddyViscosity().getScalar(i + column, j + row);

            // changed from: point[0] to point
            localViscosity[39 + 9 * row + 3 * column]     = point + 1 / parameters.flow.Re; // x-component
            localViscosity[39 + 9 * row + 3 * column + 1] = point + 1 / parameters.flow.Re; // y-component
        }
    }
}

// Load the local viscosity (normal + eddy) cube with surrounding viscosities
// in all three dimension the same value
inline void loadLocalViscosity3D(const Parameters& parameters, FlowField& flowField, FLOAT* const localViscosity, int i, int j, int k) {
    for (int layer = -1; layer <= 1; layer ++) {
        for (int row = -1; row <= 1; row++) {
            for (int column = -1; column <= 1; column++) {
                //changed from: const FLOAT* const point
                const FLOAT point = flowField.getEddyViscosity().getScalar(i + column, j + row, k + layer);

                localViscosity[39 + 27 * layer + 9 * row + 3 * column    ] = point + 1 / parameters.flow.Re; // x-component
                localViscosity[39 + 27 * layer + 9 * row + 3 * column + 1] = point + 1 / parameters.flow.Re; // y-component
                localViscosity[39 + 27 * layer + 9 * row + 3 * column + 2] = point + 1 / parameters.flow.Re; // z-component
            }
        }
    }
}
// Maps an index and a component to the corresponding value in the cube.
inline int mapd(int i, int j, int k, int component) {
    return 39 + 27 * k + 9 * j + 3 * i + component;
}

// Derivative functions. They are applied to a cube of 3x3x3 cells. lv stands for the local velocity, lm represents the local mesh sizes
// dudx <-> first derivative of u-component of velocity field w.r.t. x-direction.
inline FLOAT dudx(const FLOAT* const lv, const FLOAT* const lm) {
    // Evaluate dudx in the cell center by a central difference
    const int index0 = mapd(0, 0, 0, 0);
    const int index1 = mapd(-1, 0, 0, 0);

    return (lv[index0] - lv[index1]) / lm[index0];
}

inline FLOAT dvdy(const FLOAT* const lv, const FLOAT* const lm) {
    const int index0 = mapd(0, 0, 0, 1);
    const int index1 = mapd(0, -1, 0, 1);

    return (lv[index0] - lv[index1]) / lm[index0];
}

inline FLOAT dwdz(const FLOAT* const lv, const FLOAT* const lm) {
    const int index0 = mapd(0, 0, 0, 2);
    const int index1 = mapd(0, 0, -1, 2);

    return (lv[index0] - lv[index1]) / lm[index0];
}

// Second derivative of u-component w.r.t. x-direction, evaluated at the location of the u-component.
inline FLOAT d2udx2(const FLOAT* const lv, const FLOAT* const lm) {
    // Evaluate the second derivative at the location of the u-component of the velocity field;
    // we therefore use the two neighbouring u-components and assume arbitrary mesh sizes in both
    // directions -> the formula arises from a straight-forward taylor expansion
    // -> for equal meshsizes, we obtain the usual [1 -2 1]-like stencil.
    const int indexM1 = mapd(-1, 0, 0, 0);
    const int index0 = mapd(0, 0, 0, 0);
    const int indexP1 = mapd(1, 0, 0, 0);

    const FLOAT dx0 = lm[index0];
    const FLOAT dx1 = lm[indexP1];
    const FLOAT dxSum = dx0 + dx1;

    return 2.0 * (lv[indexP1] / (dx1 * dxSum) - lv[index0] / (dx1 * dx0) + lv[indexM1] / (dx0 * dxSum));
}

inline FLOAT d2udy2(const FLOAT* const lv, const FLOAT* const lm) {
    // Average mesh sizes, since the component u is located in the middle of the cell's face.
    const FLOAT dy_M1 = lm[mapd(0, -1, 0, 1)];
    const FLOAT dy_0  = lm[mapd(0, 0, 0, 1)];
    const FLOAT dy_P1 = lm[mapd(0, 1, 0, 1)];
    const FLOAT dy0 = 0.5 * (dy_0 + dy_M1);
    const FLOAT dy1 = 0.5 * (dy_0 + dy_P1);
    const FLOAT dySum = dy0 + dy1;

    return 2.0 * (lv[mapd(0, 1, 0, 0)] / (dy1 * dySum) - lv[mapd(0, 0, 0, 0)] / (dy1 * dy0) + lv[mapd(0, -1, 0, 0)] / (dy0 * dySum));
}

inline FLOAT d2udz2(const FLOAT* const lv, const FLOAT* const lm) {
    const FLOAT dz_M1 = lm[mapd(0, 0, -1, 2)];
    const FLOAT dz_0  = lm[mapd(0, 0, 0, 2)];
    const FLOAT dz_P1 = lm[mapd(0, 0, 1, 2)];
    const FLOAT dz0 = 0.5 * (dz_0 + dz_M1);
    const FLOAT dz1 = 0.5 * (dz_0 + dz_P1);
    const FLOAT dzSum = dz0 + dz1;

    return 2.0 * (lv[mapd(0, 0, 1, 0)] / (dz1 * dzSum) - lv[mapd(0,0,0,0)] / (dz1 * dz0) + lv[mapd(0, 0, -1, 0)] / (dz0 * dzSum));
}

// Second derivative of the v-component, evaluated at the location of the v-component.
inline FLOAT d2vdx2(const FLOAT* const lv, const FLOAT* const lm) {
    const FLOAT dx_M1 = lm[mapd(-1, 0, 0, 0)];
    const FLOAT dx_0  = lm[mapd(0, 0, 0, 0)];
    const FLOAT dx_P1 = lm[mapd(1, 0, 0, 0)];
    const FLOAT dx0 = 0.5 * (dx_0 + dx_M1);
    const FLOAT dx1 = 0.5 * (dx_0 + dx_P1);
    const FLOAT dxSum = dx0 + dx1;

    return 2.0 * (lv[mapd(1, 0, 0, 1)] / (dx1 * dxSum) - lv[mapd(0, 0, 0, 1)] / (dx1 * dx0) + lv[mapd(-1, 0, 0, 1)] / (dx0 * dxSum));
}

inline FLOAT d2vdy2 (const FLOAT* const lv, const FLOAT* const lm) {
    const int indexM1 = mapd(0, -1, 0, 1);
    const int index0 = mapd(0, 0, 0, 1);
    const int indexP1 = mapd(0, 1, 0, 1);

    const FLOAT dy0 = lm[index0];
    const FLOAT dy1 = lm[indexP1];
    const FLOAT dySum = dy0 + dy1;

    return 2.0 * (lv[indexP1] / (dy1 * dySum) - lv[index0] / (dy1 * dy0) + lv[indexM1] / (dy0 * dySum));
}

inline FLOAT d2vdz2(const FLOAT* const lv, const FLOAT* const lm) {
    const FLOAT dz_M1 = lm[mapd(0, 0, -1, 2)];
    const FLOAT dz_0  = lm[mapd(0, 0, 0, 2)];
    const FLOAT dz_P1 = lm[mapd(0, 0, 1, 2)];
    const FLOAT dz0 = 0.5 * (dz_0 + dz_M1);
    const FLOAT dz1 = 0.5 * (dz_0 + dz_P1);
    const FLOAT dzSum = dz0 + dz1;

    return 2.0 * (lv[mapd(0, 0, 1, 1)] / (dz1 * dzSum) - lv[mapd(0, 0, 0, 1)] / (dz1 * dz0) + lv[mapd(0, 0, -1, 1)] / (dz0 * dzSum));
}

// Second derivative of the w-component, evaluated at the location of the w-component.
inline FLOAT d2wdx2(const FLOAT* const lv, const FLOAT* const lm) {
    const FLOAT dx_M1 = lm[mapd(-1, 0, 0, 0)];
    const FLOAT dx_0 = lm[mapd(0, 0, 0, 0)];
    const FLOAT dx_P1 = lm[mapd(1, 0, 0, 0)];
    const FLOAT dx0 = 0.5 * (dx_0 + dx_M1);
    const FLOAT dx1 = 0.5 * (dx_0 + dx_P1);
    const FLOAT dxSum = dx0 + dx1;

    return 2.0 * (lv[mapd(1, 0, 0, 2)] / (dx1 * dxSum) - lv[mapd(0, 0, 0, 2)] / (dx1 * dx0) + lv[mapd(-1, 0, 0, 2)] / (dx0 * dxSum));
}

inline FLOAT d2wdy2(const FLOAT* const lv, const FLOAT* const lm) {
    const FLOAT dy_M1 = lm[mapd(0, -1, 0, 1)];
    const FLOAT dy_0 = lm[mapd(0, 0, 0, 1)];
    const FLOAT dy_P1 = lm[mapd(0, 1, 0, 1)];
    const FLOAT dy0 = 0.5 * (dy_0 + dy_M1);
    const FLOAT dy1 = 0.5 * (dy_0 + dy_P1);
    const FLOAT dySum = dy0 + dy1;

    return 2.0 * (lv[mapd(0, 1, 0, 2)] / (dy1 * dySum) - lv[mapd(0, 0, 0, 2)] / (dy1 * dy0) + lv[mapd(0, -1, 0, 2)] / (dy0 * dySum));
}

inline FLOAT d2wdz2(const FLOAT* const lv, const FLOAT* const lm) {
    const int index_M1 = mapd(0, 0, -1, 2);
    const int index_0 = mapd(0, 0, 0, 2);
    const int index_P1 = mapd(0, 0, 1, 2);

    const FLOAT dz0 = lm[index_0];
    const FLOAT dz1 = lm[index_P1];
    const FLOAT dzSum = dz0 + dz1;

    return 2.0 * (lv[index_P1] / (dz1 * dzSum) - lv[index_0] / (dz1 * dz0) + lv[index_M1] / (dz0 * dzSum));
}

// First derivative of product (u*v), evaluated at the location of the v-component.
inline FLOAT duvdx(const FLOAT* const lv, const Parameters& parameters, const FLOAT* const lm) {
#ifndef NDEBUG
    const FLOAT tmp1 = 1.0 / 4.0 * ((((lv[mapd(0, 0, 0, 0)] + lv[mapd(0, 1, 0, 0)]) *
        (lv[mapd(0, 0, 0, 1)] + lv[mapd(1, 0, 0, 1)])) -
        ((lv[mapd(-1, 0, 0, 0)] + lv[mapd(-1, 1, 0, 0)]) *
            (lv[mapd(-1, 0, 0, 1)] + lv[mapd(0, 0, 0, 1)])))
        + parameters.solver.gamma * ((fabs(lv[mapd(0, 0, 0, 0)] + lv[mapd(0, 1, 0, 0)]) *
            (lv[mapd(0, 0, 0, 1)] - lv[mapd(1, 0, 0, 1)])) -
            (fabs(lv[mapd(-1, 0, 0, 0)] + lv[mapd(-1, 1, 0, 0)]) *
                (lv[mapd(-1, 0, 0, 1)] - lv[mapd(0, 0, 0, 1)])))
        ) / lm[mapd(0, 0, 0, 0)];
#endif

    const FLOAT hxShort = 0.5 * lm[mapd(0, 0, 0, 0)];                           // Distance of corner points in x-direction from center v-value
    const FLOAT hxLong0 = 0.5 * (lm[mapd(0, 0, 0, 0)] + lm[mapd(-1, 0, 0, 0)]); // Distance between center and west v-value
    const FLOAT hxLong1 = 0.5 * (lm[mapd(0, 0, 0, 0)] + lm[mapd(1, 0, 0, 0)]);  // Distance between center and east v-value
    const FLOAT hyShort = 0.5 * lm[mapd(0, 0, 0, 1)];                           // Distance of center u-value from upper edge of cell
    const FLOAT hyLong = 0.5 * (lm[mapd(0, 0, 0, 1)] + lm[mapd(0, 1, 0, 1)]);   // Distance of north and center u-value

    const FLOAT u00 = lv[mapd(0, 0, 0, 0)];
    const FLOAT u01 = lv[mapd(0, 1, 0, 0)];
    const FLOAT v00 = lv[mapd(0, 0, 0, 1)];
    const FLOAT v10 = lv[mapd(1, 0, 0, 1)];

    const FLOAT uM10 = lv[mapd(-1, 0, 0, 0)];
    const FLOAT uM11 = lv[mapd(-1, 1, 0, 0)];
    const FLOAT vM10 = lv[mapd(-1, 0, 0, 1)];

    // This a central difference expression for the first-derivative. We therefore linearly interpolate u*v onto the surface of the
    // current cell (in 2D: upper left and upper right corner) and then take the central difference.
    const FLOAT secondOrder = (((hyLong - hyShort) / hyLong * u00 + hyShort / hyLong * u01) * ((hxLong1 - hxShort) / hxLong1 * v00 + hxShort / hxLong1 * v10)
        - ((hyLong - hyShort) / hyLong * uM10 + hyShort / hyLong * uM11) * ((hxLong0 - hxShort) / hxLong0 * v00 + hxShort / hxLong0 * vM10)) / (2.0 * hxShort);

    // This is a forward-difference in donor-cell style. We apply donor cell and again interpolate the velocity values (u-comp.)
    // onto the surface of the cell. We then apply the standard donor cell scheme. This will, however, result in non-equal
    // mesh spacing evaluations (in case of stretched meshes).
    const FLOAT kr = (hyLong - hyShort) / hyLong * u00 + hyShort / hyLong * u01;
    const FLOAT kl = (hyLong - hyShort) / hyLong * uM10 + hyShort / hyLong * uM11;

    const FLOAT firstOrder = 1.0 / (4.0 * hxShort) * (
        kr * (v00 + v10) - kl * (vM10 + v00) + fabs(kr) * (v00 - v10) - fabs(kl) * (vM10 - v00));

    // Return linear combination of central and donor-cell difference
    const FLOAT tmp2 = (1.0 - parameters.solver.gamma) * secondOrder + parameters.solver.gamma * firstOrder;

#ifndef NDEBUG
    if (fabs(tmp1 - tmp2) > 1.0e-12) {
        HANDLE_ERROR(1, "Error in duvdx");
    }
#endif

    return tmp2;
}

// Evaluates first derivative w.r.t. y for u*v at location of u-component. For details on implementation, see duvdx.
inline FLOAT duvdy(const FLOAT* const lv, const Parameters& parameters, const FLOAT* const lm) {
#ifndef NDEBUG
    const FLOAT tmp1 = 1.0 / 4.0 * ((((lv[mapd(0, 0, 0, 1)] + lv[mapd(1, 0, 0, 1)]) *
        (lv[mapd(0, 0, 0, 0)] + lv[mapd(0, 1, 0, 0)])) -
        ((lv[mapd(0, -1, 0, 1)] + lv[mapd(1, -1, 0, 1)]) *
            (lv[mapd(0, -1, 0, 0)] + lv[mapd(0, 0, 0, 0)]))) +
        parameters.solver.gamma * ((fabs(lv[mapd(0, 0, 0, 1)] + lv[mapd(1, 0, 0, 1)]) *
            (lv[mapd(0, 0, 0, 0)] - lv[mapd(0, 1, 0, 0)])) -
            (fabs(lv[mapd(0, -1, 0, 1)] + lv[mapd(1, -1, 0, 1)]) *
                (lv[mapd(0, -1, 0, 0)] - lv[mapd(0, 0, 0, 0)])))) /
        lm[mapd(0, 0, 0, 1)];
#endif

    const FLOAT hyShort = 0.5 * lm[mapd(0, 0, 0, 1)];                           // Distance of corner points in x-direction from center v-value
    const FLOAT hyLong0 = 0.5 * (lm[mapd(0, 0, 0, 1)] + lm[mapd(0, -1, 0, 1)]); // Distance between center and west v-value
    const FLOAT hyLong1 = 0.5 * (lm[mapd(0, 0, 0, 1)] + lm[mapd(0, 1, 0, 1)]);  // Distance between center and east v-value
    const FLOAT hxShort = 0.5 * lm[mapd(0, 0, 0, 0)];                           // Distance of center u-value from upper edge of cell
    const FLOAT hxLong = 0.5 * (lm[mapd(0, 0, 0, 0)] + lm[mapd(1, 0, 0, 0)]);   // Distance of north and center u-value

    const FLOAT v00 = lv[mapd(0, 0, 0, 1)];
    const FLOAT v10 = lv[mapd(1, 0, 0, 1)];
    const FLOAT u00 = lv[mapd(0, 0, 0, 0)];
    const FLOAT u01 = lv[mapd(0, 1, 0, 0)];

    const FLOAT v0M1 = lv[mapd(0, -1, 0, 1)];
    const FLOAT v1M1 = lv[mapd(1, -1, 0, 1)];
    const FLOAT u0M1 = lv[mapd(0, -1, 0, 0)];

    const FLOAT secondOrder = (((hxLong - hxShort) / hxLong * v00 + hxShort / hxLong * v10) * ((hyLong1 - hyShort) / hyLong1 * u00 + hyShort / hyLong1 * u01)
        - ((hxLong - hxShort) / hxLong * v0M1 + hxShort / hxLong * v1M1) * ((hyLong0 - hyShort) / hyLong0 * u00 + hyShort / hyLong0 * u0M1)) / (2.0 * hyShort);

    const FLOAT kr = (hxLong - hxShort) / hxLong * v00 + hxShort / hxLong * v10;
    const FLOAT kl = (hxLong - hxShort) / hxLong * v0M1 + hxShort / hxLong * v1M1;

    const FLOAT firstOrder = 1.0 / (4.0 * hyShort) * (
        kr * (u00 + u01) - kl * (u0M1 + u00) + fabs(kr) * (u00 - u01) - fabs(kl) * (u0M1 - u00));

    const FLOAT tmp2 = (1.0 - parameters.solver.gamma) * secondOrder + parameters.solver.gamma * firstOrder;

#ifndef NDEBUG
    if (fabs(tmp1 - tmp2) > 1.0e-12) {
        HANDLE_ERROR(1, "Error in duvdy");
    }
#endif

    return tmp2;
}

// Evaluates first derivative w.r.t. x for u*w at location of w-component. For details on implementation, see duvdx.
inline FLOAT duwdx(const FLOAT* const lv, const Parameters& parameters, const FLOAT* const lm) {
#ifndef NDEBUG
    const FLOAT tmp1 = 1.0 / 4.0 * ((((lv[mapd(0, 0, 0, 0)] + lv[mapd(0, 0, 1, 0)]) *
        (lv[mapd(0, 0, 0, 2)] + lv[mapd(1, 0, 0, 2)])) -
        ((lv[mapd(-1, 0, 0, 0)] + lv[mapd(-1, 0, 1, 0)]) *
            (lv[mapd(-1, 0, 0, 2)] + lv[mapd(0, 0, 0, 2)]))) +
        parameters.solver.gamma * ((fabs(lv[mapd(0, 0, 0, 0)] + lv[mapd(0, 0, 1, 0)]) *
            (lv[mapd(0, 0, 0, 2)] - lv[mapd(1, 0, 0, 2)])) -
            (fabs(lv[mapd(-1, 0, 0, 0)] + lv[mapd(-1, 0, 1, 0)]) *
                (lv[mapd(-1, 0, 0, 2)] - lv[mapd(0, 0, 0, 2)])))) /
        lm[mapd(0, 0, 0, 0)];
#endif

    const FLOAT hxShort = 0.5 * lm[mapd(0, 0, 0, 0)];                           // Distance of corner points in x-direction from center v-value
    const FLOAT hxLong0 = 0.5 * (lm[mapd(0, 0, 0, 0)] + lm[mapd(-1, 0, 0, 0)]); // Distance between center and west v-value
    const FLOAT hxLong1 = 0.5 * (lm[mapd(0, 0, 0, 0)] + lm[mapd(1, 0, 0, 0)]);  // Distance between center and east v-value
    const FLOAT hzShort = 0.5 * lm[mapd(0, 0, 0, 2)];                           // Distance of center u-value from upper edge of cell
    const FLOAT hzLong = 0.5 * (lm[mapd(0, 0, 0, 2)] + lm[mapd(0, 0, 1, 2)]);   // Distance of north and center u-value

    const FLOAT u00 = lv[mapd(0, 0, 0, 0)];
    const FLOAT u01 = lv[mapd(0, 0, 1, 0)];
    const FLOAT w00 = lv[mapd(0, 0, 0, 2)];
    const FLOAT w10 = lv[mapd(1, 0, 0, 2)];

    const FLOAT uM10 = lv[mapd(-1, 0, 0, 0)];
    const FLOAT uM11 = lv[mapd(-1, 0, 1, 0)];
    const FLOAT wM10 = lv[mapd(-1, 0, 0, 2)];

    const FLOAT secondOrder = (((hzLong - hzShort) / hzLong * u00 + hzShort / hzLong * u01) * ((hxLong1 - hxShort) / hxLong1 * w00 + hxShort / hxLong1 * w10)
        - ((hzLong - hzShort) / hzLong * uM10 + hzShort / hzLong * uM11) * ((hxLong0 - hxShort) / hxLong0 * w00 + hxShort / hxLong0 * wM10)) / (2.0 * hxShort);

    const FLOAT kr = (hzLong - hzShort) / hzLong * u00 + hzShort / hzLong * u01;
    const FLOAT kl = (hzLong - hzShort) / hzLong * uM10 + hzShort / hzLong * uM11;

    const FLOAT firstOrder = 1.0 / (4.0 * hxShort) * (
        kr * (w00 + w10) - kl * (wM10 + w00) + fabs(kr) * (w00 - w10) - fabs(kl) * (wM10 - w00));

    const FLOAT tmp2 = (1.0 - parameters.solver.gamma) * secondOrder + parameters.solver.gamma * firstOrder;

#ifndef NDEBUG
    if (fabs(tmp1 - tmp2) > 1.0e-12) {
        HANDLE_ERROR(1, "Error in duwdx");
    }
#endif

    return tmp2;
}

// Evaluates first derivative w.r.t. z for u*w at location of u-component. For details on implementation, see duvdx.
inline FLOAT duwdz(const FLOAT* const lv, const Parameters& parameters, const FLOAT* const lm) {
#ifndef NDEBUG
    const FLOAT tmp1 = 1.0 / 4.0 * ((((lv[mapd(0, 0, 0, 2)] + lv[mapd(1, 0, 0, 2)]) *
        (lv[mapd(0, 0, 0, 0)] + lv[mapd(0, 0, 1, 0)])) -
        ((lv[mapd(0, 0, -1, 2)] + lv[mapd(1, 0, -1, 2)]) *
            (lv[mapd(0, 0, -1, 0)] + lv[mapd(0, 0, 0, 0)]))) +
        parameters.solver.gamma * ((fabs(lv[mapd(0, 0, 0, 2)] + lv[mapd(1, 0, 0, 2)]) *
            (lv[mapd(0, 0, 0, 0)] - lv[mapd(0, 0, 1, 0)])) -
            (fabs(lv[mapd(0, 0, -1, 2)] + lv[mapd(1, 0, -1, 2)]) *
                (lv[mapd(0, 0, -1, 0)] - lv[mapd(0, 0, 0, 0)])))) /
        lm[mapd(0, 0, 0, 2)];
#endif

    const FLOAT hzShort = 0.5 * lm[mapd(0, 0, 0, 2)];                           // Distance of corner points in x-direction from center v-value
    const FLOAT hzLong0 = 0.5 * (lm[mapd(0, 0, 0, 2)] + lm[mapd(0, 0, -1, 2)]); // Distance between center and west v-value
    const FLOAT hzLong1 = 0.5 * (lm[mapd(0, 0, 0, 2)] + lm[mapd(0, 0, 1, 2)]);  // Distance between center and east v-value
    const FLOAT hxShort = 0.5 * lm[mapd(0, 0, 0, 0)];                           // Distance of center u-value from upper edge of cell
    const FLOAT hxLong = 0.5 * (lm[mapd(0, 0, 0, 0)] + lm[mapd(1, 0, 0, 0)]);   // Distance of north and center u-value

    const FLOAT w00 = lv[mapd(0, 0, 0, 2)];
    const FLOAT w10 = lv[mapd(1, 0, 0, 2)];
    const FLOAT u00 = lv[mapd(0, 0, 0, 0)];
    const FLOAT u01 = lv[mapd(0, 0, 1, 0)];

    const FLOAT w0M1 = lv[mapd(0, 0, -1, 2)];
    const FLOAT w1M1 = lv[mapd(1, 0, -1, 2)];
    const FLOAT u0M1 = lv[mapd(0, 0, -1, 0)];

    const FLOAT secondOrder = (((hxLong - hxShort) / hxLong * w00 + hxShort / hxLong * w10) * ((hzLong1 - hzShort) / hzLong1 * u00 + hzShort / hzLong1 * u01)
        - ((hxLong - hxShort) / hxLong * w0M1 + hxShort / hxLong * w1M1) * ((hzLong0 - hzShort) / hzLong0 * u00 + hzShort / hzLong0 * u0M1)) / (2.0 * hzShort);

    const FLOAT kr = (hxLong - hxShort) / hxLong * w00 + hxShort / hxLong * w10;
    const FLOAT kl = (hxLong - hxShort) / hxLong * w0M1 + hxShort / hxLong * w1M1;

    const FLOAT firstOrder = 1.0 / (4.0 * hzShort) * (
        kr * (u00 + u01) - kl * (u0M1 + u00) + fabs(kr) * (u00 - u01) - fabs(kl) * (u0M1 - u00));

    const FLOAT tmp2 = (1.0 - parameters.solver.gamma) * secondOrder + parameters.solver.gamma * firstOrder;

#ifndef NDEBUG
    if (fabs(tmp1 - tmp2) > 1.0e-12) {
        HANDLE_ERROR(1, "Error in duwdz");
    }
#endif

    return tmp2;
}

// Evaluates first derivative w.r.t. y for v*w at location of w-component. For details on implementation, see duvdx.
inline FLOAT dvwdy(const FLOAT* const lv, const Parameters& parameters, const FLOAT* const lm) {
#ifndef NDEBUG
    const FLOAT tmp1 = 1.0 / 4.0 * ((((lv[mapd(0, 0, 0, 1)] + lv[mapd(0, 0, 1, 1)]) *
        (lv[mapd(0, 0, 0, 2)] + lv[mapd(0, 1, 0, 2)])) -
        ((lv[mapd(0, -1, 0, 1)] + lv[mapd(0, -1, 1, 1)]) *
            (lv[mapd(0, -1, 0, 2)] + lv[mapd(0, 0, 0, 2)]))) +
        parameters.solver.gamma * ((fabs(lv[mapd(0, 0, 0, 1)] + lv[mapd(0, 0, 1, 1)]) *
            (lv[mapd(0, 0, 0, 2)] - lv[mapd(0, 1, 0, 2)])) -
            (fabs(lv[mapd(0, -1, 0, 1)] + lv[mapd(0, -1, 1, 1)]) *
                (lv[mapd(0, -1, 0, 2)] - lv[mapd(0, 0, 0, 2)])))) /
        lm[mapd(0, 0, 0, 1)];
#endif

    const FLOAT hyShort = 0.5 * lm[mapd(0, 0, 0, 1)];                           // Distance of corner points in x-direction from center v-value
    const FLOAT hyLong0 = 0.5 * (lm[mapd(0, 0, 0, 1)] + lm[mapd(0, -1, 0, 1)]); // Distance between center and west v-value
    const FLOAT hyLong1 = 0.5 * (lm[mapd(0, 0, 0, 1)] + lm[mapd(0, 1, 0, 1)]);  // Distance between center and east v-value
    const FLOAT hzShort = 0.5 * lm[mapd(0, 0, 0, 2)];                           // Distance of center u-value from upper edge of cell
    const FLOAT hzLong = 0.5 * (lm[mapd(0, 0, 0, 2)] + lm[mapd(0, 0, 1, 2)]);   // Distance of north and center u-value

    const FLOAT v00 = lv[mapd(0, 0, 0, 1)];
    const FLOAT v01 = lv[mapd(0, 0, 1, 1)];
    const FLOAT w00 = lv[mapd(0, 0, 0, 2)];
    const FLOAT w10 = lv[mapd(0, 1, 0, 2)];

    const FLOAT vM10 = lv[mapd(0, -1, 0, 1)];
    const FLOAT vM11 = lv[mapd(0, -1, 1, 1)];
    const FLOAT wM10 = lv[mapd(0, -1, 0, 2)];

    const FLOAT secondOrder = (((hzLong - hzShort) / hzLong * v00 + hzShort / hzLong * v01) * ((hyLong1 - hyShort) / hyLong1 * w00 + hyShort / hyLong1 * w10)
        - ((hzLong - hzShort) / hzLong * vM10 + hzShort / hzLong * vM11) * ((hyLong0 - hyShort) / hyLong0 * w00 + hyShort / hyLong0 * wM10)) / (2.0 * hyShort);

    const FLOAT kr = (hzLong - hzShort) / hzLong * v00 + hzShort / hzLong * v01;
    const FLOAT kl = (hzLong - hzShort) / hzLong * vM10 + hzShort / hzLong * vM11;

    const FLOAT firstOrder = 1.0 / (4.0 * hyShort) * (
        kr * (w00 + w10) - kl * (wM10 + w00) + fabs(kr) * (w00 - w10) - fabs(kl) * (wM10 - w00));

    const FLOAT tmp2 = (1.0 - parameters.solver.gamma) * secondOrder + parameters.solver.gamma * firstOrder;

#ifndef NDEBUG
    if (fabs(tmp1 - tmp2) > 1.0e-12) {
        HANDLE_ERROR(1, "Error in dvwdy");
    }
#endif

    return tmp2;
}

// Evaluates first derivative w.r.t. z for v*w at location of v-component. For details on implementation, see duvdx.
inline FLOAT dvwdz(const FLOAT* const lv, const Parameters& parameters, const FLOAT* const lm) {
#ifndef NDEBUG
    const FLOAT tmp1 = 1.0 / 4.0 * ((((lv[mapd(0, 0, 0, 2)] + lv[mapd(0, 1, 0, 2)]) *
        (lv[mapd(0, 0, 0, 1)] + lv[mapd(0, 0, 1, 1)])) -
        ((lv[mapd(0, 0, -1, 2)] + lv[mapd(0, 1, -1, 2)]) *
            (lv[mapd(0, 0, -1, 1)] + lv[mapd(0, 0, 0, 1)]))) +
        parameters.solver.gamma * ((fabs(lv[mapd(0, 0, 0, 2)] + lv[mapd(0, 1, 0, 2)]) *
            (lv[mapd(0, 0, 0, 1)] - lv[mapd(0, 0, 1, 1)])) -
            (fabs(lv[mapd(0, 0, -1, 2)] + lv[mapd(0, 1, -1, 2)]) *
                (lv[mapd(0, 0, -1, 1)] - lv[mapd(0, 0, 0, 1)])))) /
        lm[mapd(0, 0, 0, 2)];
#endif

    const FLOAT hzShort = 0.5 * lm[mapd(0, 0, 0, 2)];                           // Distance of corner points in x-direction from center v-value
    const FLOAT hzLong0 = 0.5 * (lm[mapd(0, 0, 0, 2)] + lm[mapd(0, 0, -1, 2)]); // Distance between center and west v-value
    const FLOAT hzLong1 = 0.5 * (lm[mapd(0, 0, 0, 2)] + lm[mapd(0, 0, 1, 2)]);  // Distance between center and east v-value
    const FLOAT hyShort = 0.5 * lm[mapd(0, 0, 0, 1)];                           // Distance of center u-value from upper edge of cell
    const FLOAT hyLong = 0.5 * (lm[mapd(0, 0, 0, 1)] + lm[mapd(0, 1, 0, 1)]);   // Distance of north and center u-value

    const FLOAT w00 = lv[mapd(0, 0, 0, 2)];
    const FLOAT w10 = lv[mapd(0, 1, 0, 2)];
    const FLOAT v00 = lv[mapd(0, 0, 0, 1)];
    const FLOAT v01 = lv[mapd(0, 0, 1, 1)];

    const FLOAT w0M1 = lv[mapd(0, 0, -1, 2)];
    const FLOAT w1M1 = lv[mapd(0, 1, -1, 2)];
    const FLOAT v0M1 = lv[mapd(0, 0, -1, 1)];

    const FLOAT secondOrder = (((hyLong - hyShort) / hyLong * w00 + hyShort / hyLong * w10) * ((hzLong1 - hzShort) / hzLong1 * v00 + hzShort / hzLong1 * v01)
        - ((hyLong - hyShort) / hyLong * w0M1 + hyShort / hyLong * w1M1) * ((hzLong0 - hzShort) / hzLong0 * v00 + hzShort / hzLong0 * v0M1)) / (2.0 * hzShort);

    const FLOAT kr = (hyLong - hyShort) / hyLong * w00 + hyShort / hyLong * w10;
    const FLOAT kl = (hyLong - hyShort) / hyLong * w0M1 + hyShort / hyLong * w1M1;

    const FLOAT firstOrder = 1.0 / (4.0 * hzShort) * (
        kr * (v00 + v01) - kl * (v0M1 + v00) + fabs(kr) * (v00 - v01) - fabs(kl) * (v0M1 - v00));

    const FLOAT tmp2 = (1.0 - parameters.solver.gamma) * secondOrder + parameters.solver.gamma * firstOrder;

#ifndef NDEBUG
    if (fabs(tmp1 - tmp2) > 1.0e-12) {
        std::cout << tmp1 << ", " << tmp2 << std::endl;
        HANDLE_ERROR(1, "Error in dvwdz");
    }
#endif

    return tmp2;
}

// First derivative of u*u w.r.t. x, evaluated at location of u-component.
inline FLOAT du2dx(const FLOAT* const lv, const Parameters& parameters, const FLOAT* const lm) {
#ifndef NDEBUG
    const FLOAT tmp1 = 1.0 / 4.0 * ((((lv[mapd(0, 0, 0, 0)] + lv[mapd(1, 0, 0, 0)]) *
        (lv[mapd(0, 0, 0, 0)] + lv[mapd(1, 0, 0, 0)])) -
        ((lv[mapd(-1, 0, 0, 0)] + lv[mapd(0, 0, 0, 0)]) *
            (lv[mapd(-1, 0, 0, 0)] + lv[mapd(0, 0, 0, 0)]))) +
        parameters.solver.gamma * ((fabs(lv[mapd(0, 0, 0, 0)] + lv[mapd(1, 0, 0, 0)]) *
            (lv[mapd(0, 0, 0, 0)] - lv[mapd(1, 0, 0, 0)])) -
            (fabs(lv[mapd(-1, 0, 0, 0)] + lv[mapd(0, 0, 0, 0)]) *
                (lv[mapd(-1, 0, 0, 0)] - lv[mapd(0, 0, 0, 0)])))) /
        lm[mapd(0, 0, 0, 0)];
#endif

    const FLOAT dxShort = 0.5 * lm[mapd(0, 0, 0, 0)];
    // const FLOAT dxLong0 = 0.5*(lm[mapd(-1,0,0,0)] + lm[mapd(0,0,0,0)]);
    const FLOAT dxLong1 = 0.5 * (lm[mapd(0, 0, 0, 0)] + lm[mapd(1, 0, 0, 0)]);

    const FLOAT u0 = lv[mapd(0, 0, 0, 0)];
    const FLOAT uM1 = lv[mapd(-1, 0, 0, 0)];
    const FLOAT u1 = lv[mapd(1, 0, 0, 0)];

    // const FLOAT kr = (dxLong1 - dxShort) / dxLong1 * u0 + dxShort / dxLong1 * u1;
    // const FLOAT kl = (dxLong0 - dxShort) / dxLong0 * u0 + dxShort / dxLong0 * uM1;
    const FLOAT kr = (u0 + u1) / 2;
    const FLOAT kl = (u0 + uM1) / 2;

    // Central difference expression which is second-order accurate for uniform meshes. We interpolate u half-way between
    // neighboured u-component values and afterwards build the central difference for u*u.

    /* const FLOAT secondOrder = (((dxLong1 - dxShort) / dxLong1 * u0 + dxShort / dxLong1 * u1) * ((dxLong1 - dxShort) / dxLong1 * u0 + dxShort / dxLong1 * u1)
        - ((dxLong0 - dxShort) / dxLong0 * u0 + dxShort / dxLong0 * uM1) * ((dxLong0 - dxShort) / dxLong0 * u0 + dxShort / dxLong0 * uM1)
        ) / (2.0 * dxShort); */

    const FLOAT secondOrder = ((u0 + u1) * (u0 + u1) - (u0 + uM1) * (u0 + uM1)) / (4 * dxLong1);

    // Donor-cell like derivative expression. We evaluate u half-way between neighboured u-components and use this as a prediction of the transport direction.
    const FLOAT firstOrder = 1.0 / (4.0 * dxShort) * (
        kr * (u0 + u1) - kl * (uM1 + u0) + fabs(kr) * (u0 - u1) - fabs(kl) * (uM1 - u0));

    // Return linear combination of central- and upwind difference
    const FLOAT tmp2 = (1.0 - parameters.solver.gamma) * secondOrder + parameters.solver.gamma * firstOrder;

#ifndef NDEBUG
    if (fabs(tmp1 - tmp2) > 1.0e-12) {
        HANDLE_ERROR(1, "Error in du2dx");
    }
#endif

    return tmp2;
}

// First derivative of v*v w.r.t. y, evaluated at location of v-component. For details, see du2dx.
inline FLOAT dv2dy(const FLOAT* const lv, const Parameters& parameters, const FLOAT* const lm) {
#ifndef NDEBUG
    const FLOAT tmp1 = 1.0 / 4.0 * ((((lv[mapd(0, 0, 0, 1)] + lv[mapd(0, 1, 0, 1)]) *
        (lv[mapd(0, 0, 0, 1)] + lv[mapd(0, 1, 0, 1)])) -
        ((lv[mapd(0, -1, 0, 1)] + lv[mapd(0, 0, 0, 1)]) *
            (lv[mapd(0, -1, 0, 1)] + lv[mapd(0, 0, 0, 1)]))) +
        parameters.solver.gamma * ((fabs(lv[mapd(0, 0, 0, 1)] + lv[mapd(0, 1, 0, 1)]) *
            (lv[mapd(0, 0, 0, 1)] - lv[mapd(0, 1, 0, 1)])) -
            (fabs(lv[mapd(0, -1, 0, 1)] + lv[mapd(0, 0, 0, 1)]) *
                (lv[mapd(0, -1, 0, 1)] - lv[mapd(0, 0, 0, 1)])))) /
        lm[mapd(0, 0, 0, 1)];
#endif

    const FLOAT dyShort = 0.5 * lm[mapd(0, 0, 0, 1)];
    // const FLOAT dyLong0 = 0.5*(lm[mapd(0,-1,0,1)] + lm[mapd(0,0,0,1)]);
    const FLOAT dyLong1 = 0.5 * (lm[mapd(0, 0, 0, 1)] + lm[mapd(0, 1, 0, 1)]);

    const FLOAT v0 = lv[mapd(0, 0, 0, 1)];
    const FLOAT vM1 = lv[mapd(0, -1, 0, 1)];
    const FLOAT v1 = lv[mapd(0, 1, 0, 1)];

    // const FLOAT kr = (dyLong1 - dyShort) / dyLong1 * v0 + dyShort / dyLong1 * v1;
    // const FLOAT kl = (dyLong0 - dyShort) / dyLong0 * v0 + dyShort / dyLong0 * vM1;
    const FLOAT kr = (v0 + v1) / 2;
    const FLOAT kl = (v0 + vM1) / 2;

    /* const FLOAT secondOrder = (((dyLong1 - dyShort) / dyLong1 * v0 + dyShort / dyLong1 * v1) * ((dyLong1 - dyShort) / dyLong1 * v0 + dyShort / dyLong1 * v1)
        - ((dyLong0 - dyShort) / dyLong0 * v0 + dyShort / dyLong0 * vM1) * ((dyLong0 - dyShort) / dyLong0 * v0 + dyShort / dyLong0 * vM1)
        ) / (2.0 * dyShort); */

    const FLOAT secondOrder = ((v0 + v1) * (v0 + v1) - (v0 + vM1) * (v0 + vM1)) / (4 * dyLong1);

    const FLOAT firstOrder = 1.0 / (4.0 * dyShort) * (
        kr * (v0 + v1) - kl * (vM1 + v0) + fabs(kr) * (v0 - v1) - fabs(kl) * (vM1 - v0));

    const FLOAT tmp2 = (1.0 - parameters.solver.gamma) * secondOrder + parameters.solver.gamma * firstOrder;

#ifndef NDEBUG
    if (fabs(tmp1 - tmp2) > 1.0e-12) {
        HANDLE_ERROR(1, "Error in dv2dy");
    }
#endif

    return tmp2;
}

// First derivative of w*w w.r.t. z, evaluated at location of w-component. For details, see du2dx.
inline FLOAT dw2dz(const FLOAT*  const lv, const Parameters & parameters, const FLOAT* const lm) {
#ifndef NDEBUG
    const FLOAT tmp1 = 1.0 / 4.0 * ((((lv[mapd(0, 0, 0, 2)] + lv[mapd(0, 0, 1, 2)]) *
        (lv[mapd(0, 0, 0, 2)] + lv[mapd(0, 0, 1, 2)])) -
        ((lv[mapd(0, 0, -1, 2)] + lv[mapd(0, 0, 0, 2)]) *
            (lv[mapd(0, 0, -1, 2)] + lv[mapd(0, 0, 0, 2)]))) +
        parameters.solver.gamma * ((fabs(lv[mapd(0, 0, 0, 2)] + lv[mapd(0, 0, 1, 2)]) *
            (lv[mapd(0, 0, 0, 2)] - lv[mapd(0, 0, 1, 2)])) -
            (fabs(lv[mapd(0, 0, -1, 2)] + lv[mapd(0, 0, 0, 2)]) *
                (lv[mapd(0, 0, -1, 2)] - lv[mapd(0, 0, 0, 2)])))) /
        lm[mapd(0, 0, 0, 2)];
#endif

    const FLOAT dzShort = 0.5 * lm[mapd(0, 0, 0, 2)];
    // const FLOAT dzLong0 = 0.5 * (lm[mapd(0, 0, -1, 2)] + lm[mapd(0, 0, 0, 2)]);
    const FLOAT dzLong1 = 0.5 * (lm[mapd(0, 0, 0, 2)] + lm[mapd(0, 0, 1, 2)]);

    const FLOAT w0 = lv[mapd(0, 0, 0, 2)];
    const FLOAT wM1 = lv[mapd(0, 0, -1, 2)];
    const FLOAT w1 = lv[mapd(0, 0, 1, 2)];

    // const FLOAT kr = (dzLong1 - dzShort) / dzLong1 * w0 + dzShort / dzLong1 * w1;
    // const FLOAT kl = (dzLong0 - dzShort) / dzLong0 * w0 + dzShort / dzLong0 * wM1;
    const FLOAT kr = (w0 + w1) / 2;
    const FLOAT kl = (w0 + wM1) / 2;

    /* const FLOAT secondOrder = (((dzLong1 - dzShort) / dzLong1 * w0 + dzShort / dzLong1 * w1) * ((dzLong1 - dzShort) / dzLong1 * w0 + dzShort / dzLong1 * w1)
        - ((dzLong0 - dzShort) / dzLong0 * w0 + dzShort / dzLong0 * wM1) * ((dzLong0 - dzShort) / dzLong0 * w0 + dzShort / dzLong0 * wM1)
        ) / (2.0 * dzShort); */

    const FLOAT secondOrder = ((w0 + w1) * (w0 + w1) - (w0 + wM1) * (w0 + wM1)) / (4 * dzLong1);

    const FLOAT firstOrder = 1.0 / (4.0 * dzShort) * (
        kr * (w0 + w1) - kl * (wM1 + w0) + fabs(kr) * (w0 - w1) - fabs(kl) * (wM1 - w0));

    const FLOAT tmp2 = (1.0 - parameters.solver.gamma) * secondOrder + parameters.solver.gamma * firstOrder;

#ifndef NDEBUG
    if (fabs(tmp1 - tmp2) > 1.0e-12) {
        HANDLE_ERROR(1, "Error in dw2dz");
    }
#endif

    return tmp2;
}

/**
 * Turbulence Modelling
 */

inline FLOAT FT_term1(const FLOAT* const lv, const FLOAT* const lm, FLOAT vijk, FLOAT vi1jk) {
    // vijk: total viscosity: vstar[i,j,k]
    // vi1jk: total viscosity: vstar[i+1,j,k]

    const int index0 = mapd(0, 0, 0, 0); // u[i,j,k]
    const int index1 = mapd(-1, 0, 0, 0); // u[i-1,j,k]
    const int index2 = mapd(1, 0, 0, 0); // u[i+1,j,k]

    //firstTerm: vstar[i+1,j,k]*(u[i+1,j,k] - u[i,j,k])
    FLOAT firstTerm = vi1jk * (lv[index2] - lv[index0]);

    //secondTerm: vstar[i,j,k]*(u[i,j,k] - u[i-1,j,k])
    FLOAT secondTerm = vijk * (lv[index0] - lv[index1]);

    //return 2*(d/dx)(vstar*(du/dx))
    return 2 * (firstTerm - secondTerm) / (lm[index0] * lm[index0]);
}

inline FLOAT FT_term2(const FLOAT* const lv, const FLOAT* const lm, FLOAT vtr, FLOAT vbr) {
    // vtr: viscosity at top right corner: v*[i+1/2, j+1/2, k]
    // vbr: viscosity at bottom right corner: v*[i+1/2, j-1/2, k]

    const int index0 = mapd(0, 0, 0, 0); // u[i,j,k]
    const int index1 = mapd(0, 1, 0, 0); // u[i,j+1,k]
    const int index2 = mapd(0, -1, 0, 0); // u[i,j-1,k]
    const int index3 = mapd(0, 0, 0, 1); // v[i,j,k]
    const int index4 = mapd(1, 0, 0, 1); // v[i+1,j,k]
    const int index5 = mapd(0, -1, 0, 1); // v[i,j-1,k]
    const int index6 = mapd(1, -1, 0, 1); // v[i+1,j-1,k]

    // firstTerm: vstar[i+1/2, j+1/2, k] * ((u[i,j+1,k]-u[i,j,k])/dy + (v[i+1,j,k]-v[i,j,k])/dx)
    FLOAT firstTerm = vtr * ((lv[index1] - lv[index0]) / lm[index3] + (lv[index4] - lv[index3]) / lm[index0]);

    // secondTerm: vstar[i+1/2, j-1/2, k] * ((u[i,j,k]-u[i,j-1,k])/dy + (v[i+1,j-1,k]-v[i,j-1,k])/dx)
    FLOAT secondTerm = vbr * ((lv[index0] - lv[index2]) / lm[index3] + (lv[index6] - lv[index5]) / lm[index0]);

    //return (d/dy)*(vstar*(du/dy + dv/dx))
    return (firstTerm - secondTerm) / lm[index3];
}

inline FLOAT FT_term3(const FLOAT* const lv, const FLOAT* const lm, FLOAT vrf, FLOAT vrb) {
    // vrf: viscosity at right front corner: v*[i+1/2, j, k+1/2]
    // vrb: viscosity at right back corner: v*[i+1/2, j, k-1/2]

    const int index0 = mapd(0, 0, 0, 0); // u[i,j,k]
    const int index1 = mapd(0, 0, 1, 0); // u[i,j,k+1]
    const int index2 = mapd(0, 0, -1, 0); // u[i,j,k-1]
    const int index3 = mapd(0, 0, 0, 2); // w[i,j,k]
    const int index4 = mapd(1, 0, 0, 2); // w[i+1,j,k]
    const int index5 = mapd(0, 0, -1, 2); // w[i,j,k-1]
    const int index6 = mapd(1, 0, -1, 2); // w[i+1,j,k-1]

    // firstTerm: vstar[i+1/2, j, k+1/2] * ((u[i,j,k+1]-u[i,j,k])/dz + (w[i+1,j,k]-w[i,j,k])/dx)
    FLOAT firstTerm = vrf * ((lv[index1] - lv[index0]) / lm[index3] + (lv[index4] - lv[index3]) / lm[index0]);

    // secondTerm: vstar[i+1/2, j, k-1/2] * ((u[i,j,k]-u[i,j,k-1])/dz + (w[i+1,j,k-1]-w[i,j,k-1])/dx)
    FLOAT secondTerm = vrb * ((lv[index0] - lv[index2]) / lm[index3] + (lv[index6] - lv[index5]) / lm[index0]);

    //return (d/dz)*(vstar*(du/dz + dw/dx))
    return (firstTerm - secondTerm) / lm[index3];
}

/**
 * Computes the F term for 2D Turbulence momentum equations
 */
inline FLOAT computeF2DT(const FLOAT* const localVelocity, const FLOAT* const localMeshsize, const FLOAT* const localViscosity, const Parameters &parameters, FLOAT dt) {
    const int index0 = mapd(0, 0, 0,0); // vstar[i,j,k]
    const int index1 = mapd(1, 0, 0, 0); // vstar[i+1,j,k]
    const int index2 = mapd(0, 1, 0, 0); // vstar[i,j+1,k]
    const int index3 = mapd(1, 1, 0, 0); // vstar[i+1,j+1,k]
    const int index4 = mapd(0, -1, 0, 0); // vstar[i,j-1,k]
    const int index5 = mapd(1, -1, 0, 0); // vstar[i+1,j-1,k]

    // vijk: total viscosity: vstar[i,j,k]
    FLOAT vijk = localViscosity[index0];

    // vi1jk: total viscosity: vstar[i+1,j,k]
    FLOAT vi1jk = localViscosity[index1];

    // vtr: viscosity at top right corner: v*[i+1/2, j+1/2, k]
    FLOAT vtr = (localViscosity[index3] + localViscosity[index2] + localViscosity[index1] + localViscosity[index0]) / 4;

    // vbr: viscosity at bottom right corner: v*[i+1/2, j-1/2, k]
    FLOAT vbr = (localViscosity[index5] + localViscosity[index4] + localViscosity[index1] + localViscosity[index0]) / 4;

    FLOAT term1 = FT_term1(localVelocity, localMeshsize, vijk, vi1jk);
    FLOAT term2 = FT_term2(localVelocity, localMeshsize, vtr, vbr);

    return localVelocity[mapd(0, 0, 0, 0)] + dt * (term1 + term2  - du2dx(localVelocity, parameters, localMeshsize)
            - duvdy(localVelocity, parameters, localMeshsize) + parameters.environment.gx);
}

/**
 * Computes the F term for 3D Turbulence momentum equations
 */
inline FLOAT computeF3DT(const FLOAT* const localVelocity, const FLOAT* const localMeshsize, const FLOAT* const localViscosity, const Parameters& parameters, FLOAT dt) {
    const int index0 = mapd(0, 0, 0, 0); // vstar[i,j,k]
    const int index1 = mapd(1, 0, 0, 0); // vstar[i+1,j,k]
    const int index2 = mapd(0, 1, 0, 0); // vstar[i,j+1,k]
    const int index3 = mapd(1, 1, 0, 0); // vstar[i+1,j+1,k]
    const int index4 = mapd(0, -1, 0, 0); // vstar[i,j-1,k]
    const int index5 = mapd(1, -1, 0, 0); // vstar[i+1,j-1,k]
    const int index6 = mapd(0, 0, 1, 0); // vstar[i,j,k+1]
    const int index7 = mapd(1, 0, 1, 0); // vstar[i+1,j,k+1]
    const int index8 = mapd(0, 0, -1, 0); // vstar[i,j,k-1]
    const int index9 = mapd(1, 0, -1, 0); // vstar[i+1,j,k-1]

    // vijk: total viscosity: vstar[i,j,k]
    FLOAT vijk = localViscosity[index0];

    // vi1jk: total viscosity: vstar[i+1,j,k]
    FLOAT vi1jk = localViscosity[index1];

    // vtr: viscosity at top right corner: v*[i+1/2, j+1/2, k]
    FLOAT vtr = (localViscosity[index3] + localViscosity[index2] + localViscosity[index1] + localViscosity[index0]) / 4;

    // vbr: viscosity at bottom right corner: v*[i+1/2, j-1/2, k]
    FLOAT vbr = (localViscosity[index5] + localViscosity[index4] + localViscosity[index1] + localViscosity[index0]) / 4;

    // vrf: viscosity at right front corner: v*[i+1/2, j, k+1/2]
    FLOAT vrf = (localViscosity[index0] + localViscosity[index1] + localViscosity[index6] + localViscosity[index7]) / 4;

    // vrb: viscosity at right back corner: v*[i+1/2, j, k-1/2]
    FLOAT vrb = (localViscosity[index0] + localViscosity[index1] + localViscosity[index8] + localViscosity[index9]) / 4;

    FLOAT term1 = FT_term1(localVelocity, localMeshsize, vijk, vi1jk);
    FLOAT term2 = FT_term2(localVelocity, localMeshsize, vtr, vbr);
    FLOAT term3 = FT_term3(localVelocity, localMeshsize, vrf, vrb);

    return localVelocity[mapd(0, 0, 0, 0)] + dt * (term1 + term2 + term3
		    - du2dx(localVelocity, parameters, localMeshsize)
		    - duvdy(localVelocity, parameters, localMeshsize)
		    - duwdz(localVelocity, parameters, localMeshsize) + parameters.environment.gx);
}

inline FLOAT GT_term1(const FLOAT* const lv, const FLOAT* const lm, FLOAT vtr, FLOAT vtl) {
    // vtr: viscosity at top right corner: v*[i+1/2, j+1/2, k]
    // vtl: viscosity at top left corner: v*[i-1/2, j+1/2, k]

    const int index0 = mapd(0, 0, 0, 0); // u[i,j,k]
    const int index1 = mapd(0, 1, 0, 0); // u[i,j+1,k]
    // Not used: const int index2 = mapd(0, -1, 0, 0); // u[i,j-1,k]
    const int index3 = mapd(0, 0, 0, 1); // v[i,j,k]
    const int index4 = mapd(1, 0, 0, 1); // v[i+1,j,k]
    // Not used: const int index5 = mapd(0, -1, 0, 1); // v[i,j-1,k]
    // Not used: const int index6 = mapd(1, -1, 0, 1); // v[i+1,j-1,k]
    const int index7 = mapd(-1, 0, 0, 1); // v[i-1,j,k]
    const int index8 = mapd(-1, 1, 0, 0); // u[i-1,j+1,k]
    const int index9 = mapd(-1, 0, 0, 0); // u[i-1,j,k]

    // firstTerm: vstar[i+1/2, j+1/2, k] * ((u[i,j+1,k]-u[i,j,k])/dy + (v[i+1,j,k]-v[i,j,k])/dx)
    FLOAT firstTerm = vtr * ((lv[index1] - lv[index0]) / lm[index3] + (lv[index4] - lv[index3]) / lm[index0]);

    // secondTerm: vstar[i-1/2, j+1/2, k] * ((v[i,j,k]-v[i-1,j,k])/dx + (u[i-1,j+1,k]-u[i-1,j,k])/dy)
    FLOAT secondTerm = vtl * ((lv[index3] - lv[index7]) / lm[index0] + (lv[index8] - lv[index9]) / lm[index3]);

    //return (d/dx)(vstar*(dv/dx + du/dy))
    return (firstTerm - secondTerm) / lm[index0];
}

inline FLOAT GT_term2(const FLOAT* const lv, const FLOAT* const lm, FLOAT vijk, FLOAT vij1k) {
    // vijk: total viscosity: vstar[i,j,k]
    // vij1k: total viscosity: vstar[i,j+1,k]

    const int index0 = mapd(0, 0, 0, 1); // v[i,j,k]
    const int index1 = mapd(0, -1, 0, 1); // v[i,j-1,k]
    const int index2 = mapd(0, 1, 0, 1); // v[i,j+1,k]

    //firstTerm: vstar[i,j+1,k]*(v[i,j+1,k] - v[i,j,k])
    FLOAT firstTerm = vij1k * (lv[index2] - lv[index0]);

    //secondTerm: vstar[i,j,k]*(v[i,j,k] - v[i,j-1,k])
    FLOAT secondTerm = vijk * (lv[index0] - lv[index1]);

    //return 2*(d/dy)(vstar*(dv/dy))
    return 2 * (firstTerm - secondTerm) / (lm[index0] * lm[index0]);
}

inline FLOAT GT_term3(const FLOAT* const lv, const FLOAT* const lm, FLOAT vtf, FLOAT vtb) {
    // vtf: viscosity at top front corner: v*[i, j+1/2, k+1/2]
    // vtb: viscosity at top back corner: v*[i, j+1/2, k-1/2]

    const int index0 = mapd(0, 0, 0, 1); // v[i,j,k]
    const int index1 = mapd(0, 0, 1, 1); // v[i,j,k+1]
    const int index2 = mapd(0, 0, -1, 1); // v[i,j,k-1]
    const int index3 = mapd(0, 0, 0, 2); // w[i,j,k]
    const int index4 = mapd(0, 1, 0, 2); // w[i,j+1,k]
    const int index5 = mapd(0, 0, -1, 2); // w[i,j,k-1]
    const int index6 = mapd(0, 1, -1, 2); // w[i,j+1,k-1]

    // firstTerm: vstar[i, j+1/2, k+1/2] * ((v[i,j,k+1]-v[i,j,k])/dz + (w[i,j+1,k]-w[i,j,k])/dy)
    FLOAT firstTerm = vtf * ((lv[index1] - lv[index0]) / lm[index3] + (lv[index4] - lv[index3]) / lm[index0]);

    // secondTerm: vstar[i, j+1/2, k-1/2] * ((v[i,j,k]-v[i,j,k-1])/dz + (w[i,j+1,k-1]-w[i,j,k-1])/dy)
    FLOAT secondTerm = vtb * ((lv[index0] - lv[index2]) / lm[index3] + (lv[index6] - lv[index5]) / lm[index0]);

    //return (d/dz)*(vstar*(dv/dz + dw/dy))
    return (firstTerm - secondTerm) / lm[index3];
}

/**
 * Computes the G term for 2D Turbulence momentum equations
 */
inline FLOAT computeG2DT(const FLOAT* const localVelocity, const FLOAT* const localMeshsize, const FLOAT* const localViscosity, const Parameters& parameters, FLOAT dt) {
    const int index0 = mapd(0, 0, 0, 0); // vstar[i,j,k]
    const int index1 = mapd(1, 0, 0, 0); // vstar[i+1,j,k]
    const int index2 = mapd(0, 1, 0, 0); // vstar[i,j+1,k]
    const int index3 = mapd(1, 1, 0, 0); // vstar[i+1,j+1,k]
    const int index4 = mapd(-1, 0, 0, 0); // vstar[i-1,j,k]
    const int index5 = mapd(-1, 1, 0, 0); // vstar[i-1,j+1,k]

    // vijk: total viscosity: vstar[i,j,k]
    FLOAT vijk =  localViscosity[index0];
    // vij1k: total viscosity: vstar[i,j+1,k]
    FLOAT vij1k =  localViscosity[index2];
    // vtr: viscosity at top right corner: v*[i+1/2, j+1/2, k]
    FLOAT vtr = (localViscosity[index3] + localViscosity[index2] + localViscosity[index1] + localViscosity[index0])/4;
    // vtl: viscosity at top left corner: v*[i-1/2, j+1/2, k]
    FLOAT vtl = (localViscosity[index0] + localViscosity[index2] + localViscosity[index4] + localViscosity[index5])/4;


    FLOAT term1 = GT_term1(localVelocity, localMeshsize, vtr, vtl);
    FLOAT term2 = GT_term2(localVelocity, localMeshsize, vijk, vij1k);

    return localVelocity[mapd(0, 0, 0, 1)] +
            dt * (term1 + term2 - duvdx(localVelocity, parameters, localMeshsize)
            - dv2dy(localVelocity, parameters, localMeshsize) + parameters.environment.gy);
}

/**
 * Computes the G term for 3D Turbulence momentum equations
 */
inline FLOAT computeG3DT(const FLOAT* const localVelocity, const FLOAT* const localMeshsize, const FLOAT* const localViscosity, const Parameters& parameters, FLOAT dt) {
	const int index0 = mapd(0, 0, 0, 0); // vstar[i,j,k]
    const int index1 = mapd(1, 0, 0, 0); // vstar[i+1,j,k]
    const int index2 = mapd(0, 1, 0, 0); // vstar[i,j+1,k]
    const int index3 = mapd(1, 1, 0, 0); // vstar[i+1,j+1,k]
    const int index4 = mapd(-1, 0, 0, 0); // vstar[i-1,j,k]
    const int index5 = mapd(-1, 1, 0, 0); // vstar[i-1,j+1,k]
    const int index6 = mapd(0, 0, 1, 0); // vstar[i,j,k+1]
    const int index7 = mapd(0, 1, 1, 0); // vstar[i,j+1,k+1]
    const int index8 = mapd(0, 0, -1, 0); // vstar[i,j,k-1]
    const int index9 = mapd(0, 1, -1, 0); // vstar[i,j+1,k-1]

    // vijk: total viscosity: vstar[i,j,k]
    FLOAT vijk =  localViscosity[index0];

    // vij1k: total viscosity: vstar[i,j+1,k]
    FLOAT vij1k =  localViscosity[index2];

    // vtr: viscosity at top right corner: v*[i+1/2, j+1/2, k]
    FLOAT vtr = (localViscosity[index3] + localViscosity[index2] + localViscosity[index1] + localViscosity[index0]) / 4;

    // vtl: viscosity at top left corner: v*[i-1/2, j+1/2, k]
    FLOAT vtl = (localViscosity[index0] + localViscosity[index2] + localViscosity[index4] + localViscosity[index5]) / 4;

    // vtf: viscosity at top front corner: v*[i, j+1/2, k+1/2]
    FLOAT vtf = (localViscosity[index0] + localViscosity[index2] + localViscosity[index6] + localViscosity[index7]) / 4;

    // vtb: viscosity at top back corner: v*[i, j+1/2, k-1/2]
    FLOAT vtb = (localViscosity[index0] + localViscosity[index2] + localViscosity[index8] + localViscosity[index9]) / 4;

    FLOAT term1 = GT_term1(localVelocity, localMeshsize, vtr, vtl);
    FLOAT term2 = GT_term2(localVelocity, localMeshsize, vijk, vij1k);
    FLOAT term3 = GT_term3(localVelocity, localMeshsize, vtf, vtb);

    return localVelocity[mapd(0, 0, 0, 1)] + dt * (term1 + term2 + term3
		    - dv2dy(localVelocity, parameters, localMeshsize) - duvdx(localVelocity, parameters, localMeshsize)
		    - dvwdz(localVelocity, parameters, localMeshsize) + parameters.environment.gy);
}

inline FLOAT HT_term1(const FLOAT* const lv, const FLOAT* const lm, FLOAT vfr, FLOAT vfl) {
    // vfr: viscosity at front right corner: v*[i+1/2, j, k+1/2]
    // vfl: viscosity at front left corner: v*[i-1/2, j, k+1/2]

    const int index0 = mapd(0, 0, 0, 0); // u[i,j,k]
    const int index1 = mapd(0, 0, 1, 0); // u[i,j,k+1]
    const int index2 = mapd(-1, 0, 0, 2); // w[i-1,j,k]
    const int index3 = mapd(0, 0, 0, 2); // w[i,j,k]
    const int index4 = mapd(1, 0, 0, 2); // w[i+1,j,k]
    const int index5 = mapd(-1, 0, 0, 0); // u[i-1,j,k]
    const int index6 = mapd(-1, 0, 1, 0); // u[i-1,j,k+1]

    // firstTerm: vstar[i+1/2, j, k+1/2] * ((u[i,j,k+1]-u[i,j,k])/dz + (w[i+1,j,k]-w[i,j,k])/dx)
    FLOAT firstTerm = vfr * ((lv[index1] - lv[index0]) / lm[index3] + (lv[index4] - lv[index3]) / lm[index0]);

    // secondTerm: vstar[i+1/2, j, k-1/2] * ((w[i,j,k]-w[i-1,j,k])/dx + (u[i-1,j,k+1]-u[i-1,j,k])/dz)
    FLOAT secondTerm = vfl * ((lv[index3] - lv[index2]) / lm[index0] + (lv[index6] - lv[index5]) / lm[index3]);

    //return (d/dz)*(vstar*(du/dz + dw/dx))
    return (firstTerm - secondTerm) / lm[index0];
}

inline FLOAT HT_term2(const FLOAT* const lv, const FLOAT* const lm, FLOAT vft, FLOAT vfb) {
    // vft: viscosity at front top corner: v*[i, j+1/2, k+1/2]
    // vfb: viscosity at front bottom corner: v*[i, j-1/2, k+1/2]

    const int index0 = mapd(0, 0, 0, 1); // v[i,j,k]
    const int index1 = mapd(0, 0, 1, 1); // v[i,j,k+1]
    const int index2 = mapd(0, -1, 0, 2); // w[i,j-1,k]
    const int index3 = mapd(0, 0, 0, 2); // w[i,j,k]
    const int index4 = mapd(0, 1, 0, 2); // w[i,j+1,k]
    const int index5 = mapd(0, -1, 0, 1); // v[i,j-1,k]
    const int index6 = mapd(0, -1, 1, 1); // v[i,j-1,k+1]

    // firstTerm: vstar[i, j+1/2, k+1/2] * ((v[i,j,k+1]-v[i,j,k])/dz + (w[i,j+1,k]-w[i,j,k])/dy)
    FLOAT firstTerm = vft * ((lv[index1] - lv[index0]) / lm[index3] + (lv[index4] - lv[index3]) / lm[index0]);

    // secondTerm: vstar[i, j+1/2, k-1/2] * ((w[i,j,k]-w[i,j-1,k])/dy + (v[i,j-1,k+1]-v[i,j-1,k])/dz)
    FLOAT secondTerm = vfb * ((lv[index3] - lv[index2]) / lm[index0] + (lv[index6] - lv[index5]) / lm[index3]);

    //return (d/dz)*(vstar*(dv/dz + dw/dy))
    return (firstTerm - secondTerm) / lm[index0];
}

inline FLOAT HT_term3(const FLOAT* const lv, const FLOAT* const lm, FLOAT vijk, FLOAT vijk1) {
    // vijk: total viscosity: vstar[i,j],j
    // vijk1: total viscosity: vstar[i,j,k+1]

    const int index0 = mapd(0, 0, 0, 2); // w[i,j,k]
    const int index1 = mapd(0, 0, -1, 2); // w[i,j,k-1]
    const int index2 = mapd(0, 0, 1, 2); // w[i,j,k+1]

    //firstTerm: vstar[i,j,k+1]*(w[i,j,k+1] - w[i,j,k])
    FLOAT firstTerm = vijk1 * (lv[index2] - lv[index0]);

    //secondTerm: vstar[i,j,k]*(w[i,j,k] - w[i,j,k-1])
    FLOAT secondTerm = vijk * (lv[index0] - lv[index1]);

    //return 2*(d/dx)(vstar*(du/dx))
    return 2 * (firstTerm - secondTerm) / (lm[index0] * lm[index0]);
}

/**
 * Computes the H term for 3D Turbulence momentum equations
 */
inline FLOAT computeH3DT(const FLOAT* const localVelocity, const FLOAT* const localMeshsize, const FLOAT* const localViscosity, const Parameters& parameters, FLOAT dt) {
    const int index0 = mapd(0, 0, 0, 0); // vstar[i,j,k]
    const int index1 = mapd(1, 0, 0, 0); // vstar[i+1,j,k]
    const int index2 = mapd(0, 1, 0, 0); // vstar[i,j+1,k]
    const int index3 = mapd(1, 0, 1, 0); // vstar[i+1,j,k+1]
    const int index4 = mapd(-1, 0, 0, 0); // vstar[i-1,j,k]
    const int index5 = mapd(-1, 0, 1, 0); // vstar[i-1,1,k+1]

    const int index6 = mapd(0, 0, 1, 0); // vstar[i,j,k+1]
    const int index7 = mapd(0, 1, 1, 0); // vstar[i,j+1,k+1]

    const int index8 = mapd(0, -1, 0, 0); // vstar[i,j-1,k]
    const int index9 = mapd(0, -1, 1, 0); // vstar[i,j-1,k+1]

    // vijk: total viscosity: vstar[i,j,k]
    FLOAT vijk =  localViscosity[index0];

    // vijk1: total viscosity: vstar[i,j,k+1]
    FLOAT vijk1 =  localViscosity[index6];

    // vfr: viscosity at front right corner: v*[i+1/2, j, k+1/2]
    FLOAT vfr = (localViscosity[index0] + localViscosity[index6] + localViscosity[index1] + localViscosity[index3]) / 4;

    // vfl: viscosity at front left corner: v*[i-1/2, j, k+1/2]
    FLOAT vfl = (localViscosity[index0] + localViscosity[index6] + localViscosity[index4] + localViscosity[index5]) / 4;

    // vft: viscosity at front top corner: v*[i, j+1/2, k+1/2]
    FLOAT vft = (localViscosity[index0] + localViscosity[index6] + localViscosity[index2] + localViscosity[index7]) / 4;

    // vfb: viscosity at front bottom corner: v*[i, j-1/2, k+1/2]
    FLOAT vfb = (localViscosity[index0] + localViscosity[index6] + localViscosity[index8] + localViscosity[index9]) / 4;

    FLOAT term1 = HT_term1(localVelocity, localMeshsize, vfr, vfl);
    FLOAT term2 = HT_term2(localVelocity, localMeshsize, vft, vfb);
    FLOAT term3 = HT_term3(localVelocity, localMeshsize, vijk, vijk1);

    return localVelocity[mapd(0, 0, 0, 2)] + dt * (term1 + term2 + term3
		    - dw2dz(localVelocity, parameters, localMeshsize) - duwdx(localVelocity, parameters, localMeshsize)
		    - dvwdy(localVelocity, parameters, localMeshsize)  + parameters.environment.gz);
}

inline FLOAT computeF2D(const FLOAT* const localVelocity, const FLOAT* const localMeshsize, const Parameters& parameters, FLOAT dt) {
    return localVelocity[mapd(0, 0, 0, 0)]
        + dt * (1 / parameters.flow.Re * (d2udx2(localVelocity, localMeshsize)
            + d2udy2(localVelocity, localMeshsize)) - du2dx(localVelocity, parameters, localMeshsize)
            - duvdy(localVelocity, parameters, localMeshsize) + parameters.environment.gx);
}

inline FLOAT computeG2D(const FLOAT* const localVelocity, const FLOAT* const localMeshsize, const Parameters& parameters, FLOAT dt) {
    return localVelocity[mapd(0, 0, 0, 1)]
        + dt * (1 / parameters.flow.Re * (d2vdx2(localVelocity, localMeshsize)
            + d2vdy2(localVelocity, localMeshsize)) - duvdx(localVelocity, parameters, localMeshsize)
            - dv2dy(localVelocity, parameters, localMeshsize) + parameters.environment.gy);
}

inline FLOAT computeF3D(const FLOAT* const localVelocity, const FLOAT* const localMeshsize, const Parameters& parameters, FLOAT dt) {
    return localVelocity[mapd(0, 0, 0, 0)]
        + dt * (1 / parameters.flow.Re * (d2udx2(localVelocity, localMeshsize)
            + d2udy2(localVelocity, localMeshsize) + d2udz2(localVelocity, localMeshsize))
            - du2dx(localVelocity, parameters, localMeshsize) - duvdy(localVelocity, parameters, localMeshsize)
            - duwdz(localVelocity, parameters, localMeshsize) + parameters.environment.gx);
}

inline FLOAT computeG3D(const FLOAT* const localVelocity, const FLOAT* const localMeshsize, const Parameters& parameters, FLOAT dt) {
    return localVelocity[mapd(0, 0, 0, 1)]
        + dt * (1 / parameters.flow.Re * (d2vdx2(localVelocity, localMeshsize)
            + d2vdy2(localVelocity, localMeshsize) + d2vdz2(localVelocity, localMeshsize))
            - dv2dy(localVelocity, parameters, localMeshsize) - duvdx(localVelocity, parameters, localMeshsize)
            - dvwdz(localVelocity, parameters, localMeshsize) + parameters.environment.gy);
}

inline FLOAT computeH3D(const FLOAT* const localVelocity, const FLOAT* const localMeshsize, const Parameters& parameters, FLOAT dt) {
    return localVelocity[mapd(0, 0, 0, 2)]
        + dt * (1 / parameters.flow.Re * (d2wdx2(localVelocity, localMeshsize)
            + d2wdy2(localVelocity, localMeshsize) + d2wdz2(localVelocity, localMeshsize))
            - dw2dz(localVelocity, parameters, localMeshsize) - duwdx(localVelocity, parameters, localMeshsize)
            - dvwdy(localVelocity, parameters, localMeshsize) + parameters.environment.gz);
}

// dudy <-> first derivative of u-component of velocity field w.r.t. y-direction.
inline FLOAT dudy(const FLOAT* const lv, const FLOAT* const lm) {
    // Evaluate dudy in the cell center by a central difference
    const int index0 = mapd(0, 0, 0, 0);
    const int index1 = mapd(0, -1, 0, 0);

    return (lv[index0] - lv[index1]) / lm[mapd(0, 0, 0, 1)];
}

// dudz <-> first derivative of u-component of velocity field w.r.t. z-direction.
inline FLOAT dudz(const FLOAT* const lv, const FLOAT* const lm) {
    // Evaluate dudz in the cell center by a central difference
    const int index0 = mapd(0, 0, 0, 0);
    const int index1 = mapd(0, 0, -1, 0);

    return (lv[index0] - lv[index1]) / lm[mapd(0, 0, 0, 2)];
}

// dvdx <-> first derivative of v-component of velocity field w.r.t. x-direction.
inline FLOAT dvdx(const FLOAT* const lv, const FLOAT* const lm) {
    const int index0 = mapd(0, 0, 0, 1);
    const int index1 = mapd(-1, 0, 0, 1);

    return (lv[index0] - lv[index1]) / lm[mapd(0, 0, 0, 0)];
}

// dvdz <-> first derivative of v-component of velocity field w.r.t. z-direction.
inline FLOAT dvdz(const FLOAT* const lv, const FLOAT* const lm) {
    // Evaluate dudz in the cell center by a central difference
    const int index0 = mapd(0, 0, 0, 1);
    const int index1 = mapd(0, 0, -1, 1);

    return (lv[index0] - lv[index1]) / lm[mapd(0, 0, 0, 2)];
}

// dwdx <-> first derivative of w-component of velocity field w.r.t. x-direction.
inline FLOAT dwdx(const FLOAT* const lv, const FLOAT* const lm) {
    // Evaluate dwdx in the cell center by a central difference
    const int index0 = mapd(0, 0, 0, 2);
    const int index1 = mapd(-1, 0, 0, 2);

    return (lv[index0] - lv[index1]) / lm[mapd(0, 0, 0, 0)];
}

// dwdy <-> first derivative of w-component of velocity field w.r.t. y-direction.
inline FLOAT dwdy(const FLOAT* const lv, const FLOAT* const lm) {
    // Evaluate dwdy in the cell center by a central difference
    const int index0 = mapd(0, 0, 0, 2);
    const int index1 = mapd(0, -1, 0, 2);

    return (lv[index0] - lv[index1]) / lm[mapd(0, 0, 0, 1)];
}

// function to compute the strain tensor squared in 2D
inline FLOAT computeStrainTensorSquared2D(const FLOAT* const localVelocity, const FLOAT* const localMeshsize) {
    FLOAT S11 = 2 * dudx(localVelocity, localMeshsize);
    FLOAT S22 = 2 * dvdy(localVelocity, localMeshsize);
    FLOAT S12 = dudy(localVelocity, localMeshsize) + dvdx(localVelocity, localMeshsize);

    return std::pow(S11, 2) + std::pow(S22, 2) + 2 * std::pow(S12, 2);
}

// function to compute the strain tensor squared in 3D
inline FLOAT computeStrainTensorSquared3D(const FLOAT* const localVelocity, const FLOAT* const localMeshsize) {
	FLOAT S11 = 2 * dudx(localVelocity, localMeshsize);
	FLOAT S22 = 2 * dvdy(localVelocity, localMeshsize);
	FLOAT S33 = 2 * dwdz(localVelocity, localMeshsize);
	FLOAT S12 = dudy(localVelocity, localMeshsize) + dvdx(localVelocity, localMeshsize);
	FLOAT S13 = dudz(localVelocity, localMeshsize) + dwdx(localVelocity, localMeshsize);
	FLOAT S23 = dvdz(localVelocity, localMeshsize) + dwdy(localVelocity, localMeshsize);

    return std::pow(S11, 2) + std::pow(S22, 2) + std::pow(S33, 2) +
           2 * (std::pow(S12, 2) + std::pow(S13, 2) + std::pow(S23, 2));
}

} // namespace NSEOF::Stencils

#endif // __STENCILS_STENCIL_FUNCTIONS_HPP__
