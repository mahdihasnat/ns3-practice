#ifndef TORAPACKET_H
#define TORAPACKET_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"

#include "tora-height.h"

namespace ns3
{

namespace tora
{


enum MessageType
{
  TORATYPE_QRY  = 1,   //!< TORATYPE_QRY
  TORATYPE_UPD  = 2,   //!< TORATYPE_UPD
  TORATYPE_CLR  = 3,   //!< TORATYPE_CLR
  TORATYPE_HLO  = 4,   //!< TORATYPE_HLO
};

/**
* \ingroup aodv
* \brief AODV types
*/
class TypeHeader : public Header
{
public:
  /**
   * constructor
   * \param t the AODV RREQ type
   */
  TypeHeader (MessageType t = TORATYPE_QRY);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  /**
   * \returns the type
   */
  MessageType Get () const
  {
    return m_type;
  }
  /**
   * Check that type if valid
   * \returns true if the type is valid
   */
  bool IsValid () const
  {
    return m_valid;
  }
  /**
   * \brief Comparison operator
   * \param o header to compare
   * \return true if the headers are equal
   */
  bool operator== (TypeHeader const & o) const;
private:
  MessageType m_type; ///< type of the message
  bool m_valid; ///< Indicates if the message is valid
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, TypeHeader const & h);


class HelloHeader : public Header
{
public:
	HelloHeader(Ipv4Address src=Ipv4Address());
	~HelloHeader();

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	uint32_t GetSerializedSize() const;
	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize(Buffer::Iterator start);
	void Print(std::ostream &os) const;

	Ipv4Address GetSrc() const
	{
		return m_src;
	}
	void SetSrc(Ipv4Address src)
	{
		m_src = src;
	}

private:
	Ipv4Address m_src;
};

std::ostream & operator<< (std::ostream & os, HelloHeader const & h);



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
	Ipv4Address m_dst; 		///< destination ip address ,32 bit
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, QryHeader const &);


class UpdHeader:public Header
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

public:
	Ipv4Address m_dst; 		///< destination ip address ,32 bit
	Height m_height;		///< height of the node ,
};

std::ostream & operator<< (std::ostream & os, UpdHeader const &);


} // tora


}// ns3

#endif