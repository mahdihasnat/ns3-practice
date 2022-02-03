/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef TORAROUTINGPROTOCOL_H
#define TORAROUTINGPROTOCOL_H

#include "ns3/ipv4-routing-protocol.h"

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


};

}

}

#endif // TORAROUTINGPROTOCOL_H