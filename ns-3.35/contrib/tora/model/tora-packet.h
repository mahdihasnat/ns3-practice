#ifndef TORAPACKET_H
#define TORAPACKET_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"

#include "tora-height.h"

namespace ns3
{

namespace tora
{

/**
 *  \ingroup tora
 *  \brief Query packet for TORA routing protocol.
    \verbatim
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|   Version #   |      Type     |          Reserved             |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                    Destination IP Address                     |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    \endverbatim
	Version 
		The TORA version number. This specification documents version 1.

   	Type
   		The TORA packet type. For QRY packet this field is set to 1.
		
	Reserved
    	Field reserved for future use.
	
   	Destination IP Address
    	The IP address for which a route is being requested.

 */

class QryHeader : public Header
{
public:
	/**
	 * constructor
	 * \param dst destination ip address description
	 */
	QryHeader(Ipv4Address dst = Ipv4Address());


	/**
	 * destructor
	 */
	virtual ~QryHeader();
	static TypeId GetTypeId ();
	TypeId GetInstanceTypeId () const;
	uint32_t GetSerializedSize () const;
	void Serialize (Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void Print (std::ostream &os) const;
	bool operator== (QryHeader const & o) const;
	/**
	 * \brief Set the destination address
	 * \param a the destination address
	 */
	void SetDst (Ipv4Address a)
	{
		m_dst = a;
	}
	/**
	 * \brief Get the destination address
	 * \return the destination address
	 */
	Ipv4Address GetDst () const
	{
		return m_dst;
	}

private:
	uint8_t m_version;		///< version number , currently 1
	uint8_t m_type;			///< packet type, 1 for Qry
	uint16_t m_reserved;	///< reserved field , must be zero
	Ipv4Address m_dst; 		///< destination ip address ,32 bit
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, QryHeader const &);


// contains destination ip, and height of node i -which is broadcasting the packet-
class UpdHeader : public Header
{
public:
	UpdHeader(Ipv4Address dst = Ipv4Address(), Height h = Height());
	virtual ~UpdHeader();

	//object
	static TypeId GetTypeId ();
	TypeId GetInstanceTypeId () const;

	//header
	uint32_t GetSerializedSize () const;
	void Serialize (Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void Print (std::ostream &os) const;

private:
	uint8_t m_version;		///< version number , currently 1
	uint8_t m_type;			///< packet type, 2 for Upd
	uint16_t m_reserved;	///< reserved field , must be zero

	Ipv4Address m_dst; 		///< destination ip address ,32 bit
	Height m_height;		///< height of the node ,

};

std::ostream & operator<< (std::ostream & os, UpdHeader const &);

} // tora


}// ns3

#endif