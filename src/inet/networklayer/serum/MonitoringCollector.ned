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

package inet.networklayer.serum;

//
// MonitoringCollector is responsible for collecting monitoring information
// to be reported by a SERUM monitoring instance.
// Currently, only PPP interfaces are considered for information collection.
//
simple MonitoringCollector
{
    @display("i=block/timer;is=s");
    
    // monitoring data collection will be enabled by this parameter
    bool enabled = default(false);
    // monitoring collection interval
    double collectionInterval @unit(s) = default(0.1s);
    // overhead for L2 header (typically PPP) to be incorporated into queue-based rate estimation
    int l2overhead @unit("Byte") = default(7B);
    // SLR filter length (in samples)
    int filterLength = default(20);
    // SLR trigger window length (fraction of filter length)
    double triggerLength = default(0.3);
    // relative filter threshold (accumulated error compared to maximum possible value, e.g. line rate or buffer size)
    double relativeErrorThreshold = default(0.005);
    
    @signal[ppp*-utilization*](type="double");
    @signal[ppp*-bitrate*](type="double");
    @signal[ppp*-*-qLength*](type="double");
    @signal[ppp*-*-qIngress*](type="double");
    @signal[ppp*-*-qEgress*](type="double");
    @signal[ppp*-*-qDrop*](type="long");
    @statisticTemplate[linkUtilization](record=vector?);
    @statisticTemplate[linkRate](record=vector?);
    @statisticTemplate[queueLength](record=vector?);
    @statisticTemplate[queueIngressRate](record=vector?);
    @statisticTemplate[queueEgressRate](record=vector?);
    @statisticTemplate[queueDrop](record=vector?);
}
