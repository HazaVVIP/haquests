#pragma once

#include <cstdint>
#include <cstddef>

namespace haquests {
namespace tcp {

class Window {
public:
    Window();
    
    // Set window size
    void setSize(uint16_t size);
    
    // Get window size
    uint16_t getSize() const;
    
    // Update window based on received data
    void update(size_t bytes_received);
    
    // Check if we can send
    bool canSend(size_t bytes) const;
    
    // Get available space
    size_t getAvailableSpace() const;

private:
    uint16_t size_;
    size_t used_;
};

} // namespace tcp
} // namespace haquests
