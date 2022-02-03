/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "tora-helper.h"
#include "ns3/tora-routing-protocol.h"

#include "ns3/aodv-helper.h"
#include "ns3/aodv-routing-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3 {

ToraHelper::ToraHelper() : 
  Ipv4RoutingHelper ()
{
  m_agentFactory.SetTypeId ("ns3::tora::RoutingProtocol");
}

ToraHelper* 
ToraHelper::Copy (void) const 
{
  return new ToraHelper (*this); 
}

Ptr<Ipv4RoutingProtocol> 
ToraHelper::Create (Ptr<Node> node) const
{
  Ptr<tora::RoutingProtocol> agent = m_agentFactory.Create<tora::RoutingProtocol> ();
  node->AggregateObject (agent);
  return agent;
}

void 
ToraHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

int64_t
ToraHelper::AssignStreams (NodeContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<Node> node;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      NS_ASSERT_MSG (ipv4, "Ipv4 not installed on node");
      Ptr<Ipv4RoutingProtocol> proto = ipv4->GetRoutingProtocol ();
      NS_ASSERT_MSG (proto, "Ipv4 routing not installed on node");
      Ptr<tora::RoutingProtocol> tora = DynamicCast<tora::RoutingProtocol> (proto);
      if (tora)
        {
          currentStream += tora->AssignStreams (currentStream);
          continue;
        }
      // Aodv may also be in a list
      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting> (proto);
      if (list)
        {
          int16_t priority;
          Ptr<Ipv4RoutingProtocol> listProto;
          Ptr<tora::RoutingProtocol> listAodv;
          for (uint32_t i = 0; i < list->GetNRoutingProtocols (); i++)
            {
              listProto = list->GetRoutingProtocol (i, priority);
              listAodv = DynamicCast<tora::RoutingProtocol> (listProto);
              if (listAodv)
                {
                  currentStream += listAodv->AssignStreams (currentStream);
                  break;
                }
            }
        }
    }
  return (currentStream - stream);
}

}

