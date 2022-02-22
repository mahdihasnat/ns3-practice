/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef TORAROUTINGPROTOCOL_H
#define TORAROUTINGPROTOCOL_H

#include "ns3/ipv4-routing-protocol.h"
#include "ns3/random-variable-stream.h"
#include "tora-packet.h"
#include "tora-height.h"

#include "tora-rqueue.h"

namespace ns3 {

namespace tora {

// enum
// {
//   LINKSTAT_UP=1;
//   LINKSTAT_DN=2;
//   LINKSTAT_UN=3;
// }

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
  void SendPacketFromQueue (Ipv4Address dst, Ptr<Ipv4Route> route);
  bool Forwarding (Ptr<const Packet> p, const Ipv4Header & header, 
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
  // if exist , then rreqflag is set, unset otherwise
  std::set<uint32_t > m_routeRequiredFlag;
  bool GetRouteRequiredFlag(uint32_t id) const
  {
    auto it = m_routeRequiredFlag.find(id);
    if(it == m_routeRequiredFlag.end())
      return false;
    return true;
  }
  void SetRouteRequiredFlag(uint32_t id)
  {
    m_routeRequiredFlag.insert(id);
  }
  void UnsetRouteRequiredFlag(uint32_t id)
  {
    m_routeRequiredFlag.erase(id);
  }


  // Current Neighbours
  std::set<uint32_t > m_currentNeighbours;
  bool IsCurrentNeighbour(uint32_t id) const
  {
    auto it = m_currentNeighbours.find(id);
    if(it == m_currentNeighbours.end())
      return false;
    return true;
  }
  void AddCurrentNeighbour(uint32_t id)
  {
    m_currentNeighbours.insert(id);
  }
  void RemoveCurrentNeighbour(uint32_t id)
  {
    m_currentNeighbours.erase(id);
  }
  uint32_t GetNumActive(void) const
  {
    return m_currentNeighbours.size();
  }

// In addition to its own height, each node i maintains a height
// array with an entry H N , for each neighbor j E N i .
// Initially the height of each neighbor is set to NULL,
// HN,,j = (-, -, -, -, j ) . If the destination is a neighbor of
// i (i.e. did E N J node i sets the height entry of the
// destination to ZERO, HN,, did = (0, 0, 0, 0, did).
//
// m_heightNeighbours[dest][neighbour] = height , null if not found
  std::map<uint32_t , std::map<uint32_t , Height >  > m_heightNeighbours;
  Height GetNeighbourHeight(uint32_t dest, uint32_t neighbour) const
  {
    if(dest == neighbour)
      return Height::GetZeroHeight(neighbour);
    auto it = m_heightNeighbours.find(dest);
    if(it == m_heightNeighbours.end())
    {
      return Height::GetNullHeight(neighbour);
    }
    else 
    {
      auto it2 = it->second.find(neighbour);
      if(it2 == it->second.end())
      {
        return Height::GetNullHeight(neighbour);
      }
      else
      {
        return it2->second;
      }
    }
  }
  void SetNeighbourHeight(uint32_t dest, uint32_t neighbour, Height height)
  {
    m_heightNeighbours[dest][neighbour] = height;
  }

// Each node i (other than the destination) maintains its
// height, Hi. Initially the height of each node in the network
// (other than the destination) is set to NULL, Hi = (-, -, -,
// -, i). Subsequently, the height of each node i can be
// modified in accordance with the rules of the protocol
// The height of the destination is always ZERO, Hdid = (0, 0 , 0,
// 0, did), where did is the destination-ID (i.e. the unique ID
// of the destination for which the algorithm is running).
  
  std::map<uint32_t, Height> m_height;
  Height GetHeight(uint32_t dest) const
  {
    auto it = m_height.find(dest);
    if(it == m_height.end())
    {
      if(dest == GetRouterId())
          return Height::GetZeroHeight(dest);
      else
        return Height::GetNullHeight(dest);
    }
    else return it->second;
  }
  void SetHeight(uint32_t dest, Height h)
  {
    m_height[dest] = h;
  }

  // link status info
// Each node i (other than the destination) also
// maintains a link-state array with an entry L S , for each
// link (i, j ) E L , where j E Ni. The state of the links is
// determined by the heights Hi and H N , and is directed
// from the higher node to the lower node. If a neighbor j is
// higher than node i, the link is marked upstream (UP). If a
// neighbor j is lower than node i , the link is marked
// downstream (DN). If the neighbors height entry, H N , j , is
// NULL, the link is marked undirected (UN). Finally, if the
// height of node i is NULL, then any neighbor’s height
// which is not NULL is considered lower, and the
// corresponding link is marked downstream (DN).

  std::map<uint,std::set<uint> > m_DNLinks;
  std::map<uint,std::set<uint> > m_UPLinks;
  bool IsDownLink(uint dest, uint neighbour) const
  {
    auto it = m_DNLinks.find(dest);
    if(it == m_DNLinks.end())
      return false;
    auto it2 = it->second.find(neighbour);
    if(it2 == it->second.end())
      return false;
    return true;
  }
  void SetDownLink(uint dest,uint neighbour)
  {
    NS_ASSERT_MSG(GetHeight(dest) == Height::GetNullHeight(dest)
        or  GetNeighbourHeight(dest, neighbour) < GetHeight(dest),
        "Height of destination is not null and height of neighbour is not null and height of destination is not greater than height of neighbour");
    m_DNLinks[dest].insert(neighbour);
    m_UPLinks[dest].erase(neighbour);
  }
  void UnSetDownLink(uint dest,uint neighbour)
  {
    NS_ASSERT_MSG(IsDownLink(dest, neighbour), "Link is not downlink");
    m_DNLinks[dest].erase(neighbour);
    if(m_DNLinks[dest].empty())
      m_DNLinks.erase(dest);
  }
  uint32_t GetDownLinkCount(uint dest) const
  {
    auto it = m_DNLinks.find(dest);
    if(it == m_DNLinks.end())
      return 0;
    return it->second.size();
  }

  bool IsUpLink(uint dest, uint neighbour) const
  {
    auto it = m_UPLinks.find(dest);
    if(it == m_UPLinks.end())
      return false;
    auto it2 = it->second.find(neighbour);
    if(it2 == it->second.end())
      return false;
    return true;
  }

  void SetUpLink(uint dest,uint neighbour)
  {
    NS_ASSERT_MSG(
        GetHeight(dest) < GetNeighbourHeight(dest, neighbour),
    "Height of destination is not  less than height of neighbour");
    m_UPLinks[dest].insert(neighbour);
    m_DNLinks[dest].erase(neighbour);
  }
  void UnSetUpLink(uint dest,uint neighbour)
  {
    NS_ASSERT_MSG(IsUpLink(dest, neighbour), "Link is not uplink");
    m_UPLinks[dest].erase(neighbour);
    if(m_UPLinks[dest].empty())
      m_UPLinks.erase(dest);
  }

  bool IsUnLink(uint dest, uint neighbour) const
  {
    return (!IsDownLink(dest, neighbour)) && (!IsUpLink(dest, neighbour));
  }

  void RemoveUpDownLink(uint neighbour)
  {
    NS_ASSERT_MSG(false ,"Not implemented");
    bool found = false;
    for(auto it = m_DNLinks.begin(); it != m_DNLinks.end(); ++it)
    {
      if(it->second.find(neighbour) != it->second.end())
      {
        NS_ASSERT_MSG(found == false, "Found more than one link");
        it->second.erase(neighbour);
        found = true;
      }
    }

  }





// last time when neighbour link became active
  std::map<uint32_t , Time > m_timeActive;
  void SetTimeActive(uint32_t neighbour, Time const & time)
  {
    m_timeActive[neighbour] = time;
  }
  Time GetTimeActive(uint32_t neighbour) const
  {
    auto it = m_timeActive.find(neighbour);
    if(it == m_timeActive.end())
    {
      NS_ASSERT(false);
    }
    return it->second;
  }

  // last time when this node sent an Upd packet regarding node dest
  std::map<uint32_t , Time > m_timeSentUpd;
  void SetTimeSentUpd(uint32_t dest, Time const & time)
  {
    m_timeSentUpd[dest] = time;
  }
  Time GetTimeSentUpd(uint32_t dest) const
  {
    auto it = m_timeSentUpd.find(dest);
    if(it == m_timeSentUpd.end())
    {
      NS_ASSERT(false);
    }
    return it->second;
  }


  bool GetRoute(Ipv4Address dst , Ptr<Ipv4Route> route) const
  {
    uint dest = dst.Get();
    auto it = m_DNLinks.find(dest);
    if(it == m_DNLinks.end())
    {
      return false;
    }
    uint32_t nextHop = *it->second.begin();
    route->SetDestination(dst);
    route->SetSource(GetMyAddress());
    route->SetGateway(Ipv4Address(nextHop));
    // get output device from ipv4
    Ptr<NetDevice> device = m_ipv4->GetNetDevice(1);
    route->SetOutputDevice(device);
    return true;
  }
  bool IsRouteValid(Ipv4Address dst) const
  {
    uint dest = dst.Get();
    return GetDownLinkCount(dest) > 0;
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

  uint32_t m_maxQueueLen;        ///< The maximum number of packets that we allow a routing protocol to buffer.
  Time m_maxQueueTime;                 ///< The maximum period of time that a routing protocol is allowed to buffer a packet for.

  /// A "drop-front" queue used by the routing layer to buffer packets to which it does not have a route.
  RequestQueue m_queue;

  void
  SetMaxQueueLen (uint32_t len)
  {
    m_maxQueueLen = len;
    m_queue.SetMaxQueueLen (len);
  }

  void
  SetMaxQueueTime (Time t)
  {
    m_maxQueueTime = t;
    m_queue.SetQueueTimeout (t);
  }

  /**
   * Get the maximum queue length
   * \returns the maximum queue length
   */
  uint32_t GetMaxQueueLen () const
  {
    return m_maxQueueLen;
  }
  



};

}

}

#endif // TORAROUTINGPROTOCOL_H