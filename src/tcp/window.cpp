#include "haquests/tcp/window.hpp"
#include "haquests/core/types.hpp"

namespace haquests {
namespace tcp {

Window::Window() : size_(core::DEFAULT_WINDOW_SIZE), used_(0) {}

void Window::setSize(uint16_t size) {
    size_ = size;
}

uint16_t Window::getSize() const {
    return size_;
}

void Window::update(size_t bytes_received) {
    if (bytes_received <= used_) {
        used_ -= bytes_received;
    } else {
        used_ = 0;
    }
}

bool Window::canSend(size_t bytes) const {
    return (used_ + bytes) <= size_;
}

size_t Window::getAvailableSpace() const {
    if (used_ >= size_) {
        return 0;
    }
    return size_ - used_;
}

} // namespace tcp
} // namespace haquests
