#ifndef __STENCILS_PRESSURE_BUFFER_FILL_STENCIL_HPP__
#define __STENCILS_PRESSURE_BUFFER_FILL_STENCIL_HPP__

#include "Stencil.hpp"
#include "Parameters.hpp"
#include "FlowField.hpp"

namespace NSEOF::Stencils {

/**
 * TODO: Write a definition here!
 */
class PressureBufferFillStencil : public BoundaryStencil<FlowField> {

public:
    explicit PressureBufferFillStencil(const Parameters&);
    ~PressureBufferFillStencil() override = default;

    void applyLeftWall   (FlowField&, int, int) override;
    void applyRightWall  (FlowField&, int, int) override;
    void applyBottomWall (FlowField&, int, int) override;
    void applyTopWall    (FlowField&, int, int) override;

    void applyLeftWall   (FlowField&, int, int, int) override;
    void applyRightWall  (FlowField&, int, int, int) override;
    void applyBottomWall (FlowField&, int, int, int) override;
    void applyTopWall    (FlowField&, int, int, int) override;
    void applyFrontWall  (FlowField&, int, int, int) override;
    void applyBackWall   (FlowField&, int, int, int) override;
};

} // namespace NSEOF::Stencils

#endif // __STENCILS_PRESSURE_BUFFER_FILL_STENCIL_HPP__
