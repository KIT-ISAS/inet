#include "inet/networklayer/contract/NetworkOptions.h"

using namespace omnetpp;

namespace inet {

Register_Class(NetworkOptions);

/*
 * NetworkOptions
 */
void NetworkOptions::copy(const NetworkOptions& other) {
#ifdef WITH_IPv6

    for (auto eh : headers) {
        delete eh;
    }

    headers.clear();
    headers.reserve(other.headers.size());

    for (auto eh : other.headers) {
         headers.push_back(eh->dup());
    }

#endif
}

NetworkOptions::NetworkOptions() :
        NetworkOptions_Base() {
}

NetworkOptions::NetworkOptions(const NetworkOptions& other) :
        NetworkOptions_Base(other) {
    copy(other);
}

NetworkOptions::~NetworkOptions() {
#ifdef WITH_IPv6

    for (auto eh : headers) {
        delete eh;
    }

    headers.clear();

#endif
}

NetworkOptions& NetworkOptions::operator=(const NetworkOptions& other) {
    if (this == &other)
        return *this;

    NetworkOptions_Base::operator=(other);
    copy(other);

    return *this;
}

NetworkOptions * NetworkOptions::dup() const {
    return new NetworkOptions(*this);
}

#ifdef WITH_IPv6

short NetworkOptions::getV6HeaderIndex(const short type) {
    short result = 0;

    for (auto eh : headers) {
        if (eh->getExtensionType() == type) {
            return result;
        }

        result++;
    }

    return -1;
}

short NetworkOptions::getV6HeaderCount() {
    return headers.size();
}

void NetworkOptions::addV6Header(IPv6ExtensionHeader * const eh) {
    ASSERT(eh);

    if (getV6HeaderIndex(eh->getExtensionType()) >= 0) {
        throw cRuntimeError(this, "addHeader() duplicate extension header: %d",
                            eh->getExtensionType());
    }

    headers.push_back(eh);
}

IPv6ExtensionHeader * NetworkOptions::getV6Header(const short index) {
    return headers[index];
}

IPv6ExtensionHeader * NetworkOptions::removeV6Header(const short index) {
    IPv6ExtensionHeader * const result = headers[index];

    headers.erase(headers.begin() + index);

    return result;
}

#endif

}
