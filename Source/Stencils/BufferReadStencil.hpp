#ifndef __STENCILS_BUFFER_READ_STENCIL_HPP__
#define __STENCILS_BUFFER_READ_STENCIL_HPP__

#include "Stencil.hpp"
#include "FlowField.hpp"
#include "Parameters.hpp"
#include "Definitions.hpp"

#include <vector>
#include <utility>

namespace NSEOF::Stencils {

/**
 * A boundary stencil that reads the buffer
 */
class BufferReadStencil : public BoundaryStencil<FlowField> {
private:
    std::vector<FLOAT> bufferLeft_;
    std::vector<FLOAT> bufferRight_;
    std::vector<FLOAT> bufferBottom_;
    std::vector<FLOAT> bufferTop_;
    std::vector<FLOAT> bufferFront_;
    std::vector<FLOAT> bufferBack_;

    std::vector<FLOAT>::iterator bufferLeftIterator_;
    std::vector<FLOAT>::iterator bufferRightIterator_;
    std::vector<FLOAT>::iterator bufferBottomIterator_;
    std::vector<FLOAT>::iterator bufferTopIterator_;
    std::vector<FLOAT>::iterator bufferFrontIterator_;
    std::vector<FLOAT>::iterator bufferBackIterator_;

public:
    explicit BufferReadStencil(const Parameters&);

    void clearBuffers();
    ~BufferReadStencil() override;

    /**
     * Gets the next value in the iterator and increments
     */
    FLOAT getNextInBufferLeft  ();
    FLOAT getNextInBufferRight ();
    FLOAT getNextInBufferBottom();
    FLOAT getNextInBufferTop   ();
    FLOAT getNextInBufferFront ();
    FLOAT getNextInBufferBack  ();

    /**
     * Setters for buffers and their iterators
     */
    void setBufferLeftIterator  (std::vector<FLOAT>&);
    void setBufferRightIterator (std::vector<FLOAT>&);
    void setBufferBottomIterator(std::vector<FLOAT>&);
    void setBufferTopIterator   (std::vector<FLOAT>&);
    void setBufferFrontIterator (std::vector<FLOAT>&);
    void setBufferBackIterator  (std::vector<FLOAT>&);

    /**
     * Setters for single elements in the buffers
     */
    void setBufferLeftElement  (int, FLOAT);
    void setBufferRightElement (int, FLOAT);
    void setBufferBottomElement(int, FLOAT);
    void setBufferTopElement   (int, FLOAT);
    void setBufferFrontElement (int, FLOAT);
    void setBufferBackElement  (int, FLOAT);
};

} // namespace NSEOF::Stencils

#endif // __STENCILS_BUFFER_READ_STENCIL_HPP__
