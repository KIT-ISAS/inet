#include "inet/transportlayer/contract/udp/UDPControlInfo.h"

using namespace omnetpp;

namespace inet {

Register_Class(UDPSendCommand);
Register_Class(UDPDataIndication);

/*
 * UDPSendCommand
 */

void UDPSendCommand::copy(const UDPSendCommand& other) {
    if (networkOptions) {
        delete networkOptions;

        networkOptions = nullptr;
    }

    if (other.networkOptions) {
        this->networkOptions = other.networkOptions->dup();
    }
}

UDPSendCommand::UDPSendCommand() :
        UDPSendCommand_Base() {
    networkOptions = nullptr;
}

UDPSendCommand::UDPSendCommand(const UDPSendCommand& other) :
        UDPSendCommand_Base(other) {
    copy(other);
}

UDPSendCommand::~UDPSendCommand() {
    if (this->networkOptions) {
        delete this->networkOptions;
    }
}

UDPSendCommand& UDPSendCommand::operator=(const UDPSendCommand& other) {
    if (this == &other)
        return *this;

    UDPSendCommand_Base::operator=(other);
    copy(other);

    return *this;
}
UDPSendCommand * UDPSendCommand::dup() const {
    return new UDPSendCommand(*this);
}

const NetworkOptionsPtr& UDPSendCommand::getNetworkOptions() const {
    return networkOptions;
}

void UDPSendCommand::setNetworkOptions(const NetworkOptionsPtr& networkOptions) {
    if (this->networkOptions) {
        delete this->networkOptions;
    }

    this->networkOptions = networkOptions;
}

NetworkOptionsPtr UDPSendCommand::replaceNetworkOptions(const NetworkOptionsPtr networkOptions) {
    NetworkOptionsPtr result = this->networkOptions;

    this->networkOptions = networkOptions;

    return result;
}

/*
 * UDPDataIndication
 */

void UDPDataIndication::copy(const UDPDataIndication& other) {
    if (networkOptions) {
        delete networkOptions;

        networkOptions = nullptr;
    }

    if (other.networkOptions) {
        this->networkOptions = other.networkOptions->dup();
    }
}

UDPDataIndication::UDPDataIndication() :
        UDPDataIndication_Base() {
    networkOptions = nullptr;
}

UDPDataIndication::UDPDataIndication(const UDPDataIndication& other) :
        UDPDataIndication_Base(other) {
    copy(other);
}

UDPDataIndication::~UDPDataIndication() {
    if (this->networkOptions) {
        delete this->networkOptions;
    }
}

UDPDataIndication& UDPDataIndication::operator=(const UDPDataIndication& other) {
    if (this == &other)
        return *this;

    UDPDataIndication_Base::operator=(other);
    copy(other);

    return *this;
}
UDPDataIndication * UDPDataIndication::dup() const {
    return new UDPDataIndication(*this);
}

const NetworkOptionsPtr& UDPDataIndication::getNetworkOptions() const {
    return networkOptions;
}

void UDPDataIndication::setNetworkOptions(const NetworkOptionsPtr& networkOptions) {
    if (this->networkOptions) {
        delete this->networkOptions;
    }

    this->networkOptions = networkOptions;
}

NetworkOptionsPtr UDPDataIndication::replaceNetworkOptions(const NetworkOptionsPtr networkOptions) {
    NetworkOptionsPtr result = this->networkOptions;

    this->networkOptions = networkOptions;

    return result;
}

}
