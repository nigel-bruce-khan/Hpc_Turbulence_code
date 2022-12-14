#include "EigenSolver.hpp"

#define ROW_MAJOR_IDX(i, j, k, sizeX, sizeY) ((i) + ((j) * (sizeX)) + ((k) * (sizeX) * (sizeY)))
#define COLUMN_MAJOR_IND(i, j, k, sizeY, sizeZ) ((k) + ((j) * (sizeZ)) + ((i) * (sizeY) * (sizeZ)))

namespace NSEOF::Solvers {

    Coefficients::Coefficients(FLOAT dxLeft, FLOAT dxRight, FLOAT dyBottom, FLOAT dyTop, FLOAT dzFront, FLOAT dzBack)
        : dxLeft(dxLeft), dxRight(dxRight), dyBottom(dyBottom), dyTop(dyTop), dzFront(dzFront), dzBack(dzBack) {}

    void EigenSolver::fillCoefficientsVector_() {
        coefficientsVector_.clear();
        coefficientsVector_.reserve(dim_);

        for (int k = 0; k < cellsZ_; k++) {
            for (int j = 0; j < cellsY_; j++) {
                for (int i = 0; i < cellsX_; i++) {
                    const FLOAT dx = parameters_.meshsize->getDx(i, j, k);
                    const FLOAT dy = parameters_.meshsize->getDy(i, j, k);
                    const FLOAT dz = parameters_.meshsize->getDz(i, j, k);

                    const FLOAT dxLeft   = 0.5 * (dx + parameters_.meshsize->getDx(i - 1, j, k));
                    const FLOAT dxRight  = 0.5 * (dx + parameters_.meshsize->getDx(i + 1, j, k));
                    const FLOAT dyBottom = 0.5 * (dy + parameters_.meshsize->getDy(i, j - 1, k));
                    const FLOAT dyTop    = 0.5 * (dy + parameters_.meshsize->getDy(i, j + 1, k));
                    const FLOAT dzFront  = 0.5 * (dz + parameters_.meshsize->getDz(i, j, k - 1));
                    const FLOAT dzBack   = 0.5 * (dz + parameters_.meshsize->getDz(i, j, k + 1));

                    coefficientsVector_.emplace_back(dxLeft, dxRight, dyBottom, dyTop, dzFront, dzBack);
                }
            }
        }
    }

    void EigenSolver::computeStencilRowForFluidCell_(VectorXd& stencilRow, const int i, const int j, const int k = 0) const {
        const Coefficients coefficients = coefficientsVector_[ROW_MAJOR_IDX(i, j, k, cellsX_, cellsY_)];
        const int centerIdx = (parameters_.geometry.dim == 2) ? cellsX_ : cellsX_ * cellsY_;

        /* Bottom */ stencilRow(centerIdx - cellsX_) = 2.0 / (coefficients.dyBottom * (coefficients.dyBottom + coefficients.dyTop));
        /* Left   */ stencilRow(centerIdx - 1      ) = 2.0 / (coefficients.dxLeft * (coefficients.dxLeft + coefficients.dxRight));
        /* Center */ stencilRow(centerIdx          ) = 2.0 / (coefficients.dxLeft * coefficients.dxRight) + 2.0 / (coefficients.dyBottom * coefficients.dyTop);
        /* Right  */ stencilRow(centerIdx + 1      ) = 2.0 / (coefficients.dxRight * (coefficients.dxLeft + coefficients.dxRight));
        /* Top    */ stencilRow(centerIdx + cellsX_) = 2.0 / (coefficients.dyTop * (coefficients.dyBottom + coefficients.dyTop));

        if (parameters_.geometry.dim == 3) { // 3D
            /* Front  */ stencilRow(centerIdx - cellsX_ * cellsY_) =  2.0 / (coefficients.dzFront * (coefficients.dzBack + coefficients.dzFront));
            /* Center */ stencilRow(centerIdx                    ) += 2.0 / (coefficients.dzFront * coefficients.dzBack);
            /* Back   */ stencilRow(centerIdx + cellsX_ * cellsY_) =  2.0 / (coefficients.dzBack * (coefficients.dzBack + coefficients.dzFront));
        }

        /* Center */ stencilRow(centerIdx) *= -1.0;
    }

    void EigenSolver::computeStencilRowForObstacleCellWithFluidAround_(const int obstacle, VectorXd& stencilRow) const {
        const auto bottomFluid = (FLOAT) ((obstacle & OBSTACLE_BOTTOM) == 0);
        const auto leftFluid   = (FLOAT) ((obstacle & OBSTACLE_LEFT)   == 0);
        const auto rightFluid  = (FLOAT) ((obstacle & OBSTACLE_RIGHT)  == 0);
        const auto topFluid    = (FLOAT) ((obstacle & OBSTACLE_TOP)    == 0);

        const int centerIdx = (parameters_.geometry.dim == 2) ? cellsX_ : cellsX_ * cellsY_;

        /* Bottom */ stencilRow(centerIdx - cellsX_) = bottomFluid;
        /* Left   */ stencilRow(centerIdx - 1      ) = leftFluid;
        /* Center */ stencilRow(centerIdx          ) = bottomFluid + leftFluid + rightFluid + topFluid;
        /* Right  */ stencilRow(centerIdx + 1      ) = rightFluid;
        /* Top    */ stencilRow(centerIdx + cellsX_) = topFluid;

        if (parameters_.geometry.dim == 3) { // 3D
            const auto frontFluid = (FLOAT) ((obstacle & OBSTACLE_FRONT) == 0);
            const auto backFluid  = (FLOAT) ((obstacle & OBSTACLE_BACK)  == 0);

            /* Front  */ stencilRow(centerIdx - cellsX_ * cellsY_) =  frontFluid;
            /* Center */ stencilRow(centerIdx                    ) += frontFluid + backFluid;
            /* Back   */ stencilRow(centerIdx + cellsX_ * cellsY_) =  backFluid;
        }

        /* Center */ stencilRow(centerIdx) *= -1.0;
    }

    void EigenSolver::computeStencilRowForObstacleCell_(VectorXd& stencilRow) const {
        const int centerIdx = (parameters_.geometry.dim == 2) ? cellsX_ : cellsX_ * cellsY_;

        /* Bottom */ stencilRow(centerIdx - cellsX_) = 0.0;
        /* Left   */ stencilRow(centerIdx - 1      ) = 0.0;
        /* Center */ stencilRow(centerIdx          ) = 1.0;
        /* Right  */ stencilRow(centerIdx + 1      ) = 0.0;
        /* Top    */ stencilRow(centerIdx + cellsX_) = 0.0;

        if (parameters_.geometry.dim == 3) { // 3D
            /* Front */ stencilRow(centerIdx - cellsX_ * cellsY_) = 0.0;
            /* Back  */ stencilRow(centerIdx + cellsX_ * cellsY_) = 0.0;
        }
    }

    /**
     * Creates the matrix using the values on the fluid region for both 2D and 3D grids
     *
     * @param sumObstacles summation of all obstacles
     * @param k the layer (face) index
     */
    void EigenSolver::computeMatrixOnFluidRegion_(const int sumObstacles, const int k = 0) {
        int row = cellsX_ + 1;
        int column = 1;

        if (parameters_.geometry.dim == 3) { // 3D
            row += k * cellsX_ * cellsY_;
            column += (k - 1) * (cellsX_ * cellsY_) + cellsX_;
        }

        for (int j = 1; j < cellsY_ - 1; j++, row += 2, column += 2) {
            for (int i = 1; i < cellsX_ - 1; i++, row++, column++) {
                int obstacle = flowField_.getFlags().getValue(i + 1, j + 1);
                int stencilRowLength = cellsX_ * 2 + 1;

                if (parameters_.geometry.dim == 3) { // 3D
                    obstacle = flowField_.getFlags().getValue(i + 1, j + 1, k + 1);
                    stencilRowLength = cellsX_ * cellsY_ * 2 + 1;
                }

                VectorXd stencilRow = VectorXd::Zero(stencilRowLength);

                if ((obstacle & OBSTACLE_SELF) == 0) { // It is a fluid cell
                    computeStencilRowForFluidCell_(stencilRow, i, j, k);
                } else if (obstacle != sumObstacles) { // Not a fluid cell, but fluid is somewhere around
                    computeStencilRowForObstacleCellWithFluidAround_(obstacle, stencilRow);
                } else { // The cell is an obstacle cell surrounded by more obstacle cells
                    computeStencilRowForObstacleCell_(stencilRow);
                }

                matA_.block(row, column, 1, stencilRowLength) = stencilRow.transpose();
            }
        }
    }

    void EigenSolver::computeMatrixOnBoundariesLeftAndRight_(const unsigned int startIdx = 0) {
        VectorXd leftWallVector(2), rightWallVector(2);

        leftWallVector << (parameters_.walls.typeLeft == DIRICHLET ? 1.0 : 0.5),
                          (parameters_.walls.typeLeft == DIRICHLET ? -1.0 : 0.5);
        rightWallVector << (parameters_.walls.typeRight == DIRICHLET ? -1.0 : 0.5),
                           (parameters_.walls.typeRight == DIRICHLET ? 1.0 : 0.5);

        for (int j = 1; j < cellsY_ - 1; j++) {
            matA_.block(startIdx + j * cellsX_, startIdx + j * cellsX_, 1, 2) = leftWallVector.transpose();
            matA_.block(startIdx + (j + 1) * cellsX_ - 1, startIdx + (j + 1) * cellsX_ - 2, 1, 2) = rightWallVector.transpose();
        }
    }

    void EigenSolver::computeMatrixOnBoundaryBottomOrTop_(BoundaryType boundaryType,
                                                          const unsigned int startIdx, const int direction) {
        const MatrixXd identityMatrix = MatrixXd::Identity(cellsX_ - 2, cellsX_ - 2);

        const MatrixXd diagMat = (boundaryType == DIRICHLET ? 1.0 : 0.5) * identityMatrix;
        const MatrixXd offDiagMat = (boundaryType == DIRICHLET ? -1.0 : 0.5) * identityMatrix;

        matA_.block(startIdx, startIdx, cellsX_ - 2, cellsX_ - 2) = diagMat;
        matA_.block(startIdx, startIdx + (direction * cellsX_), cellsX_ - 2, cellsX_ - 2) = offDiagMat;
    }

    void EigenSolver::computeMatrixOnBoundaryFrontOrBack_(BoundaryType boundaryType,
                                                          const unsigned int startIdx, const int direction) {
        const MatrixXd identityMatrix = MatrixXd::Identity(cellsX_ - 2, cellsX_ - 2);

        const MatrixXd diagMat = (boundaryType == DIRICHLET ? 1.0 : 0.5) * identityMatrix;
        const MatrixXd offDiagMat = (boundaryType == DIRICHLET ? -1.0 : 0.5) * identityMatrix;

        for (int i = 1; i < cellsY_ - 1; i++) {
            const unsigned int currentStartIdx = startIdx + i * cellsX_;
            const unsigned int currentOffDiagColStartIdx = currentStartIdx + (direction * cellsX_ * cellsY_);

            matA_.block(currentStartIdx, currentStartIdx, cellsX_ - 2, cellsX_ - 2) = diagMat;
            matA_.block(currentStartIdx, currentOffDiagColStartIdx, cellsX_ - 2, cellsX_ - 2) = offDiagMat;
        }
    }

    void EigenSolver::computeMatrix_() {
        const int sumObstacles = pow(2, parameters_.geometry.dim * 2 + 1) - 1;

        // If 3D, use the actual bounds, otherwise, use [0, 1) and iterate on k only once!
        const int kLowerBound = (parameters_.geometry.dim == 3) ? 1 : 0;
        const int kUpperBound = (parameters_.geometry.dim == 3) ? (cellsZ_ - 1) : 1;

        for (int k = kLowerBound; k < kUpperBound; k++) {
            /**
             * Fill the matrix on white region
             */

            computeMatrixOnFluidRegion_(sumObstacles, k);

            /**
             * Fill the matrix on boundary conditions (Left, right, bottom and top walls)
             */

            const int startIdx = k * cellsX_ * cellsY_;

            // Left and right walls
            computeMatrixOnBoundariesLeftAndRight_(startIdx);

            // Bottom and top walls
            computeMatrixOnBoundaryBottomOrTop_(parameters_.walls.typeBottom, startIdx + 1, 1);
            computeMatrixOnBoundaryBottomOrTop_(parameters_.walls.typeTop, startIdx + (cellsY_ - 1) * cellsX_ + 1, -1);
        }

        if (parameters_.geometry.dim == 3) { // 3D
            /**
             * Fill the matrix on boundary conditions (Front and back walls)
             */

            computeMatrixOnBoundaryFrontOrBack_(parameters_.walls.typeFront, 1, 1);
            computeMatrixOnBoundaryFrontOrBack_(parameters_.walls.typeBack, (cellsZ_ - 1) * (cellsX_ * cellsY_) + 1,-1);
        }

        /**
         * Convert the matrix to a sparse matrix
         */

        sparseMatA_ = matA_.sparseView();
    }

    void EigenSolver::initMatrix_() {
#ifdef OMP
        initParallel();

        #pragma omp parallel default(none)
        #pragma omp master
        setNbThreads(omp_get_num_threads());
#endif

        matA_ = MatrixXd::Zero(dim_, dim_);
        rhs_ = VectorXd::Zero(dim_);
        x_ = VectorXd::Zero(dim_);

        fillCoefficientsVector_();
        computeMatrix_();

        currentNumIterations_ = SOLVER_ITERATIONS_MAX_NUM;

        solver_.setMaxIterations(currentNumIterations_);
        solver_.compute(sparseMatA_);
    }

    EigenSolver::EigenSolver(FlowField& flowField, const Parameters& parameters)
        : LinearSolver(flowField, parameters)
        , cellsX_(parameters.parallel.localSize[0] + 2)
        , cellsY_(parameters.parallel.localSize[1] + 2)
        , cellsZ_(parameters.geometry.dim == 3 ? (parameters.parallel.localSize[2] + 2) : 1)
        , dim_(cellsX_ * cellsY_ * cellsZ_) {
        initMatrix_();
    }

    EigenSolver::~EigenSolver() {
        coefficientsVector_.clear();

        matA_.resize(0, 0);
        rhs_.resize(0);
        x_.resize(0);
    }

    FLOAT EigenSolver::getScalarRHS_(const int obstacle, int i, int j, int k = 0) {
        if ((obstacle & OBSTACLE_SELF) == 0) { // Fluid cell
            return flowField_.getRHS().getScalar(i, j, k);
        }

        return 0.0; // Obstacle cell
    }

    void EigenSolver::computeRHS2D_() {
        #pragma omp parallel for collapse(2) default(none) shared(cellsX_, cellsY_, flowField_, rhs_)
        for (int j = 1; j < cellsY_ - 1; j++) {
            for (int i = 1; i < cellsX_ - 1; i++) {
                const int obstacle = flowField_.getFlags().getValue(i + 1, j + 1);
                rhs_(ROW_MAJOR_IDX(i, j, 0, cellsX_, cellsY_)) = getScalarRHS_(obstacle, i + 1, j + 1);
            }
        }
    }

    void EigenSolver::computeRHS3D_() {
        #pragma omp parallel for collapse(3) default(none) shared(cellsX_, cellsY_, cellsZ_, flowField_, rhs_)
        for (int k = 1; k < cellsZ_ - 1; k++) {
            for (int j = 1; j < cellsY_ - 1; j++) {
                for (int i = 1; i < cellsX_ - 1; i++) {
                    const int obstacle = flowField_.getFlags().getValue(i + 1, j + 1, k + 1);
                    rhs_(ROW_MAJOR_IDX(i, j, k, cellsX_, cellsY_)) = getScalarRHS_(obstacle, i + 1, j + 1, k + 1);
                }
            }
        }
    }

    void EigenSolver::setPressure2D_() {
        #pragma omp parallel for collapse(2) default(none) shared(cellsX_, cellsY_, flowField_, x_)
        for (int j = 0; j < cellsY_; j++) {
            for (int i = 0; i < cellsX_; i++) {
                flowField_.getPressure().getScalar(i + 1, j + 1) = x_(ROW_MAJOR_IDX(i, j, 0, cellsX_, cellsY_));
            }
        }
    }

    void EigenSolver::setPressure3D_() {
        #pragma omp parallel for collapse(3) default(none) shared(cellsX_, cellsY_, cellsZ_, flowField_, x_)
        for (int k = 0; k < cellsZ_; k++) {
            for (int j = 0; j < cellsY_; j++) {
                for (int i = 0; i < cellsX_; i++) {
                    flowField_.getPressure().getScalar(i + 1, j + 1, k + 1) = x_(ROW_MAJOR_IDX(i, j, k, cellsX_, cellsY_));
                }
            }
        }
    }

    void EigenSolver::updateNumIterationsBasedOnError_() {
        const int stepDirection = (solver_.error() < SOLVER_LOWER_ERROR_THRESHOLD) ? -1 : 1;
        const int stepValue = stepDirection * (int) (currentNumIterations_ * SOLVER_ITERATIONS_STEP);

        currentNumIterations_ += stepValue;
        solver_.setMaxIterations(currentNumIterations_);
    }

    void EigenSolver::solve() {
        // Compute RHS
        if (parameters_.geometry.dim == 2) { // 2D
            computeRHS2D_();
        } else { // 3D
            computeRHS3D_();
        }

        x_ = solver_.solve(rhs_);

        std::cout << "# of iterations: " << solver_.iterations() << std::endl;
        std::cout << "estimated error: " << solver_.error()      << std::endl;

        if (parameters_.geometry.dim == 2) { // 2D
            setPressure2D_();
        } else { // 3D
            setPressure3D_();
        }

        updateNumIterationsBasedOnError_();
    }

    inline void EigenSolver::reInitMatrix() {
        initMatrix_();
    }
} // namespace Solvers::NSEOF
