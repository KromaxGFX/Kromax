#ifndef KROMAX_SRC_CORE_QUEUE_H
#define KROMAX_SRC_CORE_QUEUE_H

/*
 * Queue.h
 *
 * Internal Queue object. Created from a Device; executes submitted
 * command buffers.
 */

#include "Object.h"

namespace kx {

class Queue : public Object {
public:
    Queue(uint32_t family_index, uint32_t index) noexcept
        : Object(ObjectType::Queue),
          family_index_(family_index),
          index_(index) {}

    uint32_t family_index() const noexcept { return family_index_; }
    uint32_t index() const noexcept { return index_; }

private:
    uint32_t family_index_;
    uint32_t index_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_QUEUE_H */