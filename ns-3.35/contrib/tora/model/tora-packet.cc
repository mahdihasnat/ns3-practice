/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "tora-packet.h"

namespace ns3
{

namespace tora
{


//-----------------------------------------------------------------------------
// TypeHeader
//-----------------------------------------------------------------------------

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

TypeHeader::TypeHeader (MessageType t)
  : m_type (t),
    m_valid (true)
{
}

TypeId
TypeHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::tora::TypeHeader")
    .SetParent<Header> ()
    .SetGroupName ("Tora")
    .AddConstructor<TypeHeader> ()
  ;
  return tid;
}

TypeId
TypeHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
TypeHeader::GetSerializedSize () const
{
  return 1;
}

void
TypeHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 ((uint8_t) m_type);
}

uint32_t
TypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t type = i.ReadU8 ();
  m_valid = true;
  switch (type)
    {
	case TORATYPE_QRY:
	case TORATYPE_UPD:
	case TORATYPE_CLR:
      {
        m_type = (MessageType) type;
        break;
      }
    default:
      m_valid = false;
    }
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
TypeHeader::Print (std::ostream &os) const
{
  switch (m_type)
    {
    case TORATYPE_QRY:
      {
        os << "QRY";
        break;
      }
    case TORATYPE_UPD:
      {
        os << "UPD";
        break;
      }
    case TORATYPE_CLR:
      {
        os << "CLR";
        break;
      }
    default:
      os << "UNKNOWN_TYPE";
    }
}

bool
TypeHeader::operator== (TypeHeader const & o) const
{
  return (m_type == o.m_type && m_valid == o.m_valid);
}

std::ostream &
operator<< (std::ostream & os, TypeHeader const & h)
{
  h.Print (os);
  return os;
}




//-----------------------------------------------------------------------------
// QryHeader
//-----------------------------------------------------------------------------

NS_OBJECT_ENSURE_REGISTERED (QryHeader);

QryHeader::QryHeader(Ipv4Address dst)
	:	m_dst(dst)
{
	;
}

QryHeader::~QryHeader()
{
	;
}

TypeId
QryHeader::GetTypeId ()
{
	static TypeId tid = TypeId ("ns3::tora::QryHeader")
		.SetParent<Header> ()
		.SetGroupName ("Tora")
		.AddConstructor<QryHeader> ()
		;
	return tid;
}

TypeId
QryHeader::GetInstanceTypeId () const
{
	return GetTypeId ();
}

uint32_t
QryHeader::GetSerializedSize () const
{
	return 4;
}

void
QryHeader::Serialize (Buffer::Iterator start) const
{
	WriteTo(start, m_dst);
}

uint32_t
QryHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	ReadFrom(start, m_dst);
	
	uint32_t dist = i.GetDistanceFrom (start);
	NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void
QryHeader::Print(std::ostream &os) const
{
	os << "dst: " << m_dst;
}


std::ostream & 
operator<< (std::ostream & os, QryHeader const & h)
{
	h.Print (os);
	return os;
}




bool 
QryHeader:: operator== (QryHeader const & o) const
{
	return (m_dst == o.m_dst);
}


}

}