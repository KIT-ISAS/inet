//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __INET_SERUMSUPPORT_H_
#define __INET_SERUMSUPPORT_H_

#include <omnetpp.h>

#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv6/IPv6Datagram.h"

#include "inet/networklayer/serum/MonitoringCollector.h"
#include "inet/networklayer/serum/SerumHeader_m.h"

using namespace omnetpp;

namespace inet {

/*
 * Module providing SERUM support in routers.
 */
class SerumSupport: public cSimpleModule {

public:
    SerumSupport();
    void handleRoutedPacket(IPv6Datagram * const pkt, IPv6HopByHopOptionsHeader * const hho, const InterfaceEntry * const fromIE, const InterfaceEntry * const destIE);

    static void initiateRequest(IPv6ExtensionHeader * const eh, SerumRecord * const req);
    static void initiateRequest(IPv6ExtensionHeader * const eh, std::vector<SerumRecord *> reqs);
    static bool containsRequest(IPv6ExtensionHeader * const eh, const short dataDesc = -1);
    static std::vector<SerumRecord *> extractRequest(IPv6ExtensionHeader * const eh, const short dataDesc = -1);
    static void initiateResponse(IPv6ExtensionHeader * const req, IPv6HopByHopOptionsHeader * const resp, const short dataDesc = -1);
    static bool containsResponse(IPv6ExtensionHeader * const eh, const short dataDesc);
    static std::vector<SerumRecord *> extractResponse(IPv6ExtensionHeader * const eh, const short dataDesc); // provides view on response contained in EH, does not transfer ownership
    static bool containsPush(IPv6ExtensionHeader * const eh, const short dataDesc);
    static std::vector<SerumRecord *> extractPush(IPv6ExtensionHeader * const eh, const short dataDesc); // provides view on push contained in EH, does not transfer ownership

public:

    struct DatasetInfo {
        const short dataDesc;
        const bool reverse; // normally, dataset is associated with outbound interface. in reversed mode, it is associated with inbound interface
    };

    class RecordHandler {
    public:
        virtual ~RecordHandler() { };
        virtual std::vector<DatasetInfo> datasetDescriptors() = 0;
        virtual void setMonitoringCollector(MonitoringCollector * const mc) { this->mc = mc; } ;
        virtual void* interfaceAdded(const InterfaceEntry * const ie, const short dataDesc) { return nullptr; };
        virtual void handleAppendRecord(void * const handlerData, IPv6Datagram * const pkt, TLVOptions * const opts, const uint optIndex) { };
        virtual void handleInlineRecord(void * const handlerData, IPv6Datagram * const pkt, TLVOptions * const opts, const uint optIndex) { };
    protected:
        MonitoringCollector * mc = nullptr;
    };

protected:

    struct InterfaceData {
        const int interfaceId;

        std::vector<void *> handlerData;

        InterfaceData(const int interfaceId);
    };

    struct HandlerEntry {
        RecordHandler * const handler;
        const DatasetInfo di;
    };

    std::vector<InterfaceData> ifaces;
    std::vector<HandlerEntry> handlers;

    virtual int numInitStages() const;
    virtual void initialize(const int stage);
    virtual void handleMessage(cMessage * const msg);

private:
    MonitoringCollector * mc = nullptr;

    int findHandler(const short dataDesc);
    InterfaceData * getOrCreateInterface(const InterfaceEntry * const ie);
};

} //namespace

#endif
