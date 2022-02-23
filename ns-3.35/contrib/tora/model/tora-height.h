#ifndef TORAHEIGHT_H
#define TORAHEIGHT_H


#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3
{
namespace tora
{
/**
 * HEIGHT.tau   Time the reference level was created.
 * 
 * HEIGHT.oid   Unique id of the router that created the reference level.
 * 
 * HEIGHT.r     Flag indicating if it is a reflected reference level.
 * 
 * HEIGHT.delta Value used in propagation of a reference level.
 * 
 * HEIGHT.id    Unique id of the router to which the height metric refers.
 * 
 * @brief Height metrics for TORA nodes. according tora rfc draft revision 04
 * @author Mahdi Hasnat Siyam
 * @cite tora rfc https://www.ietf.org/archive/id/draft-ietf-manet-tora-spec-04.txt
 */

class Height : public Header
{
private:
	Time m_tao;
	uint32_t m_oid;
	bool m_r;
	uint32_t m_delta;
	uint32_t m_i;
public:
	Height (Time tao = Time::Max() , uint32_t oid =0, bool r =0, uint32_t delta =0, uint32_t i=0);
	virtual ~Height ();

	static Height GetNullHeight(uint32_t i);
	static Height GetZeroHeight(uint32_t i);

	bool IsNull() const
	{
		return *this == GetNullHeight(m_i);
	}

	bool IsZero() const
	{
		return *this == GetZeroHeight(m_i);
	}


	// Objects
	static TypeId GetTypeId (void);
	TypeId GetInstanceTypeId (void) const;

	// Header
	uint32_t Deserialize (Buffer::Iterator start);
	void Serialize (Buffer::Iterator start) const;
	uint32_t GetSerializedSize (void) const;
	void Print (std::ostream &os) const;


	Time GetTao () const;
	uint32_t GetOid () const;
	bool GetR () const;
	uint32_t GetDelta () const;
	uint32_t GetI () const;

	void SetDelta (uint32_t delta)
	{
		m_delta = delta;
	}
	void SetI (uint32_t i)
	{
		m_i = i;
	}
	void SetR(bool r)
	{
		m_r = r;
	}

	void SetTao (Time tao)
	{
		m_tao = tao;
	}
	void SetOid (uint32_t oid)
	{
		m_oid = oid;
	}


	bool operator <  (const Height &h) const;
	bool operator == (const Height &h) const;
	bool operator != (const Height &h) const;
	bool operator >  (const Height &h) const;
};

std::ostream & operator<< (std::ostream & os, const Height & h);

} // tora
} // ns3


#endif // TORAHEIGHT_H