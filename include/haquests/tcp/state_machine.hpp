#pragma once

#include "haquests/core/types.hpp"
#include <cstdint>

namespace haquests {
namespace tcp {

class StateMachine {
public:
    StateMachine();
    
    // Get current state
    core::TCPState getState() const;
    
    // State transitions
    void onSendSYN();
    void onReceiveSYNACK();
    void onSendACK();
    void onEstablished();
    void onSendFIN();
    void onReceiveFIN();
    void onReceiveACK();
    void onClose();
    void onReset();
    
    // Validate transition
    bool canTransition(core::TCPState new_state) const;
    
    // Force state (for testing)
    void setState(core::TCPState state);

private:
    core::TCPState state_;
};

} // namespace tcp
} // namespace haquests
