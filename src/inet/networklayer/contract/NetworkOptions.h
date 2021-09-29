#ifndef __INET_NETWORKOPTIONS_H
#define __INET_NETWORKOPTIONS_H

#include "inet/features.h"

#include "inet/networklayer/contract/NetworkOptions_m.h"

#ifdef WITH_IPv6
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#endif

namespace inet {

class NetworkOptions: public NetworkOptions_Base {
private:
    void copy(const NetworkOptions& other);

#ifdef WITH_IPv6
    std::vector<IPv6ExtensionHeader *> headers;
#endif

public:
    NetworkOptions();
    NetworkOptions(const NetworkOptions& other);
    ~NetworkOptions();

    NetworkOptions& operator=(const NetworkOptions& other);
    virtual NetworkOptions *dup() const;

#ifdef WITH_IPv6
    short getV6HeaderIndex(const short type);
    short getV6HeaderCount();

    void addV6Header(IPv6ExtensionHeader * const eh);
    IPv6ExtensionHeader * getV6Header(const short index);
    IPv6ExtensionHeader * removeV6Header(const short index);
#endif
};

} // namespace inet

#endif // ifndef __INET_NETWORKOPTIONS_H

