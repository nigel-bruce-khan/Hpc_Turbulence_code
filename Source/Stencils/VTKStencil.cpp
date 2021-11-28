#include "VTKStencil.hpp"
#include "StencilFunctions.hpp"
#include "Definitions.hpp"

namespace NSEOF::Stencils {

    CellIndex::CellIndex(int i, int j, int k) : i(i), j(j), k(k) {}

    VTKStencil::VTKStencil(const Parameters &parameters, int Nx, int Ny, int Nz)
            : FieldStencil<FlowField>(parameters)
            , pressure_(ScalarField(Nx, Ny, parameters.geometry.dim == 3 ? Nz : 1))
            , velocity_(VectorField(Nx, Ny, parameters.geometry.dim == 3 ? Nz : 1)) {}

    VTKStencil::~VTKStencil() {
        cellIndices_.clear();
    }

    // TODO: Include obstacles?? Erfan
    void VTKStencil::apply(FlowField& flowField, int i, int j, int k) {
        // Store the cell indices
        cellIndices_.emplace_back(i, j, k);

        // Get data structures stored
        FLOAT& pressure = pressure_.getScalar(i, j, k);
        FLOAT* velocity = velocity_.getVector(i, j, k);

        // Get the pressure and velocity
        if (parameters_.geometry.dim == 2) { // 2D
            flowField.getPressureAndVelocity(pressure, velocity, i, j);
        } else if (parameters_.geometry.dim == 3) { // 3D
            flowField.getPressureAndVelocity(pressure, velocity, i, j, k);
        } else {
            std::cerr << "This app only supports 2D and 3D geometry" << std::endl;
            exit(1);
        }
    }

    void VTKStencil::apply(FlowField& flowField, int i, int j) {
        apply(flowField, i, j, 0);
    }

    void VTKStencil::writePositions_(FILE* filePtr) {
        const int sizeX = parameters_.geometry.sizeX;
        const int sizeY = parameters_.geometry.sizeY;
        const int sizeZ = parameters_.geometry.dim == 3 ? parameters_.geometry.sizeZ : 0;

        fprintf(filePtr, "DATASET %s\n", parameters_.vtk.datasetName.c_str());
        fprintf(filePtr, "DIMENSIONS %d %d %d\n", (sizeX + 1), (sizeY + 1), (sizeZ + 1));
        fprintf(filePtr, "POINTS %d float\n", (sizeX + 1) * (sizeY + 1) * (sizeZ + 1));

        auto cellIndexIterator = cellIndices_.begin();
        CellIndex* cellIndex;

        FLOAT posX, posY, posZ = parameters_.parallel.firstCorner[2];

        for (int k = 0; k <= sizeZ; k++) {
            posY = parameters_.parallel.firstCorner[1];

            for (int j = 0; j <= sizeY; j++) {
                posX = parameters_.parallel.firstCorner[0];

                for (int i = 0; i <= sizeX; i++) {
                    cellIndex = &(*cellIndexIterator);

                    fprintf(filePtr, "%f %f %f\n", posX, posY, posZ);
                    posX += parameters_.meshsize->getDx(cellIndex->i, cellIndex->j, cellIndex->k);

                    // Do not iterate on the bounds to take the other node of the cell!
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

        for (auto& cellIndex : cellIndices_) {
            fprintf(filePtr, "%f\n", pressure_.getScalar(cellIndex.i, cellIndex.j, cellIndex.k));
        }

        fprintf(filePtr, "\n");
    }

    void VTKStencil::writeVelocities_(FILE* filePtr) {
        fprintf(filePtr, "VECTORS velocity float\n");

        FLOAT* velocity;
        for (auto& cellIndex : cellIndices_) {
            velocity = velocity_.getVector(cellIndex.i, cellIndex.j, cellIndex.k);
            fprintf(filePtr, "%f %f %f\n", velocity[0], velocity[1], velocity[2]);
        }

        fprintf(filePtr, "\n");
    }

    void VTKStencil::write(int timeStep) {
        // Decide on the filename
        long time = timeStep * parameters_.vtk.interval * 1e6;
        std::string filename = parameters_.vtk.outDir + "/" + parameters_.vtk.prefix + "_" +
                               std::to_string(time) + ".vtk";

        // Open the file stream
        FILE* filePtr = fopen(filename.c_str(), "w");

        // Write header to the file
        fprintf(filePtr, "%s\n", parameters_.vtk.vtkFileHeader.c_str());

        // Write data to the file
        writePositions_(filePtr);
        writePressures_(filePtr);
        writeVelocities_(filePtr);

        // Close the file stream
        fclose(filePtr);
    }
} // namespace NSEOF::Stencils
