#ifndef TORAHEIGHT_H
#define TORAHEIGHT_H


#include "ns3/core-module.h"

namespace ns3
{
namespace tora
{
/**
 * @brief Height metrics for TORA nodes. according tora rfc draft revision 04
 * @author Mahdi Hasnat Siyam
 * @cite tora rfc https://www.ietf.org/archive/id/draft-ietf-manet-tora-spec-04.txt
 */

class Height : SimpleRefCount<Height>
{
private:
	Time m_tao;
	uint32_t m_oid;
	bool m_r;
	uint32_t m_delta;
	uint32_t m_i;
public:
	Height (Time tao , uint32_t oid , bool r , uint32_t delta , uint32_t i);
	virtual ~Height ();
	Time GetTao () const;
	uint32_t GetOid () const;
	bool GetR () const;
	uint32_t GetDelta () const;
	uint32_t GetI () const;
	bool operator< (const Height &h) const;
	bool operator == (const Height &h) const;
};


} // tora
} // ns3


#endif // TORAHEIGHT_H