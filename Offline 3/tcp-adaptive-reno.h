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
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 * and Greeshma Umapathi
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

#ifndef TCP_ADAPTIVERENO_H
#define TCP_ADAPTIVERENO_H

#include "tcp-congestion-ops.h"

#include "ns3/tcp-westwood-plus.h"
#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/tcp-recovery-ops.h"
#include "ns3/traced-value.h"
#include "ns3/sequence-number.h"


namespace ns3
{

class Time;
class EventId;
class TcpHeader;
class Packet;

/**
 * \ingroup congestionOps
 *
 * \brief An implementation of TCP ADAPTIVE RENO.
 *
 *
 * The two main methods in the implementation are the CountAck (const TCPHeader&)
 * and the EstimateBW (int, const, Time). The CountAck method calculates
 * the number of acknowledged segments on the receipt of an ACK.
 * The EstimateBW estimates the bandwidth based on the value returned by CountAck
 * and the sampling interval (last RTT).
 *
 * WARNING: this TCP model lacks validation and regression tests; use with caution.
 */
class TcpAdaptiveReno : public TcpWestwoodPlus
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    TcpAdaptiveReno();
    /**
     * \brief Copy constructor
     * \param sock the object to copy
     */
    TcpAdaptiveReno(const TcpAdaptiveReno& socket); 
    virtual ~TcpAdaptiveReno();

    /**
     * \brief Filter type (None or Tustin)
     */
    enum FilterType
    {
        NONE,
        TUSTIN
    };

    uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight) override;

    void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t packetsAcked, const Time& rtt) override;

    Ptr<TcpCongestionOps> Fork() override;

  private:
    /**
     * Update the total number of acknowledged packets during the current RTT
     *
     * \param [in] acked the number of packets the currently received ACK acknowledges
     */
    void UpdateAckedSegments(int acked);

    // Estimate congestion level
    /**
     * \brief Estimate the congestion level
     * \return the congestion level
     * 
     * The congestion level is estimated as the ratio of the congestion RTT to the current RTT.
     * The congestion RTT is the RTT of the last packet loss.
     * The current RTT is the RTT of the last ACK.
     * 
    */
    double EstimateCongestionLevel();

    // Estimate the increment window
    /**
     * \brief Estimate the increment window
     * \param [in] tcb the socket state
     * 
     * The increment window is estimated as the product of the congestion level and the base window.
     * The base window is the window size at the beginning of the current RTT.
     * 
    */
    void EstimateIncWnd(Ptr<TcpSocketState> tcb);
    

  protected:

    // Congestion window calculation
    uint32_t m_inc_window; //!< The increment window
    uint32_t m_base_window; //!< The base window
    uint32_t m_probe_window; //!< The probe window

    // RTT calculation
    Time m_min_rtt; //!< The minimum RTT
    Time m_current_rtt; //!< The current RTT
    Time m_jPacketLoss_rtt; //!< The RTT of the last packet loss
    Time m_conj_rtt; //!< The congestion RTT (j th event)
    Time m_prevConjRtt; //!< The previous congestion RTT (j-1 th event)
    
    // TracedValue<DataRate> m_currentBW; //!< Current value of the estimated BW
    // DataRate m_lastSampleBW;           //!< Last bandwidth sample
    // DataRate m_lastBW;                 //!< Last bandwidth sample after being filtered
    // FilterType m_fType;                //!< 0 for none, 1 for Tustin

    // uint32_t m_ackedSegments;  //!< The number of segments ACKed between RTTs
    // bool m_IsCount;            //!< Start keeping track of m_ackedSegments for Westwood+ if TRUE
    // EventId m_bwEstimateEvent; //!< The BW estimation event for Westwood+
    // Time m_lastAck;            //!< The last ACK time

    /**
     * \brief Perform congestion avoidance.
     *
     * \param tcb the socket state
     * \param segmentsAcked the number of segments acknowledged
     */
    virtual void CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

    /**
     * Estimate the network's bandwidth
     *
     * \param [in] rtt the RTT estimation.
     * \param [in] tcb the socket state.
     */
    void EstimateBW(const Time& rtt, Ptr<TcpSocketState> tcb);
};

} // namespace ns3

#endif /* TCP_ADAPTIVERENO_H */
