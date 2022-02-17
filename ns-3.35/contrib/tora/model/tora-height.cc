/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "tora-height.h"
using namespace ns3;
using namespace tora;

NS_LOG_COMPONENT_DEFINE ("ToraHeight");

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