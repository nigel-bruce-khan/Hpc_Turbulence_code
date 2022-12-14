#include "VTKStencil.hpp"
#include "StencilFunctions.hpp"
#include "Definitions.hpp"

namespace NSEOF::Stencils {

    CellIndex::CellIndex(int i, int j, int k) : i(i), j(j), k(k) {}

    static int getNumCellsExpected(const Parameters &parameters) {
        int numCellsExpected = parameters.geometry.sizeX * parameters.geometry.sizeY;

        if (parameters.geometry.dim == 3) { // 3D
            numCellsExpected *= parameters.geometry.sizeZ;
        }

        return numCellsExpected;
    }

    VTKStencil::VTKStencil(const Parameters &parameters, const int cellsX, const int cellsY, const int cellsZ)
        : FieldStencil<FlowField>(parameters)
        , pressure_(ScalarField(cellsX, cellsY, parameters.geometry.dim == 3 ? cellsZ : 1))
        , velocity_(VectorField(cellsX, cellsY, parameters.geometry.dim == 3 ? cellsZ : 1)) {
        // Pre-allocate the space needed for storing the cell indices!
        cellIndices_.reserve(getNumCellsExpected(parameters));
    }

    void VTKStencil::clearValues(bool valuesReserved = false) {
        cellIndices_.clear();

        // Re-allocate the space needed for storing the cell indices!
        if (valuesReserved) {
            cellIndices_.reserve(getNumCellsExpected(parameters_));
        }
    }

    VTKStencil::~VTKStencil() {
        clearValues();
    }

    void VTKStencil::apply(FlowField& flowField, int i, int j, int k) {
        // Store the cell indices
        cellIndices_.emplace_back(i, j, k);

        // Get the data structures stored
        FLOAT& cellPressure = pressure_.getScalar(i, j, k);
        FLOAT* cellVelocity = velocity_.getVector(i, j, k);

        // Make sure that it is a fluid cell, and if it is not, stop the computation and store 0s instead!
        if ((flowField.getFlags().getValue(i, j, k) & OBSTACLE_SELF) != 0) {
            cellPressure = 0.0;
            for (int dim = 0; dim < parameters_.geometry.dim; dim++) cellVelocity[dim] = 0.0;

            return;
        }

        // Get the pressure and velocity
        if (parameters_.geometry.dim == 2) { // 2D
            flowField.getPressureAndVelocity(cellPressure, cellVelocity, i, j);
        } else { // 3D
            flowField.getPressureAndVelocity(cellPressure, cellVelocity, i, j, k);
        }
    }

    void VTKStencil::apply(FlowField& flowField, int i, int j) {
        apply(flowField, i, j, 0);
    }

    void VTKStencil::writePositions_(FILE* filePtr) {
        const int sizeX = parameters_.parallel.localSize[0];
        const int sizeY = parameters_.parallel.localSize[1];
        const int sizeZ = parameters_.geometry.dim == 3 ? parameters_.parallel.localSize[2] : 0;

        fprintf(filePtr, "DATASET %s\n", parameters_.vtk.datasetName.c_str());
        fprintf(filePtr, "DIMENSIONS %d %d %d\n", (sizeX + 1), (sizeY + 1), (sizeZ + 1));
        fprintf(filePtr, "POINTS %d float\n", (sizeX + 1) * (sizeY + 1) * (sizeZ + 1));

        auto cellIndexIterator = cellIndices_.begin();
        CellIndex* cellIndex;

        FLOAT posX, posY, posZ = parameters_.parallel.firstCorner[2] * parameters_.meshsize->getDz(0, 0, 0);

        for (int k = 0; k <= sizeZ; k++) {
            posY = parameters_.parallel.firstCorner[1] * parameters_.meshsize->getDy(0, 0, 0);

            for (int j = 0; j <= sizeY; j++) {
                posX = parameters_.parallel.firstCorner[0] * parameters_.meshsize->getDx(0, 0, 0);

                for (int i = 0; i <= sizeX; i++) {
                    cellIndex = &(*cellIndexIterator);

                    fprintf(filePtr, "%f %f %f\n", posX, posY, posZ);
                    posX += parameters_.meshsize->getDx(cellIndex->i, cellIndex->j, cellIndex->k);

                    // Do not iterate on the bounds to take the other node of the cell!
                    // The z-axis is only taken into account in a 3D simulation!
                    if (i != sizeX && j != sizeY && (parameters_.geometry.dim == 2 || k != sizeZ)) {
                        cellIndexIterator++;
                    }
                }

                posY += parameters_.meshsize->getDy(cellIndex->i, cellIndex->j, cellIndex->k);
            }

            posZ += parameters_.meshsize->getDz(cellIndex->i, cellIndex->j, cellIndex->k);
        }

        fprintf(filePtr, "\n");
    }

    void VTKStencil::writePressures_(FILE* filePtr) {
        fprintf(filePtr, "CELL_DATA %zu\n", cellIndices_.size());
        fprintf(filePtr, "SCALARS pressure float 1\n");
        fprintf(filePtr, "LOOKUP_TABLE default\n");

        FLOAT cellPressure;
        for (auto& cellIndex : cellIndices_) {
            cellPressure = pressure_.getScalar(cellIndex.i, cellIndex.j, cellIndex.k);
            fprintf(filePtr, "%f\n", cellPressure);
        }

        fprintf(filePtr, "\n");
    }

    void VTKStencil::writeVelocities_(FILE* filePtr) {
        fprintf(filePtr, "VECTORS velocity float\n");

        FLOAT* cellVelocity;
        for (auto& cellIndex : cellIndices_) {
            cellVelocity = velocity_.getVector(cellIndex.i, cellIndex.j, cellIndex.k);
            fprintf(filePtr, "%f %f %f\n", cellVelocity[0], cellVelocity[1], cellVelocity[2]);
        }

        fprintf(filePtr, "\n");
    }

    void VTKStencil::writeValues_(FILE* filePtr) {
        writePositions_(filePtr);
        writePressures_(filePtr);
        writeVelocities_(filePtr);
    }

    void VTKStencil::write(int timestep) {
        // Decide on the filename
        long time = timestep * parameters_.vtk.interval * 1e6;
        std::string filename = parameters_.vtk.outDir + "/" + parameters_.vtk.prefix + "_" +
                               std::to_string(parameters_.parallel.rank) + "_" +
                               std::to_string(time) + ".vtk";

        // Open the file stream
        FILE* filePtr = fopen(filename.c_str(), "w");

        // Write header to the file
        fprintf(filePtr, "%s\n", parameters_.vtk.vtkFileHeader.c_str());

        // Write data to the file
        writeValues_(filePtr);

        // Close the file stream
        fclose(filePtr);
    }

    const std::vector<CellIndex>& VTKStencil::getCellIndices_() const {
        return cellIndices_;
    }

} // namespace NSEOF::Stencils
