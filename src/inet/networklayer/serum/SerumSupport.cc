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

#include "SerumSupport.h"

#include <omnetpp/cstringtokenizer.h>
#include "inet/common/ModuleAccess.h"

using namespace omnetpp;

namespace inet {

Define_Module(SerumSupport);


SerumSupport::InterfaceData::InterfaceData(const int interfaceId) :
        interfaceId(interfaceId) {
}

SerumSupport::SerumSupport() {

}

int SerumSupport::numInitStages() const {
    return INITSTAGE_LAST;
}

void SerumSupport::initialize(const int stage) {
    if (stage != INITSTAGE_LOCAL + 1) {
        return;
    }

    mc = getModuleFromPar<MonitoringCollector>(par("monitoringCollectorModule"), this, false); // might return NULL for end systems

    const char * handlerNames = par("handlers").stringValue();

    cStringTokenizer handlerTok(handlerNames);

    for (const char * token = handlerTok.nextToken(); token != nullptr; token = handlerTok.nextToken()) {
        cModuleType * const moduleType = cModuleType::get(token);
        cModule * const module = moduleType->createScheduleInit(moduleType->getName(), this);

        SerumSupport::RecordHandler * const handler = dynamic_cast<SerumSupport::RecordHandler * const>(module);

        if (handler == nullptr) {
            error("Failed to instantiate SERUM record handler implementation %s", token);
        }

        handler->setMonitoringCollector(mc);

        for (auto di : handler->datasetDescriptors()) {
            handlers.push_back(HandlerEntry{handler, di});
        }
    }
}

void SerumSupport::handleMessage(cMessage * const msg) {
    const int msgKind = msg->getKind();

    delete msg;

    error("Received message with unexpected message kind: %i", msgKind);
}

void SerumSupport::handleRoutedPacket(IPv6Datagram * const pkt, IPv6HopByHopOptionsHeader * const hho, const InterfaceEntry * const fromIE, const InterfaceEntry * const destIE) {
    Enter_Method_Silent();

    TLVOptions * const tlv = &hho->getTlvOptions();

    for (int i = 0; i < tlv->size(); i++) {
        const short type = tlv->at(i).getType();

        if (type == MONITORING_HH_APPEND || type == MONITORING_HH_INLINE) {
            SerumRecord * const sr = dynamic_cast<SerumRecord *>(&tlv->at(i));
            const int hi = findHandler(sr->getDataDesc());

            if (hi < 0) {
                // no handler registered for this descriptor
                continue;
            }

            const InterfaceEntry * const ie = !handlers[hi].di.reverse ? destIE : fromIE;
            SerumSupport::InterfaceData * const id = getOrCreateInterface(ie);

            if (type == MONITORING_HH_INLINE) {
                handlers[hi].handler->handleInlineRecord(id->handlerData[hi], pkt, tlv, i);
            } else { // MONITORING_HH_APPEND
                handlers[hi].handler->handleAppendRecord(id->handlerData[hi], pkt, tlv, i);
            }
        }
    }
}

void SerumSupport::initiateRequest(IPv6ExtensionHeader * const eh, SerumRecord * const req) {
    ASSERT(eh->getExtensionType() == IP_PROT_IPv6EXT_DEST);

    IPv6DestinationOptionsHeader * const doh = dynamic_cast<IPv6DestinationOptionsHeader *>(eh);
    TLVOptions * const opts = &doh->getTlvOptions();

    opts->add(req);
}

void SerumSupport::initiateRequest(IPv6ExtensionHeader * const eh, std::vector<SerumRecord *> reqs) {
    ASSERT(eh->getExtensionType() == IP_PROT_IPv6EXT_DEST);

    IPv6DestinationOptionsHeader * const doh = dynamic_cast<IPv6DestinationOptionsHeader *>(eh);
    TLVOptions * const opts = &doh->getTlvOptions();

    for (auto record : reqs) {
        opts->add(record);
    }
}


bool SerumSupport::containsRequest(IPv6ExtensionHeader * const eh, const short dataDesc) {
    if (eh->getExtensionType() == IP_PROT_IPv6EXT_DEST) {
        IPv6DestinationOptionsHeader * const doh = dynamic_cast<IPv6DestinationOptionsHeader *>(eh);
        TLVOptions tlv = doh->getTlvOptions();

        for (unsigned int i = 0; i < tlv.getTlvOptionArraySize(); i++) {
            if (tlv.getTlvOption(i).getType() == MONITORING_TO_DST
                    && (dataDesc < 0 || dynamic_cast<SerumRecord *>(&tlv.getTlvOption(i))->getDataDesc() == dataDesc)) {
                return true;
            }
        }
    }

    return false;
}

std::vector<SerumRecord *> SerumSupport::extractRequest(IPv6ExtensionHeader * const eh, const short dataDesc) {
    ASSERT(eh->getExtensionType() == IP_PROT_IPv6EXT_DEST);

    std::vector<SerumRecord *> result;
    IPv6DestinationOptionsHeader * const dho = dynamic_cast<IPv6DestinationOptionsHeader *>(eh);
    TLVOptions * const tlv = &dho->getTlvOptions();

    for (unsigned int i = 0; i < tlv->getTlvOptionArraySize(); i++) {
        if (tlv->getTlvOption(i).getType() == MONITORING_TO_DST
                && (dataDesc < 0 || dynamic_cast<SerumRecord *>(&tlv->getTlvOption(i))->getDataDesc() == dataDesc)) {
            result.push_back(dynamic_cast<SerumRecord *>(&tlv->getTlvOption(i)));
        }
    }

    return result;
}

void SerumSupport::initiateResponse(IPv6ExtensionHeader * const req, IPv6HopByHopOptionsHeader * const resp, const short dataDesc) {
    ASSERT(req->getExtensionType() == IP_PROT_IPv6EXT_DEST);

    IPv6DestinationOptionsHeader * const doh = dynamic_cast<IPv6DestinationOptionsHeader *>(req);
    TLVOptions tlv = doh->getTlvOptions();

    TLVOptions * const ro = &resp->getTlvOptions();

    for (unsigned int i = 0; i < tlv.getTlvOptionArraySize(); i++) {
        if (tlv.getTlvOption(i).getType() == MONITORING_TO_DST
                && (dataDesc < 0 || dynamic_cast<SerumRecord *>(&tlv.getTlvOption(i))->getDataDesc() == dataDesc)) {
            SerumRecord * const rec = new SerumRecord();

            rec->setType(MONITORING_HH_APPEND);
            rec->setDataDesc(dynamic_cast<SerumRecord *>(&tlv.getTlvOption(i))->getDataDesc() * -1); // initial hop set negative descriptor id for empty request
            ro->add(rec);
        }
    }
}

bool SerumSupport::containsResponse(IPv6ExtensionHeader * const eh, const short dataDesc) {
    if (eh->getExtensionType() == IP_PROT_IPv6EXT_HOP) {
        IPv6HopByHopOptionsHeader * const hho = dynamic_cast<IPv6HopByHopOptionsHeader *>(eh);
        TLVOptions tlv = hho->getTlvOptions();

        for (unsigned int i = 0; i < tlv.getTlvOptionArraySize(); i++) {
            if (tlv.getTlvOption(i).getType() == MONITORING_HH_APPEND
                    && dynamic_cast<SerumRecord *>(&tlv.getTlvOption(i))->getDataDesc() == dataDesc) {
                return true;
            }
        }
    }

    return false;
}

std::vector<SerumRecord *> SerumSupport::extractResponse(IPv6ExtensionHeader * const eh, const short dataDesc) {
    ASSERT(eh->getExtensionType() == IP_PROT_IPv6EXT_HOP);

    std::vector<SerumRecord *> result;
    IPv6HopByHopOptionsHeader * const hho = dynamic_cast<IPv6HopByHopOptionsHeader *>(eh);
    TLVOptions * const tlv = &hho->getTlvOptions();

    for (unsigned int i = 0; i < tlv->getTlvOptionArraySize(); i++) {
        if (tlv->getTlvOption(i).getType() == MONITORING_HH_APPEND
                && dynamic_cast<SerumRecord *>(&tlv->getTlvOption(i))->getDataDesc() == dataDesc) {
            // automagically filters our first request, since this has negative descriptor
            result.push_back(dynamic_cast<SerumRecord *>(&tlv->getTlvOption(i)));
        }
    }

    return result;
}

bool SerumSupport::containsPush(IPv6ExtensionHeader * const eh, const short dataDesc) {
    if (eh->getExtensionType() == IP_PROT_IPv6EXT_HOP) {
        IPv6HopByHopOptionsHeader * const hho = dynamic_cast<IPv6HopByHopOptionsHeader *>(eh);
        TLVOptions tlv = hho->getTlvOptions();

        for (unsigned int i = 0; i < tlv.getTlvOptionArraySize(); i++) {
            if (tlv.getTlvOption(i).getType() == MONITORING_HH_INLINE
                    && dynamic_cast<SerumRecord *>(&tlv.getTlvOption(i))->getDataDesc() == dataDesc) {
                return true;
            }
        }
    }

    return false;
}

std::vector<SerumRecord *> SerumSupport::extractPush(IPv6ExtensionHeader * const eh, const short dataDesc) {
    ASSERT(eh->getExtensionType() == IP_PROT_IPv6EXT_HOP);

    std::vector<SerumRecord *> result;
    IPv6HopByHopOptionsHeader * const hho = dynamic_cast<IPv6HopByHopOptionsHeader *>(eh);
    TLVOptions * const tlv = &hho->getTlvOptions();

    for (unsigned int i = 0; i < tlv->getTlvOptionArraySize(); i++) {
        if (tlv->getTlvOption(i).getType() == MONITORING_HH_INLINE
                && dynamic_cast<SerumRecord *>(&tlv->getTlvOption(i))->getDataDesc() == dataDesc) {
            result.push_back(dynamic_cast<SerumRecord *>(&tlv->getTlvOption(i)));
        }
    }

    return result;
}

int SerumSupport::findHandler(const short dataDesc) {
    for (uint i = 0; i < handlers.size(); i++) {
        if (handlers[i].di.dataDesc == dataDesc) {
            return i;
        }
    }

    return -1;
}

SerumSupport::InterfaceData * SerumSupport::getOrCreateInterface(const InterfaceEntry * const ie) {
    for (auto iter = ifaces.begin(); iter != ifaces.end(); iter++) {
        if (iter->interfaceId == ie->getInterfaceId()) {
            return &(*iter);
        }
    }

    ifaces.push_back(SerumSupport::InterfaceData(ie->getInterfaceId()));

    for (auto h : handlers) {
        void * const handlerData = h.handler->interfaceAdded(ie, h.di.dataDesc);

        ifaces.back().handlerData.push_back(handlerData);
    }

    return &ifaces.back();
}

} //namespace
