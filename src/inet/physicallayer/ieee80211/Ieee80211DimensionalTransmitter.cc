//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/contract/IRadio.h"
#include "inet/physicallayer/contract/RadioControlInfo_m.h"
#include "inet/physicallayer/ieee80211/Ieee80211PhyMode.h"
#include "inet/physicallayer/ieee80211/Ieee80211DimensionalTransmitter.h"
#include "inet/physicallayer/ieee80211/Ieee80211DimensionalTransmission.h"
#include "inet/physicallayer/ieee80211/Ieee80211PhyMode.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Consts.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mode.h"

namespace inet {

using namespace ieee80211;

namespace physicallayer {

Define_Module(Ieee80211DimensionalTransmitter);

Ieee80211DimensionalTransmitter::Ieee80211DimensionalTransmitter() :
    APSKDimensionalTransmitter(),
    opMode('\0'),
    preambleMode((Ieee80211PreambleMode) - 1),
    phyMode(nullptr),
    modeSet(nullptr),
    mode(nullptr)
{
}

void Ieee80211DimensionalTransmitter::initialize(int stage)
{
    APSKDimensionalTransmitter::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        const char *opModeString = par("opMode");
        if (!strcmp("b", opModeString))
            opMode = 'b';
        else if (!strcmp("g", opModeString))
            opMode = 'g';
        else if (!strcmp("a", opModeString))
            opMode = 'a';
        else if (!strcmp("p", opModeString))
            opMode = 'p';
        else
            throw cRuntimeError("Unknown op mode");
        const char *preambleModeString = par("preambleMode");
        if (!strcmp("short", preambleModeString))
            preambleMode = IEEE80211_PREAMBLE_SHORT;
        else if (!strcmp("long", preambleModeString))
            preambleMode = IEEE80211_PREAMBLE_LONG;
        else
            throw cRuntimeError("Unknown preamble mode");
        carrierFrequency = Hz(CENTER_FREQUENCIES[par("channelNumber")]);
        phyMode = &Ieee80211Mode::getPhyMode(opMode, bitrate.get());
        modeSet = Ieee80211ModeSet::getModeSet(opMode);
        mode = modeSet->getMode(bitrate);
    }
}

const ITransmission *Ieee80211DimensionalTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, simtime_t startTime) const
{
    TransmissionRequest *controlInfo = dynamic_cast<TransmissionRequest *>(macFrame->getControlInfo());
    W transmissionPower = controlInfo && !isNaN(controlInfo->getPower().get()) ? controlInfo->getPower() : power;
    bps transmissionBitrate = controlInfo && !isNaN(controlInfo->getBitrate().get()) ? controlInfo->getBitrate() : bitrate;
    const Ieee80211PhyMode *transmissionPhyMode = transmissionBitrate != bitrate ? &Ieee80211Mode::getPhyMode(opMode, transmissionBitrate.get()) : phyMode;
    const IIeee80211Mode *transmissionMode = transmissionBitrate != bitrate ? modeSet->getMode(transmissionBitrate) : mode;
    const simtime_t duration = transmissionMode->getDuration(macFrame->getBitLength());
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    const ConstMapping *powerMapping = createPowerMapping(startTime, endTime, carrierFrequency, bandwidth, transmissionPower);
    int transmissionHeaderBitLength;
    if (opMode == 'b')
        transmissionHeaderBitLength = HEADER_WITHOUT_PREAMBLE;
    else
        transmissionHeaderBitLength = 24;
    return new Ieee80211DimensionalTransmission(transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, modulation, transmissionHeaderBitLength, macFrame->getBitLength(), carrierFrequency, bandwidth, transmissionBitrate, powerMapping, opMode, preambleMode, transmissionPhyMode, transmissionMode);
}

} // namespace physicallayer

} // namespace inet

