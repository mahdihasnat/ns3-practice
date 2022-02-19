/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef TORAROUTINGPROTOCOL_H
#define TORAROUTINGPROTOCOL_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/random-variable-stream.h"

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

private:
  /**
   * Get unique id for all routiers in network
   * @brief Get the Router Id object
   * 
   * @return uint32_t 
   */
  uint32_t GetRouterId(void) const;
  
  /// IP protocol
  Ptr<Ipv4> m_ipv4;

  //
  Ptr<NetDevice> m_lo;

  /// Provides uniform random variables.
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

};

}

}

#endif // TORAROUTINGPROTOCOL_H