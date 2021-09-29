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

cplusplus {{
#include "inet/common/INETDefs.h"
#include "inet/common/TLVOption.h"
#include "inet/networklayer/contract/ipv6/IPv6Address.h"
#include "inet/networklayer/ipv6/IPv6Datagram.h"
#include "inet/networklayer/ipv6/IPv6ExtensionHeaders.h"
}}

namespace inet;

class noncobject IPv6Address;

class noncobject IPv6ExtensionHeader;

class noncobject IPv6DestinationOptionsHeader;

class noncobject IPv6HopByHopOptionsHeader;

class noncobject TLVOptionBase;

//cplusplus {{
//#include "SomeMessage_m.h"
//}}
//packet SomeMessage;


enum SerumRecordType {
    MONITORING_TO_DST = 100; // Destination
    MONITORING_HH_APPEND = 101; // Hop by Hop
    MONITORING_HH_INLINE = 102; // Hop by Hop
}

enum SerumDataset {
    DATASET_NONE = 0;
}

class SerumRecord extends TLVOptionBase {
    short dataDesc = -1; 
    
    length = 1; // type plus descriptor in one byte
}
