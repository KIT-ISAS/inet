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

#ifndef __INET_MONITORINGCOLLECTOR_H_
#define __INET_MONITORINGCOLLECTOR_H_

#include <omnetpp.h>

#include "PiecewiseSLRFilter.h"

using namespace omnetpp;

namespace inet {

class MonitoringCollector: public cSimpleModule, public cIListener {

public:

    struct QueueStatistics {
        const char * name;
        long size; // queue size in frames. if unknown, set to 0
        PiecewiseSLRFilter avgLength; // average queue length, in frames
        PiecewiseSLRFilter avgIngressRate; // average incoming rate, in bps
        PiecewiseSLRFilter avgEgressRate; // average outbound rate, in bps
        long pktDrop;
        double avgPktSize; // average size of queued packets, in bit
        long seenPkts; // number of packets dequeued
    };
    struct Statistics {
        long lineRate; // link outbound line rate, in bps. if unknown, set to 0
        PiecewiseSLRFilter avgUtilization; // average link utilization, in percent

        std::vector<QueueStatistics> queues;
    };

    MonitoringCollector();
    virtual ~MonitoringCollector();

    Statistics getStatistics(cComponent * const interface);

    virtual void receiveSignal(cComponent *  const src, const simsignal_t id, const bool value, cObject * const details);
    virtual void receiveSignal(cComponent *  const src, const simsignal_t id, const long value, cObject * const details);
    virtual void receiveSignal(cComponent *  const src, const simsignal_t id, const unsigned long value, cObject * const details);
    virtual void receiveSignal(cComponent *  const src, const simsignal_t id, const double value, cObject * const details);
    virtual void receiveSignal(cComponent *  const src, const simsignal_t id, const SimTime& value, cObject * const details);
    virtual void receiveSignal(cComponent *  const src, const simsignal_t id, const char * const value, cObject * const details);
    virtual void receiveSignal(cComponent *  const src, const simsignal_t id, cObject * const value, cObject * const details);
protected:

    // forward declaration
    struct InterfaceData;

    struct QueueData {
        cModule * const queue;

        // only supported for DropTailQueue
        unsigned long queueSize;

        // variables for queue utilization estimation
        std::vector<simtime_t> buckets;
        simtime_t lastQueueSizeEvent = SIMTIME_ZERO;
        unsigned long lastQueueLength = 0;

        // variables for ingress rate estimation
        long bitsReceived = 0;

        // variables for egress rate estimation
        long bitsSent = 0;
        long bitsPending = 0;
        simtime_t lastPendingUpdate = SIMTIME_ZERO;

        // track dropped pkts
        long pktDrop = 0;

        // variables to estimate average packet size
        long pktBits = 0;
        long pktCount = 0;

        simsignal_t lengthSignal;
        simsignal_t ingressRateSignal;
        simsignal_t egressRateSignal;
        simsignal_t dropSignal;
        simsignal_t lengthSignalFiltered;
        simsignal_t ingressRateSignalFiltered;
        simsignal_t egressRateSignalFiltered;
        simsignal_t lengthSignalError;
        simsignal_t ingressRateSignalError;
        simsignal_t egressRateSignalError;

        QueueData(InterfaceData * const iface, cObject * const queue);

        void queueLengthEvent(const long size);
        void ingressEvent(const long msgBits);
        void dequeueEvent(const long msgBits);
        void dropEvent(const long msgBits);
        void computeQueueStatistics(QueueStatistics &stats, const simtime_t interval, const long lineRate);
    };

    struct InterfaceData {
        MonitoringCollector * mc;
        cModule * interface;

        // variables for link utilization estimation
        simtime_t accumulatedSendingTime;
        bool inTransmission;
        simtime_t lastTransmissionStart;

        // queue handling
        std::vector<QueueData> queues;

        Statistics stats;

        std::string signalPrefix;

        simsignal_t utilization;
        simsignal_t rate;
        simsignal_t utilizationFiltered;
        simsignal_t rateFiltered;
        simsignal_t utilizationError;
        simsignal_t rateError;

        InterfaceData(MonitoringCollector * const mc, cModule * const interface, const long lineRate);

        QueueData& getOrAddQueue(cObject * const queue);

        void transmissionEvent(const long status);
        void queueLengthEvent(cObject * const src, const long size);
        void ingressEvent(cObject * const src, const long msgBits);
        void dequeueEvent(cObject * const src, const long msgBits);
        void dropEvent(cObject * const src, const long msgBits);

        void computeQueueSize(const simtime_t interval);
        void computeUtilization(const simtime_t interval);
    };
    struct InterfaceLocator {
        cComponent * locator;
        InterfaceData * data;
    };

    static simsignal_t txStateSignal;
    static simsignal_t queueLengthSignal;
    static simsignal_t dequeuePkSignal;
    static simsignal_t rcvdPkSignal;
    static simsignal_t dropPkByQueueSignal;

    cMessage * tickerEvent;
    simtime_t collectionInterval;
    long l2overhead;
    long filterLength;
    double triggerLength;
    double relativeErrorThreshold;
    std::vector<InterfaceData> ifaces;
    std::vector<InterfaceLocator> locators;

    virtual void initialize();
    virtual void handleMessage(cMessage * const msg);
    InterfaceData * findIface(cComponent *  const src);
};

} //namespace

#endif
