/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>,
 *          Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 *          Greeshma Umapathi
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  https://resilinets.org/
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#include "tcp-adaptive-reno.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/rtt-estimator.h"

NS_LOG_COMPONENT_DEFINE("TcpAdaptiveReno");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(TcpAdaptiveReno);

TypeId
TcpAdaptiveReno::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::TcpAdaptiveReno")
            .SetParent<TcpNewReno>()
            .SetGroupName("Internet")
            .AddConstructor<TcpAdaptiveReno>()
            .AddAttribute(
                "FilterType",
                "Use this to choose no filter or Tustin's approximation filter",
                EnumValue(TcpAdaptiveReno::TUSTIN),
                MakeEnumAccessor(&TcpAdaptiveReno::m_fType),
                MakeEnumChecker(TcpAdaptiveReno::NONE, "None", TcpAdaptiveReno::TUSTIN, "Tustin"))
            .AddTraceSource("EstimatedBW",
                            "The estimated bandwidth",
                            MakeTraceSourceAccessor(&TcpAdaptiveReno::m_currentBW),
                            "ns3::TracedValueCallback::Double");
    return tid;
}

TcpAdaptiveReno::TcpAdaptiveReno()
    : TcpWestwoodPlus(),
      m_min_rtt(0),
      m_current_rtt(0),
      m_jPacketLoss_rtt(0),
      m_conj_rtt(0),
      m_prevConjRtt(0),
      m_inc_window(0),
      m_base_window(0),
      m_probe_window(0)
{
    NS_LOG_FUNCTION(this);
}

TcpAdaptiveReno::TcpAdaptiveReno(const TcpAdaptiveReno& socket)
   : TcpWestwoodPlus(socket),
      m_min_rtt(socket.m_min_rtt),
      m_current_rtt(socket.m_current_rtt),
      m_jPacketLoss_rtt(socket.m_jPacketLoss_rtt),
      m_conj_rtt(socket.m_conj_rtt),
      m_prevConjRtt(socket.m_prevConjRtt),
      m_inc_window(socket.m_inc_window),
      m_base_window(socket.m_base_window),
      m_probe_window(socket.m_probe_window)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_LOGIC("Invoked the copy constructor");
}


TcpAdaptiveReno::~TcpAdaptiveReno()
{
}

void
TcpAdaptiveReno::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt)
{
    NS_LOG_FUNCTION(this << tcb << packetsAcked << rtt);

    if (rtt.IsZero())
    {
        NS_LOG_WARN("RTT measured is zero!");
        return;
    }

    m_ackedSegments += packetsAcked;

    // Calculate the minimum RTT and update the current RTT
    if (m_min_rtt.IsZero() || rtt < m_min_rtt)
    {
        m_min_rtt = rtt;
    }

    m_current_rtt = rtt;

    NS_LOG_LOGIC("Minimum RTT: " << m_min_rtt.GetMilliSeconds() << " ms");
    NS_LOG_LOGIC("Current RTT: " << m_current_rtt.GetMilliSeconds() << " ms");

    TcpWestwoodPlus::EstimateBW(rtt, tcb);
}

void
TcpAdaptiveReno::EstimateBW(const Time& rtt, Ptr<TcpSocketState> tcb)
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT(!rtt.IsZero());

    // Calculate the congestion level
    double conjLevel = EstimateCongestionLevel();

    // Calculate the increment window
    EstimateIncWnd(tcb);

    // Calculate the congestion window
    uint32_t cwnd = tcb->m_cWnd.Get();

    if (m_fType == TUSTIN)
    {
        cwnd = static_cast<uint32_t>(cwnd * conjLevel);
    }
    else
    {
        cwnd = static_cast<uint32_t>(cwnd + conjLevel);
    }

    NS_LOG_LOGIC("Congestion level: " << conjLevel);
    NS_LOG_LOGIC("Increment window: " << m_inc_window);
    NS_LOG_LOGIC("Congestion window: " << cwnd);

    // Update the congestion window
    tcb->m_cWnd.Set(cwnd);

    // Schedule the next bandwidth estimation
    m_bwEstimateEvent.Cancel();
    m_bwEstimateEvent = Simulator::Schedule(rtt, &TcpAdaptiveReno::EstimateBW, this, rtt, tcb);
}

double
TcpAdaptiveReno::EstimateCongestionLevel()
{
    NS_LOG_FUNCTION(this);

    // Calculate the congestion level
    double conjLevel = 0.0;

    double m_alpha = 0.85;

    if(m_prevConjRtt < m_min_rtt)
    {
        m_alpha = 0.0;
    }

    double rtt_conj = m_alpha * m_prevConjRtt.GetSeconds() + (1 - m_alpha) * m_jPacketLoss_rtt.GetSeconds();

    m_conj_rtt = Seconds(rtt_conj);

    NS_LOG_LOGIC("conj rtt : " << m_conj_rtt << " ; m_prevConjRtt : " << m_prevConjRtt << " ; j rtt : " << m_jPacketLoss_rtt);

    double numerator_conjLevel = m_current_rtt.GetSeconds() - m_min_rtt.GetSeconds();
    double denominator_conjLevel = rtt_conj - m_min_rtt.GetSeconds();
    conjLevel = numerator_conjLevel / denominator_conjLevel;
    conjLevel = std::min(conjLevel, 1.0);

    return conjLevel;
}

void
TcpAdaptiveReno::EstimateIncWnd(Ptr<TcpSocketState> tcb)
{
    NS_LOG_FUNCTION(this);
    
    double conjLevel = EstimateCongestionLevel();
    
    constexpr double m_scaling_factor = 1000;

    double currentBW = m_currentBW.Get().GetBitRate();

    // m_currentBW; -> already calculated in packetsAck
    double m_max_inc_window =  currentBW / m_scaling_factor ;
    m_max_inc_window = m_max_inc_window * static_cast<double>(tcb->m_segmentSize * tcb->m_segmentSize);


    double alpha = 10; 
    double beta = 2 * m_max_inc_window * ((1/alpha) - ((1/alpha + 1)/(std::exp(alpha)))); 
    double gamma = 1 - (2 * m_max_inc_window * ((1/alpha) - ((1/alpha + 0.5)/(std::exp(alpha)))));

    m_inc_window = (int)((m_max_inc_window / std::exp(alpha * conjLevel)) + (beta * conjLevel) + gamma);

    NS_LOG_LOGIC ("MinRtt: " << m_min_rtt.GetMilliSeconds () << "ms");
    NS_LOG_LOGIC ("ConjRtt: " << m_conj_rtt.GetMilliSeconds () << "ms");
    NS_LOG_LOGIC("maxInc: "<<m_max_inc_window<<"; congestion: "<<conjLevel<<" ; beta: "<<beta<<" ; gamma: "<<gamma<<
    " ; exp(alpha * congestion): "<<std::exp(alpha * conjLevel));
    NS_LOG_LOGIC("m_inc_window: "<<m_inc_window<<" ; prev_wind: "<<tcb->m_cWnd<<" ; new: "<<(m_inc_window / (int)tcb->m_cWnd));

    NS_LOG_LOGIC ("Congestion level: " << conjLevel);
    NS_LOG_LOGIC ("Increment Window: " << m_inc_window);
}

void
TcpAdaptiveReno::CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
    NS_LOG_FUNCTION(this << tcb << segmentsAcked);

    if(segmentsAcked == 0)
    {
        return;
    }

    else if(segmentsAcked > 0)
    {
        // Calculate base window using new reno implementation
        double base_window = static_cast<double> ( tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get();

        base_window = std :: max(base_window, 1.0);

        m_base_window += static_cast<uint32_t> (base_window);

        // Calculate probe window
        m_probe_window = m_probe_window + (m_inc_window / tcb->m_cWnd.Get());
        m_probe_window = std :: max((double)m_probe_window, 0.0);

        // Calculate current window
        tcb->m_cWnd.Set(m_base_window + m_probe_window);

        NS_LOG_LOGIC("Base window: " << m_base_window);
        NS_LOG_LOGIC("Probe window: " << m_probe_window);
        NS_LOG_LOGIC("Current window: " << tcb->m_cWnd.Get());
        
    }
}

uint32_t
TcpAdaptiveReno::GetSsThresh (Ptr<const TcpSocketState> tcb,
                          uint32_t bytesInFlight)
{
     // Update the previous congestion RTT and the RTT of the last packet loss
    m_prevConjRtt = m_conj_rtt;
    m_jPacketLoss_rtt = m_current_rtt;

    NS_LOG_FUNCTION(this << tcb << bytesInFlight);

    uint32_t ssthresh = 0;

    // Calculate the congestion level
    double conjLevel = EstimateCongestionLevel();

    ssthresh = tcb->m_cWnd / (1 + conjLevel);

    ssthresh = std::max(ssthresh, 2 * tcb->m_segmentSize);

    // Reset the congestion window
    m_base_window = ssthresh;
    m_probe_window = 0;

    NS_LOG_LOGIC("Congestion level: " << conjLevel);
    NS_LOG_LOGIC("SSThresh: " << ssthresh);

    return ssthresh;
}

Ptr<TcpCongestionOps>
TcpAdaptiveReno::Fork()
{
    NS_LOG_FUNCTION(this);

    return CreateObject<TcpAdaptiveReno>(*this);
}

}  // namespace ns3
