/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "tora-height.h"
namespace ns3
{

namespace tora
{

NS_LOG_COMPONENT_DEFINE ("ToraHeight");

NS_OBJECT_ENSURE_REGISTERED (Height);
Height::Height (Time tao , uint32_t oid , bool r , uint32_t delta , uint32_t i):
	m_tao (tao),
	m_oid (oid),
	m_r (r),
	m_delta (delta),
	m_i (i)
{
	NS_LOG_FUNCTION (this << tao << oid << r << delta << i);
}

Height::~Height()
{
	NS_LOG_FUNCTION (this);
}


TypeId
Height::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::tora::Height")
		.SetParent<Header> ()
		.SetGroupName ("Tora")
		.AddConstructor<Height> ()
		;
	return tid;
}

TypeId
Height::GetInstanceTypeId () const
{
	return GetTypeId ();
}

uint32_t
Height::GetSerializedSize () const
{
	return 20;
}


void
Height::Serialize (Buffer::Iterator start) const
{
	start.WriteU64 (m_tao.GetInteger ());
	start.WriteU32 (m_oid);
	uint32_t multiplexed = (m_r<<31U)|(m_delta&0x7fffffff);
	start.WriteU32 (multiplexed);
	start.WriteU32 (m_i);
}

uint32_t
Height::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	m_tao = Time::FromInteger (i.ReadU64 (), Time::GetResolution());
	m_oid = i.ReadU32 ();
	uint32_t multiplexed = i.ReadU32 ();
	m_r = (multiplexed>>31U)&0x1;
	m_delta = multiplexed&0x7fffffff;
	m_i = i.ReadU32 ();

	uint32_t dist =  i.GetDistanceFrom (start);
	NS_ASSERT (dist == GetSerializedSize ());
	return dist;
}

void
Height::Print (std::ostream &os) const
{
	if(*this == GetNullHeight(m_i))
	{
		os << "Null";
	}
	else 
		os << "tao=" << m_tao << " oid=" << m_oid << " r=" << m_r << " delta=" << m_delta << " i=" << m_i;
}

std::ostream &
operator<< (std::ostream & os, Height const & h)
{
	h.Print (os);
	return os;
}

Height 
Height::GetNullHeight(uint32_t i)
{
	static Height h (Seconds(-1), 0, false, 0, i);
	return h;
}

Height 
Height::GetZeroHeight(uint32_t i)
{
	static Height h (Seconds (0), 0, false, 0, i);
	return h;
}

Time
Height::GetTao() const
{
	return m_tao;
}

uint32_t
Height::GetOid() const
{
	return m_oid;
}

bool
Height::GetR() const
{
	return m_r;
}

uint32_t
Height::GetDelta() const 
{
	return m_delta;
}

uint32_t
Height::GetI() const
{
	return m_i;
}

bool
Height:: operator<(Height const &h) const
{
	if(IsNull() && h.IsNull())
		return false;
	if(h.IsNull())
		return true;
	if(IsNull())
		return false;
	if(m_tao != h.GetTao())	return m_tao < h.GetTao();
	if(m_oid != h.GetOid())	return m_oid < h.GetOid();
	if(m_r != h.GetR())		return m_r < h.GetR();
	if(m_delta != h.GetDelta())	return m_delta < h.GetDelta();
	return m_i < h.GetI();
}

bool
Height:: operator==(Height const &h) const
{
	return m_tao == h.GetTao() && 
			m_oid == h.GetOid() && 
			m_r == h.GetR() && 
			m_delta == h.GetDelta() &&
	 		m_i == h.GetI();
}

bool
Height:: operator!=(Height const &h) const
{
	return !(*this == h);
}

bool
Height::operator>(Height const &h) const
{
	return h < *this;
}

} // tora
} // ns3