/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "tora-packet.h"

using namespace ns3;
using namespace tora;

//-----------------------------------------------------------------------------
// QryHeader
//-----------------------------------------------------------------------------

NS_OBJECT_ENSURE_REGISTERED (QryHeader);

QryHeader::QryHeader(Ipv4Address dst)
	: m_version(1),
		m_type(1),
		m_reserved(0),
		m_dst(dst)
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
	return 8;
}

void
QryHeader::Serialize (Buffer::Iterator start) const
{
	start.WriteU8 (m_version);
	start.WriteU8 (m_type);
	start.WriteU16 (m_reserved);
	WriteTo(start, m_dst);
}

uint32_t
QryHeader::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_version = start.ReadU8 ();
	m_type = start.ReadU8 ();
	m_reserved = start.ReadU16 ();
	ReadFrom(start, m_dst);
	
	uint32_t dist = i.GetDistanceFrom (start);
	NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void
QryHeader::Print(std::ostream &os) const
{
	os << "QRY destination: ipv4 " << m_dst;
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
	return (m_version == o.m_version
	 && m_type == o.m_type 
	 && m_reserved == o.m_reserved 
	 && m_dst == o.m_dst);
}


