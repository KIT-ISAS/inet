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

package inet.node.serum;

import inet.networklayer.serum.MonitoringCollector;
import inet.node.ipv6.Router6;

module SerumRouter6 extends Router6
{
    parameters:
        bool monitoring = default(true);
        
        monitoringCollector.enabled = monitoring;
        networkLayer.serumSupportEnabled = monitoring;
        networkLayer.monitoringCollectorModule = default(monitoring ? absPath(".monitoringCollector") : "");
        
    submodules:
        monitoringCollector: MonitoringCollector if monitoring {
            @display("p=174,50");
        }
}
