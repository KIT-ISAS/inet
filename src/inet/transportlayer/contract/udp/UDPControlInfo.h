//
// Copyright (C) 2012 Andras Varga
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

#ifndef __INET_UDPCONTROLINFO_H
#define __INET_UDPCONTROLINFO_H

#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include "inet/networklayer/contract/NetworkOptions.h"

namespace inet {

class UDPSendCommand: public UDPSendCommand_Base {

private:
    void copy(const UDPSendCommand& other);
public:
    UDPSendCommand();
    UDPSendCommand(const UDPSendCommand& other);
    ~UDPSendCommand();

    UDPSendCommand& operator=(const UDPSendCommand& other);
    virtual UDPSendCommand *dup() const;

    virtual const NetworkOptionsPtr& getNetworkOptions() const;
    virtual void setNetworkOptions(const NetworkOptionsPtr& networkOptions);
    virtual NetworkOptionsPtr replaceNetworkOptions(const NetworkOptionsPtr networkOptions);
};

class UDPDataIndication: public UDPDataIndication_Base {

private:
    void copy(const UDPDataIndication& other);
public:
    UDPDataIndication();
    UDPDataIndication(const UDPDataIndication& other);
    ~UDPDataIndication();

    UDPDataIndication& operator=(const UDPDataIndication& other);
    virtual UDPDataIndication *dup() const;

    virtual const NetworkOptionsPtr& getNetworkOptions() const;
    virtual void setNetworkOptions(const NetworkOptionsPtr& networkOptions);
    virtual NetworkOptionsPtr replaceNetworkOptions(const NetworkOptionsPtr networkOptions = nullptr);
};

} // namespace inet

#endif // ifndef __INET_UDPCONTROLINFO_H

