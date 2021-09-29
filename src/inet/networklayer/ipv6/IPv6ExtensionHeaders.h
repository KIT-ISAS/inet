//
// Copyright (C) 2010 Zoltan Bojthe
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

#ifndef __INET_IPV6EXTENSIONHEADERS_H
#define __INET_IPV6EXTENSIONHEADERS_H

#include "inet/networklayer/ipv6/IPv6ExtensionHeaders_m.h"

namespace inet {

class IPv6HopByHopOptionsHeader: public IPv6HopByHopOptionsHeader_Base {
public:
    IPv6HopByHopOptionsHeader() :
            IPv6HopByHopOptionsHeader_Base() { }
    IPv6HopByHopOptionsHeader(const IPv6HopByHopOptionsHeader& other) :
            IPv6HopByHopOptionsHeader_Base(other) { }
    IPv6HopByHopOptionsHeader& operator=(const IPv6HopByHopOptionsHeader& other) {
        if (this == &other)
            return *this;

        IPv6HopByHopOptionsHeader_Base::operator=(other);

        return *this;
    }
    virtual IPv6HopByHopOptionsHeader *dup() const override;

    virtual short getByteLength() const;
};

class INET_API IPv6RoutingHeader : public IPv6RoutingHeader_Base
{
  public:
    IPv6RoutingHeader() : IPv6RoutingHeader_Base() {}
    IPv6RoutingHeader(const IPv6RoutingHeader& other) : IPv6RoutingHeader_Base(other) {}
    IPv6RoutingHeader& operator=(const IPv6RoutingHeader& other) { IPv6RoutingHeader_Base::operator=(other); return *this; }
    virtual IPv6RoutingHeader *dup() const override { return new IPv6RoutingHeader(*this); }
    // ADD CODE HERE to redefine and implement pure virtual functions from IPv6RoutingHeader_Base
    virtual void setAddressArraySize(unsigned int size) override;
};

class IPv6DestinationOptionsHeader: public IPv6DestinationOptionsHeader_Base {
public:
    IPv6DestinationOptionsHeader() :
        IPv6DestinationOptionsHeader_Base() { }
    IPv6DestinationOptionsHeader(const IPv6DestinationOptionsHeader& other) :
        IPv6DestinationOptionsHeader_Base(other) { }
    IPv6DestinationOptionsHeader& operator=(const IPv6DestinationOptionsHeader& other) {
        if (this == &other)
            return *this;

        IPv6DestinationOptionsHeader_Base::operator=(other);

        return *this;
    }
    virtual IPv6DestinationOptionsHeader * dup() const override;

    virtual short getByteLength() const;
};

} // namespace inet

#endif    // __INET_IPV6EXTENSIONHEADERS_H_

