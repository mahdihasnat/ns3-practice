/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef TORAROUTINGPROTOCOL_H
#define TORAROUTINGPROTOCOL_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/random-variable-stream.h"
#include "tora-packet.h"

namespace ns3 {

namespace tora {

/**
 * \ingroup tora
 *
 * \brief TORA routing protocol
 */

class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
    /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /// constructor
  RoutingProtocol ();
  virtual ~RoutingProtocol ();
  virtual void DoDispose ();

  // Inherited from Ipv4RoutingProtocol
  Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                   UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

  Ptr<Ipv4Route>LoopbackRoute (const Ipv4Header & hdr, Ptr<NetDevice> oif) const;
  void DeferredRouteOutput (Ptr<const Packet> p, const Ipv4Header & header,
                                      UnicastForwardCallback ucb, ErrorCallback ecb);
  bool IsMyOwnAddress (Ipv4Address src);
  Ipv4Address GetMyAddress(void) const
  {
    // return address from ipv4 interface
    return m_ipv4->GetAddress(1,0).GetLocal();
  }

protected:
  virtual void DoInitialize (void);

  virtual void RecvTora (Ptr<const Packet> , const Ipv4Header & );
  void ProcessQry (Ipv4Address const & src,Ipv4Address const & dst);
  void ProcessHello (Ipv4Address const & src);
  
  void BroadcastMsg(Ptr<Packet> p);
  void SendHello ();
  void SendQry (Ipv4Address const & dst);

private:

  void Start(void);
  /**
   * Get unique id for all routiers in network
   * @brief Get the Router Id object
   * 
   * @return uint32_t 
   */
  uint32_t GetRouterId(void) const;

  static uint8_t GetIpv4HeaderProtocol(void);
  
  /// IP protocol
  Ptr<Ipv4> m_ipv4;

  // loopback devices
  Ptr<NetDevice> m_lo;

  /// Provides uniform random variables.
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  // RouteRequiredFlag for each node , 
  // if true , then rreqflag is set, unset otherwise
  std::map<uint32_t , bool> m_routeRequiredFlag;
  bool GetRouteRequiredFlag(uint32_t id) const
  {
    auto it = m_routeRequiredFlag.find(id);
    if(it == m_routeRequiredFlag.end())
      return false;
    return it->second;
  }

  // link awareness functionality
  Time m_helloInterval;
  Timer m_htimer;
  Time m_lastBcastTime;
  void HelloTimerExpire(void);
  Time m_helloRecvTimeout;
  std::map<uint32_t ,Timer > m_helloRecvTimer;
  void HelloRecvTimerExpire(uint32_t id);
  void HelloRecvUpdate(uint32_t id);

  void NotifyNeighbourUp(uint32_t id);
  void NotifyNeighbourDown(uint32_t id);

};

}

}

#endif // TORAROUTINGPROTOCOL_H