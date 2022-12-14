#ifndef __STENCILS_NEUMANN_BOUNDARY_STENCIL_HPP__
#define __STENCILS_NEUMANN_BOUNDARY_STENCIL_HPP__

#include "Stencil.hpp"
#include "FlowField.hpp"
#include "Parameters.hpp"
#include "DataStructures.hpp"

namespace NSEOF {
namespace Stencils {

class NeumannVelocityBoundaryStencil : public BoundaryStencil<FlowField> {
public:
    NeumannVelocityBoundaryStencil(const Parameters& parameters);
    ~NeumannVelocityBoundaryStencil() override = default;

    void applyLeftWall(FlowField& flowField, int i, int j) override;
    void applyRightWall(FlowField& flowField, int i, int j) override;
    void applyBottomWall(FlowField& flowField, int i, int j) override;
    void applyTopWall(FlowField& flowField, int i, int j) override;

    void applyLeftWall(FlowField& flowField, int i, int j, int k) override;
    void applyRightWall(FlowField& flowField, int i, int j, int k) override;
    void applyBottomWall(FlowField& flowField, int i, int j, int k) override;
    void applyTopWall(FlowField& flowField, int i, int j, int k) override;
    void applyFrontWall(FlowField& flowField, int i, int j, int k) override;
    void applyBackWall(FlowField& flowField, int i, int j, int k) override;
};

class NeumannFGHBoundaryStencil : public BoundaryStencil<FlowField> {
public:
    NeumannFGHBoundaryStencil(const Parameters& parameters);
    ~NeumannFGHBoundaryStencil() override = default;

    void applyLeftWall(FlowField& flowField, int i, int j) override;
    void applyRightWall(FlowField& flowField, int i, int j) override;
    void applyBottomWall(FlowField& flowField, int i, int j) override;
    void applyTopWall(FlowField& flowField, int i, int j) override;

    void applyLeftWall(FlowField& flowField, int i, int j, int k) override;
    void applyRightWall(FlowField& flowField, int i, int j, int k) override;
    void applyBottomWall(FlowField& flowField, int i, int j, int k) override;
    void applyTopWall(FlowField& flowField, int i, int j, int k) override;
    void applyFrontWall(FlowField& flowField, int i, int j, int k) override;
    void applyBackWall(FlowField& flowField, int i, int j, int k) override;
};

} // namespace Stencils
} // namespace NSEOF

#endif // __STENCILS_NEUMANN_BOUNDARY_STENCIL_HPP__
