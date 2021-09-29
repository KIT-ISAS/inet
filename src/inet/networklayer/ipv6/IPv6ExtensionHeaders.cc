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

#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"
#include "inet/common/INETUtils.h"

namespace inet {

short IPv6HopByHopOptionsHeader::getByteLength() const {
    return utils::roundUp(2 + tlvOptions.getLength(), 8);
}

IPv6HopByHopOptionsHeader * IPv6HopByHopOptionsHeader::dup() const {
    return new IPv6HopByHopOptionsHeader(*this);
}

short IPv6DestinationOptionsHeader::getByteLength() const {
    return utils::roundUp(2 + tlvOptions.getLength(), 8);
}

IPv6DestinationOptionsHeader * IPv6DestinationOptionsHeader::dup() const {
    return new IPv6DestinationOptionsHeader(*this);
}

void IPv6RoutingHeader::setAddressArraySize(unsigned int size)
{
    IPv6RoutingHeader_Base::setAddressArraySize(size);
    setByteLength(8 + 16 * size);
}

} // namespace inet

