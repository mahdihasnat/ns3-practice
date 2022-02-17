/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "tora-routing-protocol.h"
#include "ns3/core-module.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/node.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("ToraRoutingProtocol");

namespace tora{

NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);

RoutingProtocol::RoutingProtocol():Ipv4RoutingProtocol()
{
	NS_LOG_FUNCTION(this);
}
RoutingProtocol::~RoutingProtocol ()
{
}

TypeId
RoutingProtocol::GetTypeId(void)
{
	static TypeId tid = TypeId("ns3::tora::RoutingProtocol")
			.SetParent<Ipv4RoutingProtocol> ()
			.SetGroupName("Tora")
			.AddConstructor<RoutingProtocol> ()
			.AddAttribute ("UniformRv",
                   "Access to the underlying UniformRandomVariable",
                   StringValue ("ns3::UniformRandomVariable"),
                   MakePointerAccessor (&RoutingProtocol::m_uniformRandomVariable),
                   MakePointerChecker<UniformRandomVariable> ())
			;
	return tid;
}

void 
RoutingProtocol::DoDispose ()
{
	m_ipv4 = 0;
	
	Ipv4RoutingProtocol::DoDispose ();
}

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
	NS_LOG_FUNCTION (this << ipv4);
	m_ipv4 = ipv4;
}

uint32_t
RoutingProtocol::GetRouterId (void) const
{
	return m_ipv4->GetObject<Node>()->GetId();
}

void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit ) const
{

}

int64_t
RoutingProtocol::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformRandomVariable->SetStream (stream);
  return 1;
}


void 
RoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
	NS_LOG_FUNCTION (this << interface << address);
}

void 
RoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
	NS_LOG_FUNCTION (this << interface << address);
}

void 
RoutingProtocol::NotifyInterfaceUp (uint32_t interface)
{
	NS_LOG_FUNCTION (this << interface);
}
void 
RoutingProtocol::NotifyInterfaceDown (uint32_t interface)
{
	NS_LOG_FUNCTION (this << interface);
}

Ptr<Ipv4Route> 
RoutingProtocol:: RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
	NS_LOG_FUNCTION (this << header << (oif ? oif->GetIfIndex () : 0));
	
	return 0;
}

bool
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                   UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION (this << p << header << idev);
	return 0;
}


}

}