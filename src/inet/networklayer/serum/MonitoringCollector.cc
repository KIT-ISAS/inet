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

#include "MonitoringCollector.h"

namespace inet {

Define_Module(MonitoringCollector);

#define TICKER_MSG_KIND 9023 // randomly chosen

simsignal_t MonitoringCollector::txStateSignal = registerSignal("txState");
simsignal_t MonitoringCollector::queueLengthSignal = registerSignal("queueLength");
simsignal_t MonitoringCollector::dequeuePkSignal = registerSignal("dequeuePk");
simsignal_t MonitoringCollector::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t MonitoringCollector::dropPkByQueueSignal = registerSignal("dropPkByQueue");

MonitoringCollector::QueueData::QueueData(InterfaceData * const iface, cObject * const queue) :
        queue(dynamic_cast<cModule *>(queue)) {
    // frame capacity is only available for DropTailQueue currently
    if (this->queue->hasPar("frameCapacity")) {
        queueSize = this->queue->par("frameCapacity").intValue();
    } else {
        queueSize = -1;
    }

    std::string signalPrefix = iface->signalPrefix + queue->getName() + "-";
    std::string lengthName = signalPrefix + "qLength";
    std::string ingressName = signalPrefix + "qIngress";
    std::string egressName = signalPrefix + "qEgress";
    std::string dropName = signalPrefix + "qDrop";
    std::string lengthFilteredName = lengthName + "Filtered";
    std::string ingressFilteredName = ingressName + "Filtered";
    std::string egressFilteredName = egressName + "Filtered";
    std::string lengthErrorName = lengthName + "Error";
    std::string ingressErrorName = ingressName + "Error";
    std::string egressErrorName = egressName + "Error";
    lengthSignal = registerSignal(lengthName.c_str());
    ingressRateSignal = registerSignal(ingressName.c_str());
    egressRateSignal = registerSignal(egressName.c_str());
    dropSignal = registerSignal(dropName.c_str());
    lengthSignalFiltered = registerSignal(lengthFilteredName.c_str());
    ingressRateSignalFiltered = registerSignal(ingressFilteredName.c_str());
    egressRateSignalFiltered = registerSignal(egressFilteredName.c_str());
    lengthSignalError = registerSignal(lengthErrorName.c_str());
    ingressRateSignalError = registerSignal(ingressErrorName.c_str());
    egressRateSignalError = registerSignal(egressErrorName.c_str());
    cProperty* lengthStatistic = iface->mc->getProperties()->get("statisticTemplate", "queueLength");
    cProperty* ingressStatistic = iface->mc->getProperties()->get("statisticTemplate", "queueIngressRate");
    cProperty* egressStatistic = iface->mc->getProperties()->get("statisticTemplate", "queueEgressRate");
    cProperty* dropStatistic = iface->mc->getProperties()->get("statisticTemplate", "queueDrop");

    cSimulation::getActiveEnvir()->addResultRecorders(iface->mc, lengthSignal, lengthName.c_str(), lengthStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(iface->mc, ingressRateSignal, ingressName.c_str(), ingressStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(iface->mc, egressRateSignal, egressName.c_str(), egressStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(iface->mc, dropSignal, dropName.c_str(), dropStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(iface->mc, lengthSignalFiltered, lengthFilteredName.c_str(), lengthStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(iface->mc, ingressRateSignalFiltered, ingressFilteredName.c_str(), ingressStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(iface->mc, egressRateSignalFiltered, egressFilteredName.c_str(), egressStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(iface->mc, lengthSignalError, lengthErrorName.c_str(), lengthStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(iface->mc, ingressRateSignalError, ingressErrorName.c_str(), ingressStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(iface->mc, egressRateSignalError, egressErrorName.c_str(), egressStatistic);
}

void MonitoringCollector::QueueData::queueLengthEvent(const long size) {
    if (buckets.size() <= lastQueueLength) { // must grow list first
        buckets.resize(lastQueueLength + 1);
    }
    buckets[lastQueueLength] += simTime() - lastQueueSizeEvent;
    lastQueueSizeEvent = simTime();
    lastQueueLength = size;
}

void MonitoringCollector::QueueData::ingressEvent(const long msgBits) {
    bitsReceived += msgBits;
}

void MonitoringCollector::QueueData::dequeueEvent(const long msgBits) {
    bitsSent += bitsPending; // pending bits must have been sent since new packet is requested
    bitsPending = msgBits;
    lastPendingUpdate = simTime();
    // also track size and number of pkts
    pktBits += msgBits;
    pktCount++;
}

void MonitoringCollector::QueueData::dropEvent(const long msgBits) {
    pktDrop++;
}

void MonitoringCollector::QueueData::computeQueueStatistics(QueueStatistics &stats, const simtime_t interval, const long lineRate) {
    // queue length computations
    // finish actual monitoring period period
    dequeueEvent(lastQueueLength);

    simtime_t timeSum = SIMTIME_ZERO;
    double weightedSum = 0;

    for (unsigned long i = 0; i < buckets.size(); i++) {
        timeSum += buckets[i];
        weightedSum += buckets[i].dbl() * i;
        buckets[i] = SIMTIME_ZERO; // reset
    }

    stats.avgLength.filter((weightedSum > interval.dbl() / 1e6) ? weightedSum / timeSum : 0); // cut off noise floor / empty queue

    // outbound rate computations
    // compute pending bits which have already been sent
    // computation is safe even if line rate is unknown, just less precise in temporal domain
    const simtime_t sendingTime = simTime() - lastPendingUpdate;
    const long maxTransmittedBits = lineRate * sendingTime.dbl();

    if (maxTransmittedBits > bitsPending) {
        bitsSent += bitsPending;
    } else {
        bitsSent += maxTransmittedBits;
        bitsPending -= maxTransmittedBits;
        lastPendingUpdate = simTime();
    }

    // compute actual outbound rate
    stats.avgEgressRate.filter(bitsSent / interval);
    // ... and approximated inbound rate
    stats.avgIngressRate.filter(bitsReceived / interval);

    // droppet pkts
    stats.pktDrop = pktDrop;

    // compute average packet size
    stats.avgPktSize = pktCount > 0 ? pktBits / pktCount : 0;
    stats.seenPkts = pktCount;

    // reset counter
    bitsReceived = 0;
    bitsSent = 0;
    pktDrop = 0;
    pktBits = 0;
    pktCount = 0;
}

MonitoringCollector::InterfaceData::InterfaceData(MonitoringCollector * const mc, cModule * const interface, const long lineRate) :
        mc(mc),
        interface(interface),
        accumulatedSendingTime(SIMTIME_ZERO),
        inTransmission(false),
        lastTransmissionStart(SIMTIME_ZERO),
        queues(),
        stats() {
    stats.lineRate = lineRate;

    // configure filter
    stats.avgUtilization = PiecewiseSLRFilter(mc->filterLength, mc->triggerLength, mc->relativeErrorThreshold, -1.0);

    // setup per-link egress statistics
    signalPrefix = "ppp" + std::to_string(interface->getIndex()) + "-";
    std::string utilizationName = signalPrefix + "utilization";
    std::string rateName = signalPrefix + "bitrate";
    std::string utilizationFilteredName = utilizationName + "Filtered";
    std::string rateFilteredName = rateName + "Filtered";
    std::string utilizationErrorName = utilizationName + "Error";
    std::string rateErrorName = rateName + "Error";
    utilization = registerSignal(utilizationName.c_str());
    rate = registerSignal(rateName.c_str());
    utilizationFiltered = registerSignal(utilizationFilteredName.c_str());
    rateFiltered = registerSignal(rateFilteredName.c_str());
    utilizationError= registerSignal(utilizationErrorName.c_str());
    rateError = registerSignal(rateErrorName.c_str());
    cProperty* utilizationStatistic = mc->getProperties()->get("statisticTemplate", "linkUtilization");
    cProperty* rateStatistic = mc->getProperties()->get("statisticTemplate", "linkRate");

    cSimulation::getActiveEnvir()->addResultRecorders(mc, utilization, utilizationName.c_str(), utilizationStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(mc, rate, rateName.c_str(), rateStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(mc, utilizationFiltered, utilizationFilteredName.c_str(), utilizationStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(mc, rateFiltered, rateFilteredName.c_str(), rateStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(mc, utilizationError, utilizationErrorName.c_str(), utilizationStatistic);
    cSimulation::getActiveEnvir()->addResultRecorders(mc, rateError, rateErrorName.c_str(), rateStatistic);
}

void MonitoringCollector::InterfaceData::transmissionEvent(const long status) {
    if (status == 0 && inTransmission) {
        accumulatedSendingTime += simTime() - lastTransmissionStart;
        inTransmission = false;
    }
    if (status == 1) {
        lastTransmissionStart = simTime();
        inTransmission = true;
    }
}

MonitoringCollector::QueueData& MonitoringCollector::InterfaceData::getOrAddQueue(cObject * const queue) {
    for (auto it = queues.begin(); it != queues.end(); it++) {
        if (it->queue == queue) {
            return *it;
        }
    }

    queues.push_back(QueueData(this, queue));
    stats.queues.push_back(QueueStatistics());

    QueueData &qData = queues.back();
    QueueStatistics &qStats = stats.queues.back();

    qStats.name = queue->getName();
    qStats.size = std::max(qData.queueSize, (ulong) 0);

    // configure statistic filtering
    if (qData.queueSize > 0) {
        stats.queues.back().avgEgressRate = PiecewiseSLRFilter(mc->filterLength, mc->triggerLength, mc->relativeErrorThreshold, -stats.lineRate);
        stats.queues.back().avgIngressRate = PiecewiseSLRFilter(mc->filterLength, mc->triggerLength, mc->relativeErrorThreshold, -stats.lineRate);
        stats.queues.back().avgLength = PiecewiseSLRFilter(mc->filterLength, mc->triggerLength, mc->relativeErrorThreshold, -qStats.size);
    } else {
        // can not perform reasonable filtering if queue size is not known
        stats.queues.back().avgEgressRate = PiecewiseSLRFilter(1, 1, 0);
        stats.queues.back().avgIngressRate = PiecewiseSLRFilter(1, 1, 0);
        stats.queues.back().avgLength = PiecewiseSLRFilter(1, 1, 0);
    }

    return queues.back();
}

void MonitoringCollector::InterfaceData::queueLengthEvent(cObject * const src, const long length) {
    getOrAddQueue(src).queueLengthEvent(length);
}

void MonitoringCollector::InterfaceData::ingressEvent(cObject * const src, const long msgBits) {
    getOrAddQueue(src).ingressEvent(msgBits);
}

void MonitoringCollector::InterfaceData::dequeueEvent(cObject * const src, const long msgBits) {
    getOrAddQueue(src).dequeueEvent(msgBits);
}

void MonitoringCollector::InterfaceData::dropEvent(cObject * const src, const long msgBits) {
    getOrAddQueue(src).dropEvent(msgBits);
}

void MonitoringCollector::InterfaceData::computeQueueSize(const simtime_t interval) {
    ASSERT(stats.queues.size() == queues.size());

    for (uint i = 0; i < queues.size(); i++) {
        queues[i].computeQueueStatistics(stats.queues[i], interval, stats.lineRate);

        mc->emit(queues[i].lengthSignal, stats.queues[i].avgLength);
        mc->emit(queues[i].ingressRateSignal, stats.queues[i].avgIngressRate);
        mc->emit(queues[i].egressRateSignal, stats.queues[i].avgEgressRate);
        mc->emit(queues[i].dropSignal, stats.queues[i].pktDrop);
        mc->emit(queues[i].lengthSignalFiltered, stats.queues[i].avgLength.filtered());
        mc->emit(queues[i].ingressRateSignalFiltered, stats.queues[i].avgIngressRate.filtered());
        mc->emit(queues[i].egressRateSignalFiltered, stats.queues[i].avgEgressRate.filtered());
        mc->emit(queues[i].lengthSignalError, stats.queues[i].avgLength.averageError());
        mc->emit(queues[i].ingressRateSignalError, stats.queues[i].avgIngressRate.averageError());
        mc->emit(queues[i].egressRateSignalError, stats.queues[i].avgEgressRate.averageError());
    }
}

void MonitoringCollector::InterfaceData::computeUtilization(const simtime_t interval) {
    // finish actual monitoring period period
    if (inTransmission) {
        transmissionEvent(0);
        transmissionEvent(1);
    }

    stats.avgUtilization.filter(accumulatedSendingTime / interval);

    accumulatedSendingTime = 0; // reset
}

MonitoringCollector::MonitoringCollector() :
        tickerEvent(nullptr) {
}

MonitoringCollector::~MonitoringCollector() {
}

void MonitoringCollector::initialize() {
    cModule * const parent = getParentModule();

    if (par("enabled").boolValue()) {
        parent->subscribe(txStateSignal, this);
        parent->subscribe(queueLengthSignal, this);
        parent->subscribe(dequeuePkSignal, this);
        parent->subscribe(rcvdPkSignal, this);
        parent->subscribe(dropPkByQueueSignal, this);

        collectionInterval = par("collectionInterval").doubleValue();
        l2overhead = par("l2overhead").intValue();
        filterLength = par("filterLength").intValue();
        triggerLength = par("triggerLength").doubleValue();
        relativeErrorThreshold = par("relativeErrorThreshold").doubleValue();

        for (cModule::SubmoduleIterator iter(parent); !iter.end(); iter++) {
            cModule * const ppp = *iter;

            if (ppp->isName("ppp")) {
                // find link connected to pppg of parent module
                cGate * const pppPhys = ppp->gate(ppp->findGate("phys$o"));
                // find the transmission channel attached to its chain
                cChannel * const channel = pppPhys->getTransmissionChannel();
                cDatarateChannel * const drc = dynamic_cast<cDatarateChannel *>(channel);

                if (drc) {
                    EV_DEBUG << "found datarate channel with rate=" << (long) drc->getDatarate() << " for interface " << ppp->getFullPath() << endl;
                } else {
                    EV_DEBUG << "unable to determine link rate for interface " << ppp->getFullPath() << endl;
                }

                ifaces.push_back(InterfaceData(this, ppp, drc != nullptr ? (long) drc->getDatarate() : 0));
            }
        }

        tickerEvent = new cMessage("MonitoringCollector ticker event", TICKER_MSG_KIND);
        scheduleAt(collectionInterval, tickerEvent);
    }
}

void MonitoringCollector::handleMessage(cMessage * const msg) {
    switch (msg->getKind()) {
        case TICKER_MSG_KIND:
            for (auto iter = ifaces.begin(); iter != ifaces.end(); iter++) {
                iter->computeQueueSize(collectionInterval);
                iter->computeUtilization(collectionInterval);

                emit(iter->utilization, iter->stats.avgUtilization);
                emit(iter->rate, iter->stats.avgUtilization * iter->stats.lineRate);
                emit(iter->utilizationFiltered, iter->stats.avgUtilization.filtered());
                emit(iter->rateFiltered, iter->stats.avgUtilization.filtered() * iter->stats.lineRate);
                emit(iter->utilizationError, iter->stats.avgUtilization.averageError());
                emit(iter->rateError, iter->stats.avgUtilization.averageError() * iter->stats.lineRate);

                if (iter->interface->getIndex() < 2) {
                    EV_DETAIL << "interface " << iter->interface->getFullPath() << " link utilization " << iter->stats.avgUtilization << " queues:" << endl;

                    for (auto q : iter->stats.queues) {
                        EV_DETAIL << "queue " << q.name << " avg length " << q.avgLength << " avg rate " << q.avgEgressRate << endl;
                    }
                }
            }

            scheduleAt(simTime() + collectionInterval, msg);
            break;
        default:
            const int msgKind = msg->getKind();

            delete msg;

            error("Received message with unexpected message kind: %i", msgKind);
            break;
    }
}

MonitoringCollector::Statistics MonitoringCollector::getStatistics(cComponent * const interface) {
    InterfaceData * const iface = findIface(interface);

    if (iface == nullptr) {
        error("Tried to obtain statistical data for unknown interface %s", interface->getFullPath());
    }

    return iface->stats;
}

MonitoringCollector::InterfaceData * MonitoringCollector::findIface(cComponent *  const src) {
    ASSERT(src);

    // fast path, use cached information
    for (auto locator : locators) {
        if (locator.locator == src) {
            return locator.data;
        }
    }

    // slow path
    auto parent = getParentModule();

    for (auto iter = ifaces.begin(); iter != ifaces.end(); iter++) {
        cComponent * comp = src;

        while (comp != parent) {
            if (comp == iter->interface) {
                locators.push_back(InterfaceLocator{src, &(*iter)});

                return &(*iter);
            } else {
                comp = comp->getParentModule();
            }
        }
    }

    return nullptr;
}


void MonitoringCollector::receiveSignal(cComponent *  const src, const simsignal_t id, const bool value, cObject * const details) {
    error("Received unexpected signal from component: %s", src->getFullPath());
}

void MonitoringCollector::receiveSignal(cComponent *  const src, const simsignal_t id, const long value, cObject * const details) {
    MonitoringCollector::InterfaceData * const iface = findIface(src);

    if (iface != nullptr) {
        if (id == txStateSignal) {
            iface->transmissionEvent(value);
        } else if (id == queueLengthSignal) {
            iface->queueLengthEvent(src, value);
        } else {
            error("Received unexpected signal from component: %s", src->getFullPath());
        }
    }
}

void MonitoringCollector::receiveSignal(cComponent *  const src, const simsignal_t id, const unsigned long value, cObject * const details) {
    error("Received unexpected signal from component: %s", src->getFullPath());
}

void MonitoringCollector::receiveSignal(cComponent *  const src, const simsignal_t id, const double value, cObject * const details)  {
    error("Received unexpected signal from component: %s", src->getFullPath());
}

void MonitoringCollector::receiveSignal(cComponent *  const src, const simsignal_t id, const SimTime& value, cObject * const details) {
    error("Received unexpected signal from component: %s", src->getFullPath());
}

void MonitoringCollector::receiveSignal(cComponent *  const src, const simsignal_t id, const char * const value, cObject * const details) {
    error("Received unexpected signal from component: %s", src->getFullPath());
}

void MonitoringCollector::receiveSignal(cComponent *  const src, const simsignal_t id, cObject * const value, cObject * const details) {
    MonitoringCollector::InterfaceData * const iface = findIface(src);

    if (iface != nullptr) {
        if (id == rcvdPkSignal) {
            cPacket * const msg = dynamic_cast<cPacket *>(value);

            ASSERT(msg);

            iface->ingressEvent(src, msg->getBitLength() + 8 * l2overhead);
        } else if (id == dequeuePkSignal) {
            cPacket * const msg = dynamic_cast<cPacket *>(value);

            ASSERT(msg);

            iface->dequeueEvent(src, msg->getBitLength() + 8 * l2overhead);
        } else if (id == dropPkByQueueSignal) {
            cPacket * const msg = dynamic_cast<cPacket *>(value);

            ASSERT(msg);

            iface->dropEvent(src, msg->getBitLength() + 8 * l2overhead);
        } else {
            error("Received unexpected signal from component: %s", src->getFullPath());
        }
    }
}

} //namespace
