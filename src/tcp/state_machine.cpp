#include "haquests/tcp/state_machine.hpp"

namespace haquests {
namespace tcp {

StateMachine::StateMachine() : state_(core::TCPState::CLOSED) {}

core::TCPState StateMachine::getState() const {
    return state_;
}

void StateMachine::onSendSYN() {
    if (state_ == core::TCPState::CLOSED) {
        state_ = core::TCPState::SYN_SENT;
    }
}

void StateMachine::onReceiveSYNACK() {
    if (state_ == core::TCPState::SYN_SENT) {
        state_ = core::TCPState::ESTABLISHED;
    }
}

void StateMachine::onSendACK() {
    if (state_ == core::TCPState::SYN_RECEIVED) {
        state_ = core::TCPState::ESTABLISHED;
    }
}

void StateMachine::onEstablished() {
    state_ = core::TCPState::ESTABLISHED;
}

void StateMachine::onSendFIN() {
    if (state_ == core::TCPState::ESTABLISHED) {
        state_ = core::TCPState::FIN_WAIT_1;
    }
}

void StateMachine::onReceiveFIN() {
    if (state_ == core::TCPState::ESTABLISHED) {
        state_ = core::TCPState::CLOSE_WAIT;
    } else if (state_ == core::TCPState::FIN_WAIT_1) {
        state_ = core::TCPState::CLOSING;
    } else if (state_ == core::TCPState::FIN_WAIT_2) {
        state_ = core::TCPState::TIME_WAIT;
    }
}

void StateMachine::onReceiveACK() {
    if (state_ == core::TCPState::FIN_WAIT_1) {
        state_ = core::TCPState::FIN_WAIT_2;
    } else if (state_ == core::TCPState::CLOSING) {
        state_ = core::TCPState::TIME_WAIT;
    } else if (state_ == core::TCPState::LAST_ACK) {
        state_ = core::TCPState::CLOSED;
    }
}

void StateMachine::onClose() {
    state_ = core::TCPState::CLOSED;
}

void StateMachine::onReset() {
    state_ = core::TCPState::CLOSED;
}

bool StateMachine::canTransition(core::TCPState new_state) const {
    // Simplified transition validation
    return true;
}

void StateMachine::setState(core::TCPState state) {
    state_ = state;
}

} // namespace tcp
} // namespace haquests
