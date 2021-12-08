#include "FGHStencil.hpp"
#include "StencilFunctions.hpp"
#include "Definitions.hpp"

namespace NSEOF {
namespace Stencils {

FGHStencil::FGHStencil(const Parameters& parameters)
    : FieldStencil<FlowField>(parameters) {}

void FGHStencil::apply(FlowField& flowField, int i, int j) {
    // Load local velocities into the center layer of the local array
    loadLocalVelocity2D(flowField, localVelocity_, i, j);
    loadLocalMeshsize2D(parameters_, localMeshsize_, i, j);
    

    FLOAT* const values = flowField.getFGH().getVector(i, j);
 
    if(parameters_.turbulence.on == 1){
	loadLocalViscosity2D(flowField, localViscosity_, i,j);
        values[0] = computeF2DT(localVelocity_, localMeshsize_, localViscosity_, parameters_, parameters_.timestep.dt);
        values[1] = computeG2DT(localVelocity_, localMeshsize_, localViscosity_, parameters_, parameters_.timestep.dt);

    }else{
	// Now the localVelocity array should contain lexicographically ordered elements around the given index
	values[0] = computeF2D(localVelocity_, localMeshsize_, parameters_, parameters_.timestep.dt);
        values[1] = computeG2D(localVelocity_, localMeshsize_, parameters_, parameters_.timestep.dt);
    }

}

void FGHStencil::apply(FlowField& flowField, int i, int j, int k) {
    // The same as in 2D, with slight modifications.

    const int obstacle = flowField.getFlags().getValue(i, j, k);
    FLOAT* const values = flowField.getFGH().getVector(i, j, k);

    if ((obstacle & OBSTACLE_SELF) == 0) { // If the cell is fluid
        loadLocalVelocity3D(flowField, localVelocity_, i, j, k);
        loadLocalMeshsize3D(parameters_, localMeshsize_, i, j, k);
	
    	if(parameters_.turbulence.on == 1){
		loadLocalViscosity2D(flowField, localViscosity_, i,j);
		if ((obstacle & OBSTACLE_RIGHT) == 0) { // If the right cell is fluid
                        values[0] = computeF3DT(localVelocity_, localMeshsize_, localViscosity_, parameters_, parameters_.timestep.dt);
                }
                if ((obstacle & OBSTACLE_TOP) == 0) {
                        values[1] = computeG3DT(localVelocity_, localMeshsize_, localViscosity_, parameters_, parameters_.timestep.dt);
                }
                if ((obstacle & OBSTACLE_BACK) == 0) {
                        values[2] = computeH3DT(localVelocity_, localMeshsize_, localViscosity_, parameters_, parameters_.timestep.dt);
		}
	}else{
		if ((obstacle & OBSTACLE_RIGHT) == 0) { // If the right cell is fluid
			values[0] = computeF3D(localVelocity_, localMeshsize_, parameters_, parameters_.timestep.dt);
                }
                if ((obstacle & OBSTACLE_TOP) == 0) {
                        values[1] = computeG3D(localVelocity_, localMeshsize_, parameters_, parameters_.timestep.dt);
                }
                if ((obstacle & OBSTACLE_BACK) == 0) {
                        values[2] = computeH3D(localVelocity_, localMeshsize_, parameters_, parameters_.timestep.dt);
                }
	}
    }
}

} // namespace Stencils
} // namespace NSEOF
