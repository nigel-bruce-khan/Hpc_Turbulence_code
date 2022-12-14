#include "PetscParallelConfiguration.hpp"

namespace NSEOF {
namespace ParallelManagers {

PetscParallelConfiguration::PetscParallelConfiguration(Parameters& parameters)
    : parameters_(parameters) {

    // Obtain the rank of the current processor
    int rank, nproc;
    MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
    MPI_Comm_size(PETSC_COMM_WORLD, &nproc);

    parameters.parallel.rank = rank;

    // Obtain the position of this subdomain, and locate its neighbors.
    createIndices();
    locateNeighbors();
    computeSizes();

    int nprocFromFile = parameters_.parallel.numProcessors[0] *
                        parameters_.parallel.numProcessors[1];

    if (parameters.geometry.dim == 3) {
        nprocFromFile *= parameters_.parallel.numProcessors[2];
    }

    if (nproc != nprocFromFile) {
#if BUILD_WITH_EIGEN
        // TODO (Parallelization of Eigen Solver): Implement Eigen Solver to also work with multiple processors
        HANDLE_ERROR(1, "The Eigen Solver currently supports only sequential solving, not parallel");
#else
        HANDLE_ERROR(1, "The number of processors specified in the configuration file doesn't match the communicator");
#endif
    }
}

PetscParallelConfiguration::~PetscParallelConfiguration() {
    freeSizes();
}

void PetscParallelConfiguration::locateNeighbors() {
    int i = parameters_.parallel.indices[0];
    int j = parameters_.parallel.indices[1];
    int k = parameters_.parallel.indices[2];

    if (parameters_.geometry.dim == 2) {
        parameters_.parallel.leftNb   = computeRankFromIndices(i - 1, j, 0);
        parameters_.parallel.rightNb  = computeRankFromIndices(i + 1, j, 0);
        parameters_.parallel.bottomNb = computeRankFromIndices(i, j - 1, 0);
        parameters_.parallel.topNb    = computeRankFromIndices(i, j + 1, 0);

        parameters_.parallel.rightTopNb = computeRankFromIndices(i + 1, j + 1, 0);
        parameters_.parallel.leftBottomNb = computeRankFromIndices(i - 1, j - 1, 0);
        parameters_.parallel.rightBottomNb = computeRankFromIndices(i + 1, j - 1, 0);
        parameters_.parallel.leftTopNb = computeRankFromIndices(i - 1, j + 1, 0);

        // The following two are not used in this case
        parameters_.parallel.frontNb = MPI_PROC_NULL;
        parameters_.parallel.backNb  = MPI_PROC_NULL;
    } else {
        parameters_.parallel.leftNb   = computeRankFromIndices(i - 1, j, k);
        parameters_.parallel.rightNb  = computeRankFromIndices(i + 1, j, k);
        parameters_.parallel.bottomNb = computeRankFromIndices(i, j - 1, k);
        parameters_.parallel.topNb    = computeRankFromIndices(i, j + 1, k);
        parameters_.parallel.frontNb  = computeRankFromIndices(i, j, k - 1);
        parameters_.parallel.backNb   = computeRankFromIndices(i, j, k + 1);

        parameters_.parallel.rightTopNb = computeRankFromIndices(i + 1, j + 1, k);
        parameters_.parallel.leftBottomNb = computeRankFromIndices(i - 1, j - 1, k);
        parameters_.parallel.rightBottomNb = computeRankFromIndices(i + 1, j - 1, k);
        parameters_.parallel.leftTopNb = computeRankFromIndices(i - 1, j + 1, k);
    }

    // If periodic boundaries declared, let the process itself deal with it, without communication.
    int rank;
    MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
}

void PetscParallelConfiguration::createIndices() {
    int& rank = parameters_.parallel.rank;
    parameters_.parallel.indices[0] = rank % parameters_.parallel.numProcessors[0];
    parameters_.parallel.indices[1] = (rank / parameters_.parallel.numProcessors[0]) % parameters_.parallel.numProcessors[1];
    parameters_.parallel.indices[2] = rank / (parameters_.parallel.numProcessors[0] * parameters_.parallel.numProcessors[1]);
}

int PetscParallelConfiguration::computeRankFromIndices(int i, int j, int k) const {
    if (i < 0 || i >= parameters_.parallel.numProcessors[0] ||
        j < 0 || j >= parameters_.parallel.numProcessors[1] ||
        k < 0 || k >= parameters_.parallel.numProcessors[2]) {
        return MPI_PROC_NULL;
    }
    int nrank = i + j * parameters_.parallel.numProcessors[0];
    if (parameters_.geometry.dim == 3) {
        nrank += k * parameters_.parallel.numProcessors[0] * parameters_.parallel.numProcessors[1];
    }
    return nrank;
}

void PetscParallelConfiguration::computeSizes() {
    int dim = parameters_.geometry.dim;

    for (int i = 0; i < dim; i++) {
        parameters_.parallel.sizes[i] = new
#ifdef BUILD_WITH_PETSC
        PetscInt
#else
        int
#endif   
        [parameters_.parallel.numProcessors[i]];
    }

    int geometrySizes[3];
    geometrySizes[0] = parameters_.geometry.sizeX;
    geometrySizes[1] = parameters_.geometry.sizeY;
    geometrySizes[2] = parameters_.geometry.sizeZ;

    for (int i = 0; i < dim; i++){
        for (int j = 0; j < parameters_.parallel.numProcessors[i]; j++) {
            parameters_.parallel.sizes[i][j] = geometrySizes[i] / parameters_.parallel.numProcessors[i];
            if (j < geometrySizes[i] % parameters_.parallel.numProcessors[i]) {
                parameters_.parallel.sizes[i][j]++;
            }
        }
    }

    // Locate the position of the first element of the subdomain. Useful for plotting later on.
    for (int i = 0; i < dim; i++) {
        parameters_.parallel.firstCorner[i] = 0;
        for (int j = 0; j < parameters_.parallel.indices[i]; j++) {
            parameters_.parallel.firstCorner[i] += parameters_.parallel.sizes[i][j];
        }
    }

    if (dim == 2) {
        parameters_.parallel.firstCorner[2] = 0;
    }

    // Select the local sizes from the already computed sizes
    for (int i = 0; i < dim; i++) {
        parameters_.parallel.localSize[i] = parameters_.parallel.sizes[i][parameters_.parallel.indices[i]];
    }

    // If the domain lies on an edge, add one to that direction, for the artificial external pressures in the PETSc solver.
    for (int i = 0; i < dim; i++) {
        parameters_.parallel.sizes[i][0] ++;
        parameters_.parallel.sizes[i][parameters_.parallel.numProcessors[i] - 1] ++;
    }
}

void PetscParallelConfiguration::freeSizes() {
    int dim = parameters_.geometry.dim;

    for (int i = 0; i < dim; i++){
        delete[] parameters_.parallel.sizes[i];
    }
}

} // namespace ParallelManagers
} // namespace NSEOF
