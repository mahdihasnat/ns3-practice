/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "tora-routing-protocol.h"
#include "tora-packet.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/node.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("ToraRoutingProtocol");

namespace tora{


/**
* \ingroup tora
* \brief Tag used by TORA implementation - borrowed from AODV
*/
class DeferredRouteOutputTag : public Tag
{

public:
  /**
   * \brief Constructor
   * \param o the output interface
   */
  DeferredRouteOutputTag (int32_t o = -1) : Tag (),
                                            m_oif (o)
  {
  }

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::tora::DeferredRouteOutputTag")
      .SetParent<Tag> ()
      .SetGroupName ("Tora")
      .AddConstructor<DeferredRouteOutputTag> ()
    ;
    return tid;
  }

  TypeId  GetInstanceTypeId () const
  {
    return GetTypeId ();
  }

  /**
   * \brief Get the output interface
   * \return the output interface
   */
  int32_t GetInterface () const
  {
    return m_oif;
  }

  /**
   * \brief Set the output interface
   * \param oif the output interface
   */
  void SetInterface (int32_t oif)
  {
    m_oif = oif;
  }

  uint32_t GetSerializedSize () const
  {
    return sizeof(int32_t);
  }

  void  Serialize (TagBuffer i) const
  {
    i.WriteU32 (m_oif);
  }

  void  Deserialize (TagBuffer i)
  {
    m_oif = i.ReadU32 ();
  }

  void  Print (std::ostream &os) const
  {
    os << "DeferredRouteOutputTag: output interface = " << m_oif;
  }

private:
  /// Positive if output device is fixed in RouteOutput
  int32_t m_oif;
};

NS_OBJECT_ENSURE_REGISTERED (DeferredRouteOutputTag);

//------------------------------------------------------------------------

NS_OBJECT_ENSURE_REGISTERED(RoutingProtocol);

// https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml#:~:text=138,MANET%20Protocols
uint8_t 
RoutingProtocol::GetIpv4HeaderProtocol(void)
{
	return 138;
}

RoutingProtocol::RoutingProtocol():
				Ipv4RoutingProtocol(),
				m_helloInterval(Seconds(1)),
				m_htimer (Timer::CANCEL_ON_DESTROY),
				m_lastBcastTime (Seconds (0)),
				m_helloRecvTimeout(Seconds(1))
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
			.AddAttribute ("HelloInterval", "HELLO messages emission interval.",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&RoutingProtocol::m_helloInterval),
                   MakeTimeChecker ())
			.AddAttribute ("HelloRecvTimeout", "HELLO messages reception timeout.",
				   TimeValue (Seconds (0.9)),
				   MakeTimeAccessor (&RoutingProtocol::m_helloRecvTimeout),
				   MakeTimeChecker ())
			;
	return tid;
}

void 
RoutingProtocol::DoDispose ()
{
	NS_LOG_FUNCTION(this);
	m_ipv4 = 0;
	m_routeRequiredFlag.clear();
	m_htimer.Cancel();
	Ipv4RoutingProtocol::DoDispose ();
}

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
	NS_LOG_FUNCTION (this << ipv4);
	NS_ASSERT (ipv4 != 0);
  	NS_ASSERT (m_ipv4 == 0);
	
	m_ipv4 = ipv4;
	// Create lo route. It is asserted that the only one interface up for now is loopback
	NS_ASSERT (m_ipv4->GetNInterfaces () == 1 && m_ipv4->GetAddress (0, 0).GetLocal () == Ipv4Address ("127.0.0.1"));
	m_lo = m_ipv4->GetNetDevice (0);
	NS_ASSERT (m_lo != 0);

	Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}

uint32_t
RoutingProtocol::GetRouterId (void) const
{
	return GetMyAddress().Get();
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

	NS_ASSERT_MSG(header.GetProtocol() != GetIpv4HeaderProtocol() , "Manet routing protocol is not allowed to be used as the IPv4 header protocol");

	// Valid route not found, in this case we return loopback.
	// Actual route request will be deferred until packet will be fully formed,
	// routed to loopback, received from loopback and passed to RouteInput (see below)
	uint32_t iif = (oif ? m_ipv4->GetInterfaceForDevice (oif) : -1);
	DeferredRouteOutputTag tag (iif);
	NS_LOG_DEBUG ("Valid Route not found");
	if (!p->PeekPacketTag (tag))
	{
		p->AddPacketTag (tag);
	}
	return LoopbackRoute (header, oif);
}


/**
   * \brief Route an input packet (to be forwarded or locally delivered)
   *
   * This lookup is used in the forwarding process.  The packet is
   * handed over to the Ipv4RoutingProtocol, and will get forwarded onward
   * by one of the callbacks.  The Linux equivalent is ip_route_input().
   * There are four valid outcomes, and a matching callbacks to handle each.
   *
   * \param p received packet
   * \param header input parameter used to form a search key for a route
   * \param idev Pointer to ingress network device
   * \param ucb Callback for the case in which the packet is to be forwarded
   *            as unicast
   * \param mcb Callback for the case in which the packet is to be forwarded
   *            as multicast
   * \param lcb Callback for the case in which the packet is to be locally
   *            delivered
   * \param ecb Callback to call if there is an error in forwarding
   * \returns true if the Ipv4RoutingProtocol takes responsibility for 
   *          forwarding or delivering the packet, false otherwise
   */ 
bool
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                   UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                   LocalDeliverCallback lcb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION (this << p << header << idev);

	NS_ASSERT (m_ipv4 != 0);
	NS_ASSERT (p != 0);
	// Check if input device supports IP
	NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
	// int32_t iif = m_ipv4->GetInterfaceForDevice (idev);
	
	// Ipv4Address dst = header.GetDestination ();
  	// Ipv4Address origin = header.GetSource ();

	// Deferred route request
	if (idev == m_lo)
	{
		DeferredRouteOutputTag tag;
		if (p->PeekPacketTag (tag))
		{
			DeferredRouteOutput (p, header, ucb, ecb);
			return true;
		}
	}

	if(header.GetProtocol() == GetIpv4HeaderProtocol())
	{
		NS_LOG_DEBUG ("Recieved packet with TORA header");
		RecvTora(p,header);
		return true;
	}


	return 0;
}



Ptr<Ipv4Route>
RoutingProtocol::LoopbackRoute (const Ipv4Header & hdr, Ptr<NetDevice> oif) const
{
	NS_LOG_FUNCTION (this << hdr);
	NS_ASSERT (m_lo != 0);
	Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
	rt->SetDestination (hdr.GetDestination ());
	// Comment from AODV:
	//
	// Source address selection here is tricky.  The loopback route is
	// returned when AODV does not have a route; this causes the packet
	// to be looped back and handled (cached) in RouteInput() method
	// while a route is found. However, connection-oriented protocols
	// like TCP need to create an endpoint four-tuple (src, src port,
	// dst, dst port) and create a pseudo-header for checksumming.  So,
	// AODV needs to guess correctly what the eventual source address
	// will be.
	//
	// For single interface, single address nodes, this is not a problem.
	// When there are possibly multiple outgoing interfaces, the policy
	// implemented here is to pick the first available AODV interface.
	// If RouteOutput() caller specified an outgoing interface, that
	// further constrains the selection of source address
	//

	// set source address of ipv4interface
	rt->SetSource(m_ipv4->GetAddress(1,0).GetLocal()); 

	NS_ASSERT_MSG (rt->GetSource () != Ipv4Address (), "Valid AODV source address not found");
	rt->SetGateway (Ipv4Address ("127.0.0.1"));
	rt->SetOutputDevice (m_lo);
	return rt;
}

void
RoutingProtocol:: BroadcastMsg(Ptr<Packet> p)
{
	NS_LOG_FUNCTION (this << p);
	
	// get DownTargetCallback from IpL4Protocol
	Ptr<IpL4Protocol> protocol = m_ipv4->GetProtocol (17);
	NS_ASSERT_MSG (protocol != 0, "No IpL4Protocol installed");
	IpL4Protocol::DownTargetCallback callback = protocol->GetDownTarget ();
	NS_ASSERT_MSG (callback.IsNull() == false, "No down callback found installed");

	// callback signature
	// void (Ptr<Node> node, Ptr<Packet> packet, Ipv4Address source, 
	//        Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route)

	Ipv4Address src = m_ipv4->GetAddress(1,0).GetLocal();
	Ipv4Address dst = m_ipv4->GetAddress(1,0).GetBroadcast();
	uint8_t prot = 138;
	NS_LOG_DEBUG("packet: " << p << " src: " << src << " dst: " << dst << " prot: " << prot);

	callback (p, src, dst, prot, 0);

	NS_UNUSED (callback);
}


void
RoutingProtocol::DeferredRouteOutput (Ptr<const Packet> p, const Ipv4Header & header,
                                      UnicastForwardCallback ucb, ErrorCallback ecb)
{
	NS_LOG_FUNCTION (this << p << header);
	NS_ASSERT (p != 0 && p != Ptr<Packet> ());


	// create new QryHeader  packet
	Ptr<Packet> packet = Create<Packet>();
	
	SendQry(header.GetDestination());
}

bool
RoutingProtocol::IsMyOwnAddress (Ipv4Address src)
{
	NS_LOG_FUNCTION (this << src);
	for(uint i=0;i<m_ipv4->GetNInterfaces();i++)
	{
		if(m_ipv4->GetAddress(i,0).GetLocal()==src)
		{
			return true;
		}
	}
	return false;
}

void
RoutingProtocol::RecvTora (Ptr<const Packet> p,const Ipv4Header & header)
{
	NS_LOG_FUNCTION (this << *p);
	Ptr<Packet> packet = p->Copy ();
	NS_LOG_INFO("packet: " << packet);
	TypeHeader theader;
	packet->RemoveHeader(theader);
	if(theader.IsValid() == false)
	{
		NS_ASSERT_MSG(false, "TORA packet invalid type header");
		return;
	}
	NS_LOG_DEBUG("header: " << theader);

	switch(theader.Get())
	{
		case TORATYPE_QRY:
			{
				QryHeader qheader;
				packet->RemoveHeader(qheader);
				ProcessQry(header.GetSource() , qheader.GetDst());
			}
			break;

		case TORATYPE_HLO:
			{
				HelloHeader hheader;
				packet->RemoveHeader(hheader);
				ProcessHello(hheader.GetSrc());
			}
			break;

		default:
			NS_ASSERT_MSG(false, "TORA packet not handled");
	}
}



void
RoutingProtocol::ProcessQry (Ipv4Address const & src,Ipv4Address const & dst)
{

	ProcessHello(src);
	NS_LOG_FUNCTION (this << dst);
	if(GetRouteRequiredFlag(dst.Get()) == true)
	{
		return ;
	}
}

void
RoutingProtocol::ProcessHello (Ipv4Address const & src)
{
	NS_LOG_FUNCTION (this << src);
	HelloRecvUpdate(src.Get());
}

void
RoutingProtocol:: SendHello ()
{
	NS_LOG_FUNCTION (this);
	Ptr<Packet> p = Create<Packet> ();
	HelloHeader h;
	h.SetSrc (GetMyAddress());
	TypeHeader t (TORATYPE_HLO);
	NS_ASSERT_MSG (t.IsValid(), "TORA Type Header is invalid");
	p->AddHeader (h);
	p->AddHeader (t);
	BroadcastMsg(p);
}

void
RoutingProtocol:: SendQry (Ipv4Address const & dst)
{
	NS_LOG_FUNCTION (this << dst);

	Ptr<Packet> p = Create<Packet> ();

	// create new QryHeader
	QryHeader qryHeader;
	qryHeader.SetDst(dst);
	p->AddHeader(qryHeader);

	TypeHeader typeHeader(TORATYPE_QRY);
	p->AddHeader(typeHeader);
	
	m_lastBcastTime = Simulator::Now();
	BroadcastMsg(p);
}

void
RoutingProtocol:: HelloTimerExpire(void)
{
	NS_LOG_FUNCTION (this);
	Time offset = Time (Seconds (0));
	if (m_lastBcastTime > Time (Seconds (0)))
	{
		offset = Simulator::Now () - m_lastBcastTime;
		NS_LOG_DEBUG ("Hello deferred due to last bcast at:" << m_lastBcastTime);
	}
	else
	{
		SendHello ();
	}
	m_htimer.Cancel ();
	Time diff = m_helloInterval - offset;
	m_htimer.Schedule (std::max (Time (Seconds (0)), diff));
	m_lastBcastTime = Time (Seconds (0));
}

void
RoutingProtocol::DoInitialize (void)
{
	NS_LOG_FUNCTION (this);
	uint32_t startTime;

	m_htimer.SetFunction (&RoutingProtocol::HelloTimerExpire, this);
	startTime = m_uniformRandomVariable->GetInteger (0, 100);
	NS_LOG_DEBUG ("Starting at time " << startTime << "ms");
	m_htimer.Schedule (MilliSeconds (startTime));
    
	Ipv4RoutingProtocol::DoInitialize ();
}

void
RoutingProtocol::Start(void)
{
	NS_LOG_FUNCTION (this);
}


void
RoutingProtocol:: NotifyNeighbourUp(uint32_t id)
{

}
void
RoutingProtocol:: NotifyNeighbourDown(uint32_t id)
{

}

void 
RoutingProtocol::HelloRecvTimerExpire(uint32_t neighbour)
{
	NS_LOG_FUNCTION (this <<" neighbour: " << neighbour);
	NotifyNeighbourDown(neighbour);
	m_helloRecvTimer.erase(neighbour);
}

void
RoutingProtocol:: HelloRecvUpdate(uint32_t neighbour)
{
	NS_LOG_FUNCTION (this <<" neighbour: " << neighbour);
	auto it = m_helloRecvTimer.find(neighbour);
	if(it != m_helloRecvTimer.end())
	{
		it->second.Cancel();
		it->second.Schedule(m_helloRecvTimeout);
		NotifyNeighbourUp(neighbour);
	}
	else 
	{
		m_helloRecvTimer[neighbour] = Timer(Timer::CANCEL_ON_DESTROY);
		m_helloRecvTimer[neighbour].SetFunction(&RoutingProtocol::HelloRecvTimerExpire, this);
		m_helloRecvTimer[neighbour].SetArguments(neighbour);
		m_helloRecvTimer[neighbour].Schedule(m_helloRecvTimeout);
	}

}


} // tora

} // ns3