#include "DistanceStencil.hpp"

namespace NSEOF::Stencils {

DistanceStencil::DistanceStencil(const Parameters& parameters, const int cellsX, const int cellsY, const int cellsZ)
    : FieldStencil<FlowField>(parameters)
    , cellsX_(cellsX), cellsY_(cellsY), cellsZ_(cellsZ)
    , leftWall_(parameters_.parallel.leftNb == MPI_PROC_NULL && parameters_.simulation.scenario != "channel")
    , rightWall_(parameters_.parallel.rightNb == MPI_PROC_NULL && parameters_.simulation.scenario != "channel")
    , bottomWall_(parameters_.parallel.bottomNb == MPI_PROC_NULL)
    , topWall_(parameters_.parallel.topNb == MPI_PROC_NULL)
    , frontWall_(parameters_.parallel.frontNb == MPI_PROC_NULL)
    , backWall_(parameters_.parallel.backNb == MPI_PROC_NULL)
    , stepXBound_(ceil(cellsX * parameters.bfStep.xRatio)), stepYBound_(ceil(cellsY * parameters.bfStep.yRatio)) {}

/**
 * Calculates the distance to the nearest wall in the given direction
 *
 * Note: To limit if conditions we are only checking for u-velocities on each wall
 */
FLOAT DistanceStencil::calculateDistToNearestWallInGivenDir_(const bool firstWall, const bool secondWall,
                                                             const int idx, const FLOAT cellSize, const FLOAT sizeInOneDir) {
    const FLOAT firstDist = idx;
    const FLOAT secondDist = sizeInOneDir - idx;

    if (firstWall && secondWall) {
        const int closestDist = idx <= sizeInOneDir / 2 ? firstDist : secondDist;
        return std::abs(closestDist * cellSize);
    } else if (firstWall) {
        return std::abs(firstDist * cellSize);
    } else if (secondWall) {
        return std::abs(secondDist * cellSize);
    }

    // distance is maximal if both walls are not actually walls!
    return std::numeric_limits<FLOAT>::infinity();
}

/**
 * Only goes into the following loops if there is a step
 *
 * Note: we check both loops in the same z coordinate as the original point
 *       so don't need to count it as there is no step in the Z-axis
 */
void DistanceStencil::calculateSteps_(FLOAT& distToWall, int i, int j, int k) {
    if (stepXBound_ * stepYBound_ == 0) {
        return;
    }

    // Top boundary loop
    for (int x = 0; x <= stepXBound_; x++) {
        FLOAT dx = (i - x) * parameters_.meshsize->getDx(i, j, k);
        FLOAT dy = (j - stepYBound_) * parameters_.meshsize->getDy(i, j, k);
        FLOAT stepDis = sqrt((dx * dx) + (dy * dy));

        distToWall = std::abs(std::min(distToWall, stepDis));
    }

    // Left boundary loop
    for (int y = 0; y <= stepYBound_; y++) {
        FLOAT dx = (i - stepXBound_) * parameters_.meshsize->getDx(i, j, k);
        FLOAT dy = (j - y) * parameters_.meshsize->getDy(i, j, k);
        FLOAT stepDis = sqrt((dx * dx) + (dy * dy));

        distToWall = std::abs(std::min(distToWall, stepDis));
    }
}

void DistanceStencil::apply(FlowField& flowField, int i, int j, int k) {
    FLOAT& distToWall = flowField.getDistance().getScalar(i, j, k);

    if ((flowField.getFlags().getValue(i, j, k) & OBSTACLE_SELF) != 0) { // If it is not a fluid cell, the dist is zero.
        distToWall = 0;
    } else { // If it is a fluid cell, calculate the distance
        // check left and right wall to calculate the closest distance in the x-direction
        const FLOAT distX = calculateDistToNearestWallInGivenDir_(leftWall_,
                                                                  rightWall_,
                                                                  i, parameters_.meshsize->getDx(i, j, k), cellsX_);

        // check bottom and top wall to calculate the closest distance in the y-direction
        const FLOAT distY = calculateDistToNearestWallInGivenDir_(bottomWall_,
                                                                  topWall_,
                                                                  j, parameters_.meshsize->getDy(i, j, k), cellsY_);

        if (parameters_.geometry.dim == 2) { // 2D
            // Find the distance of cell to the nearest wall
            distToWall = std::abs(std::min(distX, distY));
        } else { // 3D
            // check front and back wall to calculate the closest distance in the z-direction
            const FLOAT distZ = calculateDistToNearestWallInGivenDir_(frontWall_,
                                                                      backWall_,
                                                                      k, parameters_.meshsize->getDz(i, j, k), cellsZ_);

            // Find the distance of cell to the nearest wall
            distToWall = std::abs(std::min(std::min(distX, distY), distZ));
        }

        // Calculate steps if there are any (only for backward channel scenario)
        calculateSteps_(distToWall, i, j, k);
    }
}

void DistanceStencil::apply(FlowField& flowField, int i, int j) {
    apply(flowField, i, j, 0);
}

} // namespace NSEOF::Stencils
