/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Georgia Tech Research Corporation
 * Copyright (c) 2010 Adrian Sai-wah Tam
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Adrian Sai-wah Tam <adrian.sw.tam@gmail.com>
 */

#define NS_LOG_APPEND_CONTEXT \
		if (m_node) { std::clog << Simulator::Now ().GetSeconds () << " [node " << m_node->GetId () << "] "; }

#include "ns3/abort.h"
#include "ns3/node.h"
#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/log.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6.h"
#include "ns3/ipv4-interface-address.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/simulation-singleton.h"
#include "udp-header.h" // Changed to Add ECMP
#include "tcp-header.h" // Changed to Add ECMP
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/priority-tag.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/trace-source-accessor.h"
#include "tcp-socket-base.h"
#include "tcp-l4-protocol.h"
#include "ipv4-end-point.h"
#include "ipv6-end-point.h"
#include "ipv6-l3-protocol.h"
#include "tcp-header.h"
#include "rtt-estimator.h"
#include <iostream>
#include <iomanip>
#include <string>

#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("TcpSocketBase");

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t TCP_PROT_NUMBER = 6; // Changed to Add ECMP
const uint8_t UDP_PROT_NUMBER = 17; // Changed to Add ECMP


using namespace std;
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpSocketBase);

TypeId
TcpSocketBase::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::TcpSocketBase")
		.SetParent<TcpSocket> ()
		//    .AddAttribute ("TcpState", "State in TCP state machine",
		//                   TypeId::ATTR_GET,
		//                   EnumValue (CLOSED),
		//                   MakeEnumAccessor (&TcpSocketBase::m_state),
		//                   MakeEnumChecker (CLOSED, "Closed"))
		.AddAttribute ("MaxSegLifetime",
				"Maximum segment lifetime in seconds, use for TIME_WAIT state transition to CLOSED state",
				DoubleValue (120), /* RFC793 says MSL=2 minutes*/
				MakeDoubleAccessor (&TcpSocketBase::m_msl),
				MakeDoubleChecker<double> (0))
		.AddAttribute ("MaxWindowSize", "Max size of advertised window",
				UintegerValue (65535),
				MakeUintegerAccessor (&TcpSocketBase::m_maxWinSize),
				MakeUintegerChecker<uint16_t> ())
		.AddAttribute ("IcmpCallback", "Callback invoked whenever an icmp error is received on this socket.",
				CallbackValue (),
				MakeCallbackAccessor (&TcpSocketBase::m_icmpCallback),
				MakeCallbackChecker ())
		.AddAttribute ("IcmpCallback6", "Callback invoked whenever an icmpv6 error is received on this socket.",
				CallbackValue (),
				MakeCallbackAccessor (&TcpSocketBase::m_icmpCallback6),
				MakeCallbackChecker ())
				.AddAttribute ("FlowID", "The priority of the socket",
						UintegerValue (0),
						MakeUintegerAccessor (&TcpSocketBase::m_flowid),
						MakeUintegerChecker<uint32_t> ())
						.AddAttribute ("TenantID", "tenant ",
								UintegerValue (0),
								MakeUintegerAccessor (&TcpSocketBase::m_tenantid),
								MakeUintegerChecker<uint32_t> ())
		.AddAttribute ("IncastServer",
				"Whether the node is Incast Server",
				BooleanValue (false),
				MakeBooleanAccessor (&TcpSocketBase::m_incastserver),
				MakeBooleanChecker ())
		.AddAttribute ("Blocking",
				"Set the socket's send function blocking",
				BooleanValue (false),
				MakeBooleanAccessor (&TcpSocketBase::m_blocking),
				MakeBooleanChecker ())
		.AddAttribute ("MultiQueue",
				"Set the use of Multiple queue in the NIC",
				BooleanValue (false),
				MakeBooleanAccessor (&TcpSocketBase::m_multiqueue),
				MakeBooleanChecker ())
		.AddAttribute ("FCTFileName",
				"Name of the File to dump Flow Completion Time Stats",
				StringValue ("Stats.txt"),
				MakeStringAccessor (&TcpSocketBase::SetFilename),
				MakeStringChecker ())
		.AddAttribute ("TotalTenants",
				"Total number of tenants in the system",
				UintegerValue (1),
				MakeUintegerAccessor (&TcpSocketBase::m_totalTenants),
				MakeUintegerChecker<uint32_t> ())
		.AddTraceSource ("RTO",
				"Retransmission timeout",
				MakeTraceSourceAccessor (&TcpSocketBase::m_rto))
		.AddTraceSource ("RTT",
				"Last RTT sample",
				MakeTraceSourceAccessor (&TcpSocketBase::m_lastRtt))
		.AddTraceSource ("NextTxSequence",
				"Next sequence number to send (SND.NXT)",
				MakeTraceSourceAccessor (&TcpSocketBase::m_nextTxSequence))
		.AddTraceSource ("HighestSequence",
				"Highest sequence number ever sent in socket's life time",
				MakeTraceSourceAccessor (&TcpSocketBase::m_highTxMark))
		.AddTraceSource ("State",
				"TCP state",
				MakeTraceSourceAccessor (&TcpSocketBase::m_state))
		.AddTraceSource ("RWND",
				"Remote side's flow control window",
				MakeTraceSourceAccessor (&TcpSocketBase::m_rWnd))
		;
	return tid;
}

TcpSocketBase::TcpSocketBase (void)
: m_dupAckCount (0),
  m_delAckCount (0),
  m_flowid (-1),
  m_tenantid (-1),
  m_seqnum (0),
  m_windownum (0),
  m_blocked (false),
  m_ackdBytes (0),
  m_endPoint (0),
  m_endPoint6 (0),
  m_node (0),
  m_tcp (0),
  m_rtt (0),
  m_totalmarkedPackets(0),
  m_totalEcnCount(0),
  m_nextTxSequence (0),
  // Change this for non-zero initial sequence number
  m_highTxMark (0),
  m_rxBuffer (0),
  m_txBuffer (0),
  m_state (CLOSED),
  m_errno (ERROR_NOTERROR),
  m_closeNotified (false),
  m_closeOnEmpty (false),
  m_shutdownSend (false),
  m_shutdownRecv (false),
  m_connected (false),
  m_segmentSize (0),
  // For attribute initialization consistency (quiet valgrind)
  m_rWnd (0),
	samplingInterval(Seconds(0.01)),
	m_logger_packet(0)
{
//	  Simulator::Schedule(samplingInterval, &TcpSocketBase::logger, this);

	//	Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
//	x->SetAttribute ("Min", DoubleValue (0));
//	x->SetAttribute ("Max", DoubleValue (100));
//	if (x->GetValue() > 30)
//		m_tenant_id = 1; //(uint32_t) x->GetValue() % m_totalTenants;
//	else
//		m_tenant_id = 2;


	NS_LOG_FUNCTION (this);
}

TcpSocketBase::TcpSocketBase (const TcpSocketBase& sock)
: TcpSocket (sock),
  //copy object::m_tid and socket::callbacks
  m_dupAckCount (sock.m_dupAckCount),
  m_delAckCount (0),
  m_delAckMaxCount (sock.m_delAckMaxCount),
  m_noDelay (sock.m_noDelay),
  m_cnRetries (sock.m_cnRetries),
  m_delAckTimeout (sock.m_delAckTimeout),
  m_persistTimeout (sock.m_persistTimeout),
  m_cnTimeout (sock.m_cnTimeout),
  m_seqnum (0),
  m_windownum (0),
  m_flowid (sock.m_flowid),
  m_tenantid (sock.m_tenantid),
  m_totalTenants (sock.m_totalTenants),
  m_blocking (sock.m_blocking),
  //m_blocked (sock.m_blocked),
  m_blocked (false),
  m_ackdBytes (sock.m_ackdBytes),
  m_endPoint (0),
  m_endPoint6 (0),
  m_node (sock.m_node),
  m_tcp (sock.m_tcp),
  m_rtt (0),
  m_totalmarkedPackets(0),
  m_nextTxSequence (sock.m_nextTxSequence),
  m_highTxMark (sock.m_highTxMark),
  m_rxBuffer (sock.m_rxBuffer),
  m_txBuffer (sock.m_txBuffer),
  m_state (sock.m_state),
  m_errno (sock.m_errno),
  m_closeNotified (sock.m_closeNotified),
  m_closeOnEmpty (sock.m_closeOnEmpty),
  m_shutdownSend (sock.m_shutdownSend),
  m_shutdownRecv (sock.m_shutdownRecv),
  m_connected (sock.m_connected),
  m_msl (sock.m_msl),
  m_segmentSize (sock.m_segmentSize),
  m_maxWinSize (sock.m_maxWinSize),
  m_rWnd (sock.m_rWnd),
	samplingInterval(sock.samplingInterval),
	m_logger_packet(sock.m_logger_packet)
{
	NS_LOG_FUNCTION (this);
	NS_LOG_LOGIC ("Invoked the copy constructor");
	// Copy the rtt estimator if it is set
	if (sock.m_rtt)
	{
		m_rtt = sock.m_rtt->Copy ();
	}
	// Reset all callbacks to null
	Callback<void, Ptr< Socket > > vPS = MakeNullCallback<void, Ptr<Socket> > ();
	Callback<void, Ptr<Socket>, const Address &> vPSA = MakeNullCallback<void, Ptr<Socket>, const Address &> ();
	Callback<void, Ptr<Socket>, uint32_t> vPSUI = MakeNullCallback<void, Ptr<Socket>, uint32_t> ();
	SetConnectCallback (vPS, vPS);
	SetDataSentCallback (vPSUI);
	SetSendCallback (vPSUI);
	SetRecvCallback (vPS);

	  Simulator::Schedule(samplingInterval, &TcpSocketBase::logger, this);

}

TcpSocketBase::~TcpSocketBase (void)
{
	NS_LOG_FUNCTION (this);
	m_node = 0;
	if (m_endPoint != 0)
	{
		NS_ASSERT (m_tcp != 0);
		/*
		 * Upon Bind, an Ipv4Endpoint is allocated and set to m_endPoint, and
		 * DestroyCallback is set to TcpSocketBase::Destroy. If we called
		 * m_tcp->DeAllocate, it wil destroy its Ipv4EndpointDemux::DeAllocate,
		 * which in turn destroys my m_endPoint, and in turn invokes
		 * TcpSocketBase::Destroy to nullify m_node, m_endPoint, and m_tcp.
		 */
		NS_ASSERT (m_endPoint != 0);
		m_tcp->DeAllocate (m_endPoint);
		NS_ASSERT (m_endPoint == 0);
	}
	if (m_endPoint6 != 0)
	{
		NS_ASSERT (m_tcp != 0);
		NS_ASSERT (m_endPoint6 != 0);
		m_tcp->DeAllocate (m_endPoint6);
		NS_ASSERT (m_endPoint6 == 0);
	}
	m_tcp = 0;
	CancelAllTimers ();
}




///////////////////////////////////////////////

class MyTagFlow : public Tag
{
public:
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream &os) const;

	// these are our accessors to our tag structure
	void SetSimpleValue (uint32_t value);
	uint32_t GetSimpleValue (void) const;
private:
	uint32_t m_flowNumber;
};

TypeId
MyTagFlow::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::MyTagFlow")
						.SetParent<Tag> ()
						.AddConstructor<MyTagFlow> ()
						.AddAttribute ("SimpleValue",
								"A simple value",
								EmptyAttributeValue (),
								MakeUintegerAccessor (&MyTagFlow::GetSimpleValue),
								MakeUintegerChecker<uint32_t> ())
								;
	return tid;
}
TypeId
MyTagFlow::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}
uint32_t
MyTagFlow::GetSerializedSize (void) const
{
	return 1;
}
void
MyTagFlow::Serialize (TagBuffer i) const
{
	i.WriteU32 (m_flowNumber);
}
void
MyTagFlow::Deserialize (TagBuffer i)
{
	m_flowNumber = i.ReadU32 ();
}
void
MyTagFlow::Print (std::ostream &os) const
{
	os << "v=" << (uint32_t)m_flowNumber;
}
void
MyTagFlow::SetSimpleValue (uint32_t value)
{
	m_flowNumber = value;
}
uint32_t
MyTagFlow::GetSimpleValue (void) const
{
	return m_flowNumber;
}

/////////////////////////////////////////////////////////



////////////////////////////////////////////////////////

class MyTagQueue : public Tag
{
public:
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream &os) const;

	// these are our accessors to our tag structure
	void SetSimpleValue (uint32_t value);
	uint32_t GetSimpleValue (void) const;
private:
	uint32_t m_flowNumber;
};

TypeId
MyTagQueue::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::MyTagQueue")
 .SetParent<Tag> ()
 .AddConstructor<MyTagQueue> ()
 .AddAttribute ("SimpleValue",
		 "A simple value",
		 EmptyAttributeValue (),
		 MakeUintegerAccessor (&MyTagQueue::GetSimpleValue),
		 MakeUintegerChecker<uint32_t> ())
		 ;
return tid;
}
TypeId
MyTagQueue::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}
uint32_t
MyTagQueue::GetSerializedSize (void) const
{
	return 1;
}
void
MyTagQueue::Serialize (TagBuffer i) const
{
	i.WriteU32 (m_flowNumber);
}
void
MyTagQueue::Deserialize (TagBuffer i)
{
	m_flowNumber = i.ReadU32 ();
}
void
MyTagQueue::Print (std::ostream &os) const
{
	os << "v=" << (uint32_t)m_flowNumber;
}
void
MyTagQueue::SetSimpleValue (uint32_t value)
{
	m_flowNumber = value;
}
uint32_t
MyTagQueue::GetSimpleValue (void) const
{
	return m_flowNumber;
}

///////////////////////////////////////////////////////

class MyTagHop : public Tag
{
public:
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream &os) const;

	// these are our accessors to our tag structure
	void SetSimpleValue (uint32_t value);
	uint32_t GetSimpleValue (void) const;
private:
	uint32_t m_flowNumber;
};

TypeId
MyTagHop::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::MyTagHop")
                                                                                                		 .SetParent<Tag> ()
																										 .AddConstructor<MyTagHop> ()
																										 .AddAttribute ("SimpleValue",
																												 "A simple value",
																												 EmptyAttributeValue (),
																												 MakeUintegerAccessor (&MyTagHop::GetSimpleValue),
																												 MakeUintegerChecker<uint32_t> ())
																												 ;
	return tid;
}
TypeId
MyTagHop::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}
uint32_t
MyTagHop::GetSerializedSize (void) const
{
	return 1;
}
void
MyTagHop::Serialize (TagBuffer i) const
{
	i.WriteU32 (m_flowNumber);
}
void
MyTagHop::Deserialize (TagBuffer i)
{
	m_flowNumber = i.ReadU32 ();
}
void
MyTagHop::Print (std::ostream &os) const
{
	os << "v=" << (uint32_t)m_flowNumber;
}
void
MyTagHop::SetSimpleValue (uint32_t value)
{
	m_flowNumber = value;
}
uint32_t
MyTagHop::GetSimpleValue (void) const
{
	return m_flowNumber;
}



////////////////////////////////////////////////////////



////////////////////////////////////////////////////////

class MyTagTenant : public Tag
{
public:
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (TagBuffer i) const;
	virtual void Deserialize (TagBuffer i);
	virtual void Print (std::ostream &os) const;

	// these are our accessors to our tag structure
	void SetSimpleValue (uint32_t value);
	uint32_t GetSimpleValue (void) const;
private:
	uint32_t m_flowNumber;
};

TypeId
MyTagTenant::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::MyTagTenant")
				.SetParent<Tag> ()
				.AddConstructor<MyTagTenant> ()
				.AddAttribute ("SimpleValue",
						"A simple value",
						EmptyAttributeValue (),
						MakeUintegerAccessor (&MyTagTenant::GetSimpleValue),
						MakeUintegerChecker<uint32_t> ())
						;
	return tid;
}
TypeId
MyTagTenant::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}
uint32_t
MyTagTenant::GetSerializedSize (void) const
{
	return 1;
}
void
MyTagTenant::Serialize (TagBuffer i) const
{
	i.WriteU32 (m_flowNumber);
}
void
MyTagTenant::Deserialize (TagBuffer i)
{
	m_flowNumber = i.ReadU32 ();
}
void
MyTagTenant::Print (std::ostream &os) const
{
	os << "v=" << (uint32_t)m_flowNumber;
}
void
MyTagTenant::SetSimpleValue (uint32_t value)
{
	m_flowNumber = value;
}
uint32_t
MyTagTenant::GetSimpleValue (void) const
{
	return m_flowNumber;
}

///////////////////////////////////////////////////////



/** Associate a node with this TCP socket */
void
TcpSocketBase::SetNode (Ptr<Node> node)
{
	m_node = node;
}

/** Associate the L4 protocol (e.g. mux/demux) with this socket */
void
TcpSocketBase::SetTcp (Ptr<TcpL4Protocol> tcp)
{
	m_tcp = tcp;
}

/** Set an RTT estimator with this socket */
void
TcpSocketBase::SetRtt (Ptr<RttEstimator> rtt)
{
	m_rtt = rtt;
}

uint32_t
TcpSocketBase::GetTupleValue (Ipv4Address saddr, Ipv4Address daddr,uint16_t port, TcpHeader tcpHeader)
{
	// We do not care if this value rolls over
	/* uint32_t tupleValue = saddr.Get () +
                          daddr.Get () +
                          port;
                  tupleValue += tcpHeader.GetSourcePort ();
                  tupleValue += tcpHeader.GetDestinationPort ();
	 */
	if(!m_incastserver) {
		if (++m_node->m_node_lastQ >= 8) m_node->m_node_lastQ = 0;
	}
	else {
		if (++m_node->m_node_lastQ >= 30) m_node->m_node_lastQ = 0;
	}

	return m_node->m_node_lastQ;

}


/** Inherit from Socket class: Returns error code */
enum Socket::SocketErrno
TcpSocketBase::GetErrno (void) const
{
	return m_errno;
}

/** Inherit from Socket class: Returns socket type, NS3_SOCK_STREAM */
enum Socket::SocketType
TcpSocketBase::GetSocketType (void) const
{
	return NS3_SOCK_STREAM;
}

/** Inherit from Socket class: Returns associated node */
Ptr<Node>
TcpSocketBase::GetNode (void) const
{
	NS_LOG_FUNCTION_NOARGS ();
	return m_node;
}

/** Inherit from Socket class: Bind socket to an end-point in TcpL4Protocol */
int
TcpSocketBase::Bind (void)
{
	NS_LOG_FUNCTION (this);
	m_endPoint = m_tcp->Allocate ();
	//  flowstarttime = Simulator::Now();
	if (0 == m_endPoint)
	{
		m_errno = ERROR_ADDRNOTAVAIL;
		return -1;
	}
	m_tcp->m_sockets.push_back (this);
	flowbytes = 0;
	timeouts = -1;
	return SetupCallback ();
}

int
TcpSocketBase::Bind6 (void)
{
	NS_LOG_FUNCTION (this);
	m_endPoint6 = m_tcp->Allocate6 ();
	if (0 == m_endPoint6)
	{
		m_errno = ERROR_ADDRNOTAVAIL;
		return -1;
	}
	m_tcp->m_sockets.push_back (this);
	return SetupCallback ();
}

/** Inherit from Socket class: Bind socket (with specific address) to an end-point in TcpL4Protocol */
int
TcpSocketBase::Bind (const Address &address)
{
	NS_LOG_FUNCTION (this << address);
	if (InetSocketAddress::IsMatchingType (address))
	{
		InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
		Ipv4Address ipv4 = transport.GetIpv4 ();
		uint16_t port = transport.GetPort ();
		if (ipv4 == Ipv4Address::GetAny () && port == 0)
		{
			m_endPoint = m_tcp->Allocate ();
		}
		else if (ipv4 == Ipv4Address::GetAny () && port != 0)
		{
			m_endPoint = m_tcp->Allocate (port);
		}
		else if (ipv4 != Ipv4Address::GetAny () && port == 0)
		{
			m_endPoint = m_tcp->Allocate (ipv4);
		}
		else if (ipv4 != Ipv4Address::GetAny () && port != 0)
		{
			m_endPoint = m_tcp->Allocate (ipv4, port);
		}
		if (0 == m_endPoint)
		{
			m_errno = port ? ERROR_ADDRINUSE : ERROR_ADDRNOTAVAIL;
			return -1;
		}
	}
	else if (Inet6SocketAddress::IsMatchingType (address))
	{
		Inet6SocketAddress transport = Inet6SocketAddress::ConvertFrom (address);
		Ipv6Address ipv6 = transport.GetIpv6 ();
		uint16_t port = transport.GetPort ();
		if (ipv6 == Ipv6Address::GetAny () && port == 0)
		{
			m_endPoint6 = m_tcp->Allocate6 ();
		}
		else if (ipv6 == Ipv6Address::GetAny () && port != 0)
		{
			m_endPoint6 = m_tcp->Allocate6 (port);
		}
		else if (ipv6 != Ipv6Address::GetAny () && port == 0)
		{
			m_endPoint6 = m_tcp->Allocate6 (ipv6);
		}
		else if (ipv6 != Ipv6Address::GetAny () && port != 0)
		{
			m_endPoint6 = m_tcp->Allocate6 (ipv6, port);
		}
		if (0 == m_endPoint6)
		{
			m_errno = port ? ERROR_ADDRINUSE : ERROR_ADDRNOTAVAIL;
			return -1;
		}
	}
	else
	{
		m_errno = ERROR_INVAL;
		return -1;
	}
	m_tcp->m_sockets.push_back (this);
	NS_LOG_LOGIC ("TcpSocketBase " << this << " got an endpoint: " << m_endPoint);

	return SetupCallback ();
}

/** Inherit from Socket class: Initiate connection to a remote address:port */
int
TcpSocketBase::Connect (const Address & address)
{
	NS_LOG_FUNCTION (this << address);

	// If haven't do so, Bind() this socket first
	if (InetSocketAddress::IsMatchingType (address) && m_endPoint6 == 0)
	{
		if (m_endPoint == 0)
		{
			if (Bind () == -1)
			{
				NS_ASSERT (m_endPoint == 0);
				return -1; // Bind() failed
			}
			NS_ASSERT (m_endPoint != 0);
		}
		InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
		m_endPoint->SetPeer (transport.GetIpv4 (), transport.GetPort ());
		m_endPoint6 = 0;

		// Get the appropriate local address and port number from the routing protocol and set up endpoint
		if (SetupEndpoint () != 0)
		{ // Route to destination does not exist
			return -1;
		}
	}
	else if (Inet6SocketAddress::IsMatchingType (address)  && m_endPoint == 0)
	{
		// If we are operating on a v4-mapped address, translate the address to
		// a v4 address and re-call this function
		Inet6SocketAddress transport = Inet6SocketAddress::ConvertFrom (address);
		Ipv6Address v6Addr = transport.GetIpv6 ();
		if (v6Addr.IsIpv4MappedAddress () == true)
		{
			Ipv4Address v4Addr = v6Addr.GetIpv4MappedAddress ();
			return Connect (InetSocketAddress (v4Addr, transport.GetPort ()));
		}

		if (m_endPoint6 == 0)
		{
			if (Bind6 () == -1)
			{
				NS_ASSERT (m_endPoint6 == 0);
				return -1; // Bind() failed
			}
			NS_ASSERT (m_endPoint6 != 0);
		}
		m_endPoint6->SetPeer (v6Addr, transport.GetPort ());
		m_endPoint = 0;

		// Get the appropriate local address and port number from the routing protocol and set up endpoint
		if (SetupEndpoint6 () != 0)
		{ // Route to destination does not exist
			return -1;
		}
	}
	else
	{
		m_errno = ERROR_INVAL;
		return -1;
	}

	// Re-initialize parameters in case this socket is being reused after CLOSE
	m_rtt->Reset ();
	m_cnCount = m_cnRetries;

	// DoConnect() will do state-checking and send a SYN packet
	return DoConnect ();
}

/** Inherit from Socket class: Listen on the endpoint for an incoming connection */
int
TcpSocketBase::Listen (void)
{
	NS_LOG_FUNCTION (this);
	// Linux quits EINVAL if we're not in CLOSED state, so match what they do
	if (m_state != CLOSED)
	{
		m_errno = ERROR_INVAL;
		return -1;
	}
	// In other cases, set the state to LISTEN and done
	NS_LOG_INFO ("CLOSED -> LISTEN");
	m_state = LISTEN;
	return 0;
}

/** Inherit from Socket class: Kill this socket and signal the peer (if any) */
int
TcpSocketBase::Close (void)
{
	NS_LOG_FUNCTION (this);
	// First we check to see if there is any unread rx data
	// Bug number 426 claims we should send reset in this case.
	if (m_rxBuffer.Size () != 0)
	{
		SendRST ();
		return 0;
	}
	if (m_txBuffer.SizeFromSequence (m_nextTxSequence) > 0)
	{ // App close with pending data must wait until all data transmitted
		if (m_closeOnEmpty == false)
		{
			m_closeOnEmpty = true;
			NS_LOG_INFO ("Socket " << this << " deferring close, state " << TcpStateName[m_state]);
		}
		return 0;
	}
	return DoClose ();
}

/** Inherit from Socket class: Signal a termination of send */
int
TcpSocketBase::ShutdownSend (void)
{
	NS_LOG_FUNCTION (this);
	m_shutdownSend = true;
	return 0;
}

/** Inherit from Socket class: Signal a termination of receive */
int
TcpSocketBase::ShutdownRecv (void)
{
	NS_LOG_FUNCTION (this);
	m_shutdownRecv = true;
	return 0;
}

/** Inherit from Socket class: Send a packet. Parameter flags is not used.
    Packet has no TCP header. Invoked by upper-layer application */
int
TcpSocketBase::Send (Ptr<Packet> p, uint32_t flags)
{
	NS_LOG_FUNCTION (this << p);
	NS_ABORT_MSG_IF (flags, "use of flags is not supported in TcpSocketBase::Send()");
	if (m_state == ESTABLISHED || m_state == SYN_SENT || m_state == CLOSE_WAIT)
	{
		// Store the packet into Tx buffer
		if (!m_txBuffer.Add (p))
		{ // TxBuffer overflow, send failed
			m_errno = ERROR_MSGSIZE;
			return -1;
		}
		// Submit the data to lower layers
		NS_LOG_LOGIC ("txBufSize=" << m_txBuffer.Size () << " state " << TcpStateName[m_state]);
		if (m_state == ESTABLISHED || m_state == CLOSE_WAIT)
		{ // Try to send the data out
			SendPendingData (m_connected);
		}
		flowbytes += p->GetSize ();
		return p->GetSize ();
	}
	else
	{ // Connection not established yet
		m_errno = ERROR_NOTCONN;
		return -1; // Send failure
	}
}

/** Inherit from Socket class: In TcpSocketBase, it is same as Send() call */
int
TcpSocketBase::SendTo (Ptr<Packet> p, uint32_t flags, const Address &address)
{
	return Send (p, flags); // SendTo() and Send() are the same
}

/** Inherit from Socket class: Return data to upper-layer application. Parameter flags
    is not used. Data is returned as a packet of size no larger than maxSize */
Ptr<Packet>
TcpSocketBase::Recv (uint32_t maxSize, uint32_t flags)
{
	NS_LOG_FUNCTION (this);
	NS_ABORT_MSG_IF (flags, "use of flags is not supported in TcpSocketBase::Recv()");
	if (m_rxBuffer.Size () == 0 && m_state == CLOSE_WAIT)
	{
		return Create<Packet> (); // Send EOF on connection close
	}
	Ptr<Packet> outPacket = m_rxBuffer.Extract (maxSize);
	if (outPacket != 0 && outPacket->GetSize () != 0)
	{
		SocketAddressTag tag;
		if (m_endPoint != 0)
		{
			tag.SetAddress (InetSocketAddress (m_endPoint->GetPeerAddress (), m_endPoint->GetPeerPort ()));
		}
		else if (m_endPoint6 != 0)
		{
			tag.SetAddress (Inet6SocketAddress (m_endPoint6->GetPeerAddress (), m_endPoint6->GetPeerPort ()));
		}
		outPacket->AddPacketTag (tag);
	}
	return outPacket;
}

/** Inherit from Socket class: Recv and return the remote's address */
Ptr<Packet>
TcpSocketBase::RecvFrom (uint32_t maxSize, uint32_t flags, Address &fromAddress)
{
	NS_LOG_FUNCTION (this << maxSize << flags);
	Ptr<Packet> packet = Recv (maxSize, flags);
	// Null packet means no data to read, and an empty packet indicates EOF
	if (packet != 0 && packet->GetSize () != 0)
	{
		if (m_endPoint != 0)
		{
			fromAddress = InetSocketAddress (m_endPoint->GetPeerAddress (), m_endPoint->GetPeerPort ());
		}
		else if (m_endPoint6 != 0)
		{
			fromAddress = Inet6SocketAddress (m_endPoint6->GetPeerAddress (), m_endPoint6->GetPeerPort ());
		}
		else
		{
			fromAddress = InetSocketAddress (Ipv4Address::GetZero (), 0);
		}
	}
	return packet;
}

/** Inherit from Socket class: Get the max number of bytes an app can send */
uint32_t
TcpSocketBase::GetTxAvailable (void) const
{
	NS_LOG_FUNCTION (this);
	uint32_t avail;
	if (m_blocked)
	{
		//NS_LOG_UNCOND ("Blocked socket so GetTxAvailable = 0");
		avail = 0;
	}
	else
	{
		avail = m_txBuffer.Available ();
	}
	return avail;
}

/** Inherit from Socket class: Get the max number of bytes an app can read */
uint32_t
TcpSocketBase::GetRxAvailable (void) const
{
	NS_LOG_FUNCTION (this);
	return m_rxBuffer.Available ();
}

/** Inherit from Socket class: Return local address:port */
int
TcpSocketBase::GetSockName (Address &address) const
{
	NS_LOG_FUNCTION (this);
	if (m_endPoint != 0)
	{
		address = InetSocketAddress (m_endPoint->GetLocalAddress (), m_endPoint->GetLocalPort ());
	}
	else if (m_endPoint6 != 0)
	{
		address = Inet6SocketAddress (m_endPoint6->GetLocalAddress (), m_endPoint6->GetLocalPort ());
	}
	else
	{ // It is possible to call this method on a socket without a name
		// in which case, behavior is unspecified
		// Should this return an InetSocketAddress or an Inet6SocketAddress?
		address = InetSocketAddress (Ipv4Address::GetZero (), 0);
	}
	return 0;
}

/** Inherit from Socket class: Bind this socket to the specified NetDevice */
void
TcpSocketBase::BindToNetDevice (Ptr<NetDevice> netdevice)
{
	NS_LOG_FUNCTION (netdevice);
	Socket::BindToNetDevice (netdevice); // Includes sanity check
	if (m_endPoint == 0 && m_endPoint6 == 0)
	{
		if (Bind () == -1)
		{
			NS_ASSERT ((m_endPoint == 0 && m_endPoint6 == 0));
			return;
		}
		NS_ASSERT ((m_endPoint != 0 && m_endPoint6 != 0));
	}

	if (m_endPoint != 0)
	{
		m_endPoint->BindToNetDevice (netdevice);
	}
	// No BindToNetDevice() for Ipv6EndPoint
	return;
}

/** Clean up after Bind. Set up callback functions in the end-point. */
int
TcpSocketBase::SetupCallback (void)
{
	NS_LOG_FUNCTION (this);

	if (m_endPoint == 0 && m_endPoint6 == 0)
	{
		return -1;
	}
	if (m_endPoint != 0)
	{
		m_endPoint->SetRxCallback (MakeCallback (&TcpSocketBase::ForwardUp, Ptr<TcpSocketBase> (this)));
		m_endPoint->SetIcmpCallback (MakeCallback (&TcpSocketBase::ForwardIcmp, Ptr<TcpSocketBase> (this)));
		m_endPoint->SetDestroyCallback (MakeCallback (&TcpSocketBase::Destroy, Ptr<TcpSocketBase> (this)));
	}
	if (m_endPoint6 != 0)
	{
		m_endPoint6->SetRxCallback (MakeCallback (&TcpSocketBase::ForwardUp6, Ptr<TcpSocketBase> (this)));
		m_endPoint6->SetIcmpCallback (MakeCallback (&TcpSocketBase::ForwardIcmp6, Ptr<TcpSocketBase> (this)));
		m_endPoint6->SetDestroyCallback (MakeCallback (&TcpSocketBase::Destroy6, Ptr<TcpSocketBase> (this)));
	}

	return 0;
}

/** Perform the real connection tasks: Send SYN if allowed, RST if invalid */
int
TcpSocketBase::DoConnect (void)
{
	NS_LOG_FUNCTION (this);

	// A new connection is allowed only if this socket does not have a connection
	if (m_state == CLOSED || m_state == LISTEN || m_state == SYN_SENT || m_state == LAST_ACK || m_state == CLOSE_WAIT)
	{ // send a SYN packet and change state into SYN_SENT
		SendEmptyPacket (TcpHeader::SYN, 0);
		NS_LOG_INFO (TcpStateName[m_state] << " -> SYN_SENT");
		m_state = SYN_SENT;
	}
	else if (m_state != TIME_WAIT)
	{ // In states SYN_RCVD, ESTABLISHED, FIN_WAIT_1, FIN_WAIT_2, and CLOSING, an connection
		// exists. We send RST, tear down everything, and close this socket.
		SendRST ();
		CloseAndNotify ();
	}
	return 0;
}

/** Do the action to close the socket. Usually send a packet with appropriate
    flags depended on the current m_state. */
int
TcpSocketBase::DoClose (void)
{
	NS_LOG_FUNCTION (this);
	switch (m_state)
	{
	case SYN_RCVD:
	case ESTABLISHED:
		// send FIN to close the peer
		SendEmptyPacket (TcpHeader::FIN, 0);
		NS_LOG_INFO ("ESTABLISHED -> FIN_WAIT_1");
		m_state = FIN_WAIT_1;
		break;
	case CLOSE_WAIT:
		// send FIN+ACK to close the peer
		SendEmptyPacket (TcpHeader::FIN | TcpHeader::ACK , 0);
		NS_LOG_INFO ("CLOSE_WAIT -> LAST_ACK");
		m_state = LAST_ACK;
		break;
	case SYN_SENT:
	case CLOSING:
		// Send RST if application closes in SYN_SENT and CLOSING
		SendRST ();
		CloseAndNotify ();
		break;
	case LISTEN:
	case LAST_ACK:
		// In these three states, move to CLOSED and tear down the end point
		CloseAndNotify ();
		break;
	case CLOSED:
	case FIN_WAIT_1:
	case FIN_WAIT_2:
	case TIME_WAIT:
	default: /* mute compiler */
		// Do nothing in these four states
		break;
	}
	return 0;
}

/** Peacefully close the socket by notifying the upper layer and deallocate end point */
void
TcpSocketBase::CloseAndNotify (void)
{
	NS_LOG_FUNCTION (this);

	if (!m_closeNotified)
	{
		NotifyNormalClose ();
	}
	if (m_state != TIME_WAIT)
	{
		DeallocateEndPoint ();
	}
	m_closeNotified = true;
	NS_LOG_INFO (TcpStateName[m_state] << " -> CLOSED");
	CancelAllTimers ();
	m_state = CLOSED;
}


/** Tell if a sequence number range is out side the range that my rx buffer can
    accpet */
bool
TcpSocketBase::OutOfRange (SequenceNumber32 head, SequenceNumber32 tail) const
{
	if (m_state == LISTEN || m_state == SYN_SENT || m_state == SYN_RCVD)
	{ // Rx buffer in these states are not initialized.
		return false;
	}
	if (m_state == LAST_ACK || m_state == CLOSING || m_state == CLOSE_WAIT)
	{ // In LAST_ACK and CLOSING states, it only wait for an ACK and the
		// sequence number must equals to m_rxBuffer.NextRxSequence ()
		return (m_rxBuffer.NextRxSequence () != head);
	}

	// In all other cases, check if the sequence number is in range
	return (tail < m_rxBuffer.NextRxSequence () || m_rxBuffer.MaxRxSequence () <= head);
}

/** Function called by the L3 protocol when it received a packet to pass on to
    the TCP. This function is registered as the "RxCallback" function in
    SetupCallback(), which invoked by Bind(), and CompleteFork() */
void
TcpSocketBase::ForwardUp (Ptr<Packet> packet, Ipv4Header header, uint16_t port,
		Ptr<Ipv4Interface> incomingInterface)
{
	DoForwardUp (packet, header, port, incomingInterface);
}

void
TcpSocketBase::ForwardUp6 (Ptr<Packet> packet, Ipv6Header header, uint16_t port)
{
	DoForwardUp (packet, header, port);
}

void
TcpSocketBase::ForwardIcmp (Ipv4Address icmpSource, uint8_t icmpTtl,
		uint8_t icmpType, uint8_t icmpCode,
		uint32_t icmpInfo)
{
	NS_LOG_FUNCTION (this << icmpSource << (uint32_t)icmpTtl << (uint32_t)icmpType <<
			(uint32_t)icmpCode << icmpInfo);
	if (!m_icmpCallback.IsNull ())
	{
		m_icmpCallback (icmpSource, icmpTtl, icmpType, icmpCode, icmpInfo);
	}
}

void
TcpSocketBase::ForwardIcmp6 (Ipv6Address icmpSource, uint8_t icmpTtl,
		uint8_t icmpType, uint8_t icmpCode,
		uint32_t icmpInfo)
{
	NS_LOG_FUNCTION (this << icmpSource << (uint32_t)icmpTtl << (uint32_t)icmpType <<
			(uint32_t)icmpCode << icmpInfo);
	if (!m_icmpCallback6.IsNull ())
	{
		m_icmpCallback6 (icmpSource, icmpTtl, icmpType, icmpCode, icmpInfo);
	}
}

/** The real function to handle the incoming packet from lower layers. This is
    wrapped by ForwardUp() so that this function can be overloaded by daughter
    classes. */
void
TcpSocketBase::DoForwardUp (Ptr<Packet> packet, Ipv4Header header, uint16_t port,
		Ptr<Ipv4Interface> incomingInterface)
{
	NS_LOG_LOGIC ("Socket " << this << " forward up " <<
			m_endPoint->GetPeerAddress () <<
			":" << m_endPoint->GetPeerPort () <<
			" to " << m_endPoint->GetLocalAddress () <<
			":" << m_endPoint->GetLocalPort ());
	Address fromAddress = InetSocketAddress (header.GetSource (), port);
	Address toAddress = InetSocketAddress (header.GetDestination (), m_endPoint->GetLocalPort ());

	// Peel off TCP header and do validity checking
	TcpHeader tcpHeader;

	uint8_t ecncount = header.GetTos() & 0x1C;
	ecncount = ecncount >> 2 ;
	tcpHeader.SetEcnCount(ecncount);

	packet->RemoveHeader (tcpHeader);
	if (tcpHeader.GetFlags () & TcpHeader::ACK)
	{
		EstimateRtt (tcpHeader);
	}
	ReadOptions (tcpHeader);

	// Update Rx window size, i.e. the flow control window
	if (m_rWnd.Get () == 0 && tcpHeader.GetWindowSize () != 0)
	{ // persist probes end
		NS_LOG_LOGIC (this << " Leaving zerowindow persist state");
		m_persistEvent.Cancel ();
	}
	m_rWnd = tcpHeader.GetWindowSize ();

	// Discard fully out of range data packets
	if (packet->GetSize ()
			&& OutOfRange (tcpHeader.GetSequenceNumber (), tcpHeader.GetSequenceNumber () + packet->GetSize ()))
	{
		// std::cout<< " NODE: " << m_node->GetId()<< std::endl;
		NS_LOG_LOGIC ("At state " << TcpStateName[m_state] <<
				" received packet of seq [" << tcpHeader.GetSequenceNumber () <<
				":" << tcpHeader.GetSequenceNumber () + packet->GetSize () <<
				") out of range [" << m_rxBuffer.NextRxSequence () << ":" <<
				m_rxBuffer.MaxRxSequence () << ")");
		// Acknowledgement should be sent for all unacceptable packets (RFC793, p.69)
		if (m_state == ESTABLISHED && !(tcpHeader.GetFlags () & TcpHeader::RST))
		{
			SendEmptyPacket (TcpHeader::ACK , tcpHeader.GetEcnCount ());
		}
		return;
	}

	// TCP state machine code in different process functions
	// C.f.: tcp_rcv_state_process() in tcp_input.c in Linux kernel
	switch (m_state)
	{
	case ESTABLISHED:
		ProcessEstablished (packet, tcpHeader);
		break;
	case LISTEN:
		ProcessListen (packet, tcpHeader, fromAddress, toAddress);
		break;
	case TIME_WAIT:
		// Do nothing
		break;
	case CLOSED:
		// Send RST if the incoming packet is not a RST
		if ((tcpHeader.GetFlags () & ~(TcpHeader::PSH | TcpHeader::URG)) != TcpHeader::RST)
		{ // Since m_endPoint is not configured yet, we cannot use SendRST here
			TcpHeader h;
			h.SetFlags (TcpHeader::RST);
			h.SetSequenceNumber (m_nextTxSequence);
			h.SetAckNumber (m_rxBuffer.NextRxSequence ());
			h.SetSourcePort (tcpHeader.GetDestinationPort ());
			h.SetDestinationPort (tcpHeader.GetSourcePort ());
			h.SetWindowSize (AdvertisedWindowSize ());
			AddOptions (h);
			SendPacket (Create<Packet> (), h, header.GetDestination (), header.GetSource (), m_boundnetdevice);
		}
		break;
	case SYN_SENT:
		ProcessSynSent (packet, tcpHeader);
		break;
	case SYN_RCVD:
		ProcessSynRcvd (packet, tcpHeader, fromAddress, toAddress);
		break;
	case FIN_WAIT_1:
	case FIN_WAIT_2:
	case CLOSE_WAIT:
		ProcessWait (packet, tcpHeader);
		break;
	case CLOSING:
		ProcessClosing (packet, tcpHeader);
		break;
	case LAST_ACK:
		ProcessLastAck (packet, tcpHeader);
		break;
	default: // mute compiler
	break;
	}
}

void
TcpSocketBase::DoForwardUp (Ptr<Packet> packet, Ipv6Header header, uint16_t port)
{
	NS_LOG_LOGIC ("Socket " << this << " forward up " <<
			m_endPoint6->GetPeerAddress () <<
			":" << m_endPoint6->GetPeerPort () <<
			" to " << m_endPoint6->GetLocalAddress () <<
			":" << m_endPoint6->GetLocalPort ());
	Address fromAddress = Inet6SocketAddress (header.GetSourceAddress (), port);
	Address toAddress = Inet6SocketAddress (header.GetDestinationAddress (), m_endPoint6->GetLocalPort ());

	// Peel off TCP header and do validity checking
	TcpHeader tcpHeader;
	packet->RemoveHeader (tcpHeader);
	if (tcpHeader.GetFlags () & TcpHeader::ACK)
	{
		EstimateRtt (tcpHeader);
	}
	ReadOptions (tcpHeader);

	// Update Rx window size, i.e. the flow control window
	if (m_rWnd.Get () == 0 && tcpHeader.GetWindowSize () != 0)
	{ // persist probes end
		NS_LOG_LOGIC (this << " Leaving zerowindow persist state");
		m_persistEvent.Cancel ();
	}
	m_rWnd = tcpHeader.GetWindowSize ();

	// Discard fully out of range packets
	if (packet->GetSize ()
			&& OutOfRange (tcpHeader.GetSequenceNumber (), tcpHeader.GetSequenceNumber () + packet->GetSize ()))
	{

		std::cout<< " NODE1: " << m_node->GetId()<< std::endl;
		NS_LOG_LOGIC ("At state " << TcpStateName[m_state] <<
				" received packet of seq [" << tcpHeader.GetSequenceNumber () <<
				":" << tcpHeader.GetSequenceNumber () + packet->GetSize () <<
				") out of range [" << m_rxBuffer.NextRxSequence () << ":" <<
				m_rxBuffer.MaxRxSequence () << ")");
		// Acknowledgement should be sent for all unacceptable packets (RFC793, p.69)
		if (m_state == ESTABLISHED && !(tcpHeader.GetFlags () & TcpHeader::RST))
		{
			SendEmptyPacket (TcpHeader::ACK , tcpHeader.GetEcnCount ());
		}
		return;
	}

	// TCP state machine code in different process functions
	// C.f.: tcp_rcv_state_process() in tcp_input.c in Linux kernel
	switch (m_state)
	{
	case ESTABLISHED:
		ProcessEstablished (packet, tcpHeader);
		break;
	case LISTEN:
		ProcessListen (packet, tcpHeader, fromAddress, toAddress);
		break;
	case TIME_WAIT:
		// Do nothing
		break;
	case CLOSED:
		// Send RST if the incoming packet is not a RST
		if ((tcpHeader.GetFlags () & ~(TcpHeader::PSH | TcpHeader::URG)) != TcpHeader::RST)
		{ // Since m_endPoint is not configured yet, we cannot use SendRST here
			TcpHeader h;
			h.SetFlags (TcpHeader::RST);
			h.SetSequenceNumber (m_nextTxSequence);
			h.SetAckNumber (m_rxBuffer.NextRxSequence ());
			h.SetSourcePort (tcpHeader.GetDestinationPort ());
			h.SetDestinationPort (tcpHeader.GetSourcePort ());
			h.SetWindowSize (AdvertisedWindowSize ());
			AddOptions (h);
			SendPacket (Create<Packet> (), h, header.GetDestinationAddress (), header.GetSourceAddress (), m_boundnetdevice);
		}
		break;
	case SYN_SENT:
		ProcessSynSent (packet, tcpHeader);
		break;
	case SYN_RCVD:
		ProcessSynRcvd (packet, tcpHeader, fromAddress, toAddress);
		break;
	case FIN_WAIT_1:
	case FIN_WAIT_2:
	case CLOSE_WAIT:
		ProcessWait (packet, tcpHeader);
		break;
	case CLOSING:
		ProcessClosing (packet, tcpHeader);
		break;
	case LAST_ACK:
		ProcessLastAck (packet, tcpHeader);
		break;
	default: // mute compiler
	break;
	}
}

/** Received a packet upon ESTABLISHED state. This function is mimicking the
    role of tcp_rcv_established() in tcp_input.c in Linux kernel. */
void
TcpSocketBase::ProcessEstablished (Ptr<Packet> packet, const TcpHeader& tcpHeader)
{
	NS_LOG_FUNCTION (this << tcpHeader);
	// Extract the flags. PSH and URG are not honoured.
	uint8_t tcpflags = tcpHeader.GetFlags () & ~(TcpHeader::PSH | TcpHeader::URG);

	// Different flags are different events
	if (tcpflags == TcpHeader::ACK)
	{
		ReceivedAck (packet, tcpHeader);
	}
	else if (tcpflags == TcpHeader::SYN)
	{ // Received SYN, old NS-3 behaviour is to set state to SYN_RCVD and
		// respond with a SYN+ACK. But it is not a legal state transition as of
		// RFC793. Thus this is ignored.
	}
	else if (tcpflags == (TcpHeader::SYN | TcpHeader::ACK))
	{ // No action for received SYN+ACK, it is probably a duplicated packet
	}
	else if (tcpflags == TcpHeader::FIN || tcpflags == (TcpHeader::FIN | TcpHeader::ACK))
	{ // Received FIN or FIN+ACK, bring down this socket nicely
		PeerClose (packet, tcpHeader);
	}
	else if (tcpflags == 0)
	{ // No flags means there is only data
		ReceivedData (packet, tcpHeader);
		if (m_rxBuffer.Finished ())
		{
			PeerClose (packet, tcpHeader);
		}
	}
	else
	{ // Received RST or the TCP flags is invalid, in either case, terminate this socket
		if (tcpflags != TcpHeader::RST)
		{ // this must be an invalid flag, send reset
			NS_LOG_LOGIC ("Illegal flag " << tcpflags << " received. Reset packet is sent.");
			SendRST ();
		}
		CloseAndNotify ();
	}
}

/** Process the newly received ACK */
void
TcpSocketBase::ReceivedAck (Ptr<Packet> packet, const TcpHeader& tcpHeader)
{
	NS_LOG_FUNCTION (this << tcpHeader);

	// Received ACK. Compare the ACK number against highest unacked seqno
	if (0 == (tcpHeader.GetFlags () & TcpHeader::ACK))
	{ // Ignore if no ACK flag
	}
	else if (tcpHeader.GetAckNumber () < m_txBuffer.HeadSequence ())
	{ // Case 1: Old ACK, ignored.
		NS_LOG_LOGIC ("Ignored ack of " << tcpHeader.GetAckNumber ());
	}
	else if (tcpHeader.GetAckNumber () == m_txBuffer.HeadSequence ())
	{ // Case 2: Potentially a duplicated ACK
		if (tcpHeader.GetAckNumber () < m_nextTxSequence && packet->GetSize() == 0)
		{
			NS_LOG_LOGIC ("Dupack of " << tcpHeader.GetAckNumber ());
			DupAck (tcpHeader, ++m_dupAckCount);
		}
		// otherwise, the ACK is precisely equal to the nextTxSequence
		NS_ASSERT (tcpHeader.GetAckNumber () <= m_nextTxSequence);
	}
	else if (tcpHeader.GetAckNumber () > m_txBuffer.HeadSequence ())
	{ // Case 3: New ACK, reset m_dupAckCount and update m_txBuffer
		NS_LOG_LOGIC ("New ack of " << tcpHeader.GetAckNumber ());
		NewAck (tcpHeader.GetAckNumber ());
		m_dupAckCount = 0;
	}
	// If there is any data piggybacked, store it into m_rxBuffer
	if (packet->GetSize () > 0)
	{
		ReceivedData (packet, tcpHeader);
	}
}

/** Received a packet upon LISTEN state. */
void
TcpSocketBase::ProcessListen (Ptr<Packet> packet, const TcpHeader& tcpHeader,
		const Address& fromAddress, const Address& toAddress)
{
	NS_LOG_FUNCTION (this << tcpHeader);

	// Extract the flags. PSH and URG are not honoured.
	uint8_t tcpflags = tcpHeader.GetFlags () & ~(TcpHeader::PSH | TcpHeader::URG);

	// Fork a socket if received a SYN. Do nothing otherwise.
	// C.f.: the LISTEN part in tcp_v4_do_rcv() in tcp_ipv4.c in Linux kernel
	if (tcpflags != TcpHeader::SYN)
	{
		return;
	}

	// Call socket's notify function to let the server app know we got a SYN
	// If the server app refuses the connection, do nothing
	if (!NotifyConnectionRequest (fromAddress))
	{
		return;
	}
	// Clone the socket, simulate fork
	Ptr<TcpSocketBase> newSock = Fork ();
	NS_LOG_LOGIC ("Cloned a TcpSocketBase " << newSock);
	Simulator::ScheduleNow (&TcpSocketBase::CompleteFork, newSock,
			packet, tcpHeader, fromAddress, toAddress);
}

/** Received a packet upon SYN_SENT */
void
TcpSocketBase::ProcessSynSent (Ptr<Packet> packet, const TcpHeader& tcpHeader)
{
	NS_LOG_FUNCTION (this << tcpHeader);

	// Extract the flags. PSH and URG are not honoured.
	uint8_t tcpflags = tcpHeader.GetFlags () & ~(TcpHeader::PSH | TcpHeader::URG);

	if (tcpflags == 0)
	{ // Bare data, accept it and move to ESTABLISHED state. This is not a normal behaviour. Remove this?
		NS_LOG_INFO ("SYN_SENT -> ESTABLISHED");
		m_state = ESTABLISHED;
		m_connected = true;
		m_retxEvent.Cancel ();
		m_delAckCount = m_delAckMaxCount;
		ReceivedData (packet, tcpHeader);
		Simulator::ScheduleNow (&TcpSocketBase::ConnectionSucceeded, this);
	}
	else if (tcpflags == TcpHeader::ACK)
	{ // Ignore ACK in SYN_SENT
	}
	else if (tcpflags == TcpHeader::SYN)
	{ // Received SYN, move to SYN_RCVD state and respond with SYN+ACK
		NS_LOG_INFO ("SYN_SENT -> SYN_RCVD");
		m_state = SYN_RCVD;
		m_cnCount = m_cnRetries;
		m_rxBuffer.SetNextRxSequence (tcpHeader.GetSequenceNumber () + SequenceNumber32 (1));
		SendEmptyPacket (TcpHeader::SYN | TcpHeader::ACK , tcpHeader.GetEcnCount());
	}
	else if (tcpflags == (TcpHeader::SYN | TcpHeader::ACK)
			&& m_nextTxSequence + SequenceNumber32 (1) == tcpHeader.GetAckNumber ())
	{ // Handshake completed
		NS_LOG_INFO ("SYN_SENT -> ESTABLISHED");
		m_state = ESTABLISHED;
		m_connected = true;
		flowstarttime = Simulator::Now();
		m_retxEvent.Cancel ();
		m_rxBuffer.SetNextRxSequence (tcpHeader.GetSequenceNumber () + SequenceNumber32 (1));
		m_highTxMark = ++m_nextTxSequence;
		m_txBuffer.SetHeadSequence (m_nextTxSequence);
		SendEmptyPacket (TcpHeader::ACK, tcpHeader.GetEcnCount());
		SendPendingData (m_connected);
		Simulator::ScheduleNow (&TcpSocketBase::ConnectionSucceeded, this);
		// Always respond to first data packet to speed up the connection.
		// Remove to get the behaviour of old NS-3 code.
		m_delAckCount = m_delAckMaxCount;
	}
	else
	{ // Other in-sequence input
		if (tcpflags != TcpHeader::RST)
		{ // When (1) rx of FIN+ACK; (2) rx of FIN; (3) rx of bad flags
			NS_LOG_LOGIC ("Illegal flag " << std::hex << static_cast<uint32_t> (tcpflags) << std::dec << " received. Reset packet is sent.");
			SendRST ();
		}
		CloseAndNotify ();
	}
}

/** Received a packet upon SYN_RCVD */
void
TcpSocketBase::ProcessSynRcvd (Ptr<Packet> packet, const TcpHeader& tcpHeader,
		const Address& fromAddress, const Address& toAddress)
{
	NS_LOG_FUNCTION (this << tcpHeader);

	// Extract the flags. PSH and URG are not honoured.
	uint8_t tcpflags = tcpHeader.GetFlags () & ~(TcpHeader::PSH | TcpHeader::URG);

	if (tcpflags == 0
			|| (tcpflags == TcpHeader::ACK
					&& m_nextTxSequence + SequenceNumber32 (1) == tcpHeader.GetAckNumber ()))
	{ // If it is bare data, accept it and move to ESTABLISHED state. This is
		// possibly due to ACK lost in 3WHS. If in-sequence ACK is received, the
		// handshake is completed nicely.
		NS_LOG_INFO ("SYN_RCVD -> ESTABLISHED");
		m_state = ESTABLISHED;
		m_connected = true;
		m_retxEvent.Cancel ();
		m_highTxMark = ++m_nextTxSequence;
		m_txBuffer.SetHeadSequence (m_nextTxSequence);
		if (m_endPoint)
		{
			m_endPoint->SetPeer (InetSocketAddress::ConvertFrom (fromAddress).GetIpv4 (),
					InetSocketAddress::ConvertFrom (fromAddress).GetPort ());
		}
		else if (m_endPoint6)
		{
			m_endPoint6->SetPeer (Inet6SocketAddress::ConvertFrom (fromAddress).GetIpv6 (),
					Inet6SocketAddress::ConvertFrom (fromAddress).GetPort ());
		}
		// Always respond to first data packet to speed up the connection.
		// Remove to get the behaviour of old NS-3 code.
		m_delAckCount = m_delAckMaxCount;
		ReceivedAck (packet, tcpHeader);
		NotifyNewConnectionCreated (this, fromAddress);
		// As this connection is established, the socket is available to send data now
		if (GetTxAvailable () > 0)
		{
			NotifySend (GetTxAvailable ());
		}
	}
	else if (tcpflags == TcpHeader::SYN)
	{ // Probably the peer lost my SYN+ACK
		m_rxBuffer.SetNextRxSequence (tcpHeader.GetSequenceNumber () + SequenceNumber32 (1));
		SendEmptyPacket (TcpHeader::SYN | TcpHeader::ACK , tcpHeader.GetEcnCount ());
	}
	else if (tcpflags == (TcpHeader::FIN | TcpHeader::ACK))
	{
		if (tcpHeader.GetSequenceNumber () == m_rxBuffer.NextRxSequence ())
		{ // In-sequence FIN before connection complete. Set up connection and close.
			m_connected = true;
			m_retxEvent.Cancel ();
			m_highTxMark = ++m_nextTxSequence;
			m_txBuffer.SetHeadSequence (m_nextTxSequence);
			if (m_endPoint)
			{
				m_endPoint->SetPeer (InetSocketAddress::ConvertFrom (fromAddress).GetIpv4 (),
						InetSocketAddress::ConvertFrom (fromAddress).GetPort ());
			}
			else if (m_endPoint6)
			{
				m_endPoint6->SetPeer (Inet6SocketAddress::ConvertFrom (fromAddress).GetIpv6 (),
						Inet6SocketAddress::ConvertFrom (fromAddress).GetPort ());
			}
			PeerClose (packet, tcpHeader);
		}
	}
	else
	{ // Other in-sequence input
		if (tcpflags != TcpHeader::RST)
		{ // When (1) rx of SYN+ACK; (2) rx of FIN; (3) rx of bad flags
			NS_LOG_LOGIC ("Illegal flag " << tcpflags << " received. Reset packet is sent.");
			if (m_endPoint)
			{
				m_endPoint->SetPeer (InetSocketAddress::ConvertFrom (fromAddress).GetIpv4 (),
						InetSocketAddress::ConvertFrom (fromAddress).GetPort ());
			}
			else if (m_endPoint6)
			{
				m_endPoint6->SetPeer (Inet6SocketAddress::ConvertFrom (fromAddress).GetIpv6 (),
						Inet6SocketAddress::ConvertFrom (fromAddress).GetPort ());
			}
			SendRST ();
		}
		CloseAndNotify ();
	}
}

/** Received a packet upon CLOSE_WAIT, FIN_WAIT_1, or FIN_WAIT_2 states */
void
TcpSocketBase::ProcessWait (Ptr<Packet> packet, const TcpHeader& tcpHeader)
{
	NS_LOG_FUNCTION (this << tcpHeader);

	// Extract the flags. PSH and URG are not honoured.
	uint8_t tcpflags = tcpHeader.GetFlags () & ~(TcpHeader::PSH | TcpHeader::URG);

	if (packet->GetSize () > 0 && tcpflags != TcpHeader::ACK)
	{ // Bare data, accept it
		ReceivedData (packet, tcpHeader);
	}
	else if (tcpflags == TcpHeader::ACK)
	{ // Process the ACK, and if in FIN_WAIT_1, conditionally move to FIN_WAIT_2
		ReceivedAck (packet, tcpHeader);
		if (m_state == FIN_WAIT_1 && m_txBuffer.Size () == 0
				&& tcpHeader.GetAckNumber () == m_highTxMark + SequenceNumber32 (1))
		{ // This ACK corresponds to the FIN sent
			NS_LOG_INFO ("FIN_WAIT_1 -> FIN_WAIT_2");
			m_state = FIN_WAIT_2;
		}
	}
	else if (tcpflags == TcpHeader::FIN || tcpflags == (TcpHeader::FIN | TcpHeader::ACK))
	{ // Got FIN, respond with ACK and move to next state
		if (tcpflags & TcpHeader::ACK)
		{ // Process the ACK first
			ReceivedAck (packet, tcpHeader);
		}
		m_rxBuffer.SetFinSequence (tcpHeader.GetSequenceNumber ());
	}
	else if (tcpflags == TcpHeader::SYN || tcpflags == (TcpHeader::SYN | TcpHeader::ACK))
	{ // Duplicated SYN or SYN+ACK, possibly due to spurious retransmission
		return;
	}
	else
	{ // This is a RST or bad flags
		if (tcpflags != TcpHeader::RST)
		{
			NS_LOG_LOGIC ("Illegal flag " << tcpflags << " received. Reset packet is sent.");
			SendRST ();
		}
		CloseAndNotify ();
		return;
	}

	// Check if the close responder sent an in-sequence FIN, if so, respond ACK
	if ((m_state == FIN_WAIT_1 || m_state == FIN_WAIT_2) && m_rxBuffer.Finished ())
	{
		if (m_state == FIN_WAIT_1)
		{
			NS_LOG_INFO ("FIN_WAIT_1 -> CLOSING");
			m_state = CLOSING;
			if (m_txBuffer.Size () == 0
					&& tcpHeader.GetAckNumber () == m_highTxMark + SequenceNumber32 (1))
			{ // This ACK corresponds to the FIN sent
				TimeWait ();
			}
		}
		else if (m_state == FIN_WAIT_2)
		{
			TimeWait ();
		}
		SendEmptyPacket (TcpHeader::ACK, tcpHeader.GetEcnCount ());
		if (!m_shutdownRecv)
		{
			NotifyDataRecv ();
		}
	}
}

/** Received a packet upon CLOSING */
void
TcpSocketBase::ProcessClosing (Ptr<Packet> packet, const TcpHeader& tcpHeader)
{
	NS_LOG_FUNCTION (this << tcpHeader);

	// Extract the flags. PSH and URG are not honoured.
	uint8_t tcpflags = tcpHeader.GetFlags () & ~(TcpHeader::PSH | TcpHeader::URG);

	if (tcpflags == TcpHeader::ACK)
	{
		if (tcpHeader.GetSequenceNumber () == m_rxBuffer.NextRxSequence ())
		{ // This ACK corresponds to the FIN sent
			TimeWait ();
		}
	}
	else
	{ // CLOSING state means simultaneous close, i.e. no one is sending data to
		// anyone. If anything other than ACK is received, respond with a reset.
		if (tcpflags == TcpHeader::FIN || tcpflags == (TcpHeader::FIN | TcpHeader::ACK))
		{ // FIN from the peer as well. We can close immediately.
			SendEmptyPacket (TcpHeader::ACK , tcpHeader.GetEcnCount ());
		}
		else if (tcpflags != TcpHeader::RST)
		{ // Receive of SYN or SYN+ACK or bad flags or pure data
			NS_LOG_LOGIC ("Illegal flag " << tcpflags << " received. Reset packet is sent.");
			SendRST ();
		}
		CloseAndNotify ();
	}
}

/** Received a packet upon LAST_ACK */
void
TcpSocketBase::ProcessLastAck (Ptr<Packet> packet, const TcpHeader& tcpHeader)
{
	NS_LOG_FUNCTION (this << tcpHeader);

	// Extract the flags. PSH and URG are not honoured.
	uint8_t tcpflags = tcpHeader.GetFlags () & ~(TcpHeader::PSH | TcpHeader::URG);

	if (tcpflags == 0)
	{
		ReceivedData (packet, tcpHeader);
	}
	else if (tcpflags == TcpHeader::ACK)
	{
		if (tcpHeader.GetSequenceNumber () == m_rxBuffer.NextRxSequence ())
		{ // This ACK corresponds to the FIN sent. This socket closed peacefully.
			CloseAndNotify ();
		}
	}
	else if (tcpflags == TcpHeader::FIN)
	{ // Received FIN again, the peer probably lost the FIN+ACK
		SendEmptyPacket (TcpHeader::FIN | TcpHeader::ACK , tcpHeader.GetEcnCount ());
	}
	else if (tcpflags == (TcpHeader::FIN | TcpHeader::ACK) || tcpflags == TcpHeader::RST)
	{
		CloseAndNotify ();
	}
	else
	{ // Received a SYN or SYN+ACK or bad flags
		NS_LOG_LOGIC ("Illegal flag " << tcpflags << " received. Reset packet is sent.");
		SendRST ();
		CloseAndNotify ();
	}
}

/** Peer sent me a FIN. Remember its sequence in rx buffer. */
void
TcpSocketBase::PeerClose (Ptr<Packet> p, const TcpHeader& tcpHeader)
{
	NS_LOG_FUNCTION (this << tcpHeader);

	// Ignore all out of range packets
	if (tcpHeader.GetSequenceNumber () < m_rxBuffer.NextRxSequence ()
			|| tcpHeader.GetSequenceNumber () > m_rxBuffer.MaxRxSequence ())
	{
		return;
	}
	// For any case, remember the FIN position in rx buffer first
	m_rxBuffer.SetFinSequence (tcpHeader.GetSequenceNumber () + SequenceNumber32 (p->GetSize ()));
	NS_LOG_LOGIC ("Accepted FIN at seq " << tcpHeader.GetSequenceNumber () + SequenceNumber32 (p->GetSize ()));
	// If there is any piggybacked data, process it
	if (p->GetSize ())
	{
		ReceivedData (p, tcpHeader);
	}
	// Return if FIN is out of sequence, otherwise move to CLOSE_WAIT state by DoPeerClose
	if (!m_rxBuffer.Finished ())
	{
		return;
	}

	// Simultaneous close: Application invoked Close() when we are processing this FIN packet
	if (m_state == FIN_WAIT_1)
	{
		NS_LOG_INFO ("FIN_WAIT_1 -> CLOSING");
		m_state = CLOSING;
		return;
	}

	DoPeerClose (); // Change state, respond with ACK
}

/** Received a in-sequence FIN. Close down this socket. */
void
TcpSocketBase::DoPeerClose (void)
{
	NS_ASSERT (m_state == ESTABLISHED || m_state == SYN_RCVD);

	// Move the state to CLOSE_WAIT
	NS_LOG_INFO (TcpStateName[m_state] << " -> CLOSE_WAIT");
	m_state = CLOSE_WAIT;

	if (!m_closeNotified)
	{
		// The normal behaviour for an application is that, when the peer sent a in-sequence
		// FIN, the app should prepare to close. The app has two choices at this point: either
		// respond with ShutdownSend() call to declare that it has nothing more to send and
		// the socket can be closed immediately; or remember the peer's close request, wait
		// until all its existing data are pushed into the TCP socket, then call Close()
		// explicitly.
		NS_LOG_LOGIC ("TCP " << this << " calling NotifyNormalClose");
		NotifyNormalClose ();
		m_closeNotified = true;
	}
	if (m_shutdownSend)
	{ // The application declares that it would not sent any more, close this socket
		Close ();
	}
	else
	{ // Need to ack, the application will close later
		SendEmptyPacket (TcpHeader::ACK , 0);
	}
	if (m_state == LAST_ACK)
	{
		NS_LOG_LOGIC ("TcpSocketBase " << this << " scheduling LATO1");
		m_lastAckEvent = Simulator::Schedule (m_rtt->RetransmitTimeout (),
				&TcpSocketBase::LastAckTimeout, this);
	}
}

/** Kill this socket. This is a callback function configured to m_endpoint in
   SetupCallback(), invoked when the endpoint is destroyed. */
void
TcpSocketBase::Destroy (void)
{
	NS_LOG_FUNCTION (this);
	m_endPoint = 0;
	if (m_tcp != 0)
	{
		std::vector<Ptr<TcpSocketBase> >::iterator it
		= std::find (m_tcp->m_sockets.begin (), m_tcp->m_sockets.end (), this);
		if (it != m_tcp->m_sockets.end ())
		{
			m_tcp->m_sockets.erase (it);
		}
	}
	NS_LOG_LOGIC (this << " Cancelled ReTxTimeout event which was set to expire at " <<
			(Simulator::Now () + Simulator::GetDelayLeft (m_retxEvent)).GetSeconds ());
	CancelAllTimers ();
}

/** Kill this socket. This is a callback function configured to m_endpoint in
   SetupCallback(), invoked when the endpoint is destroyed. */
void
TcpSocketBase::Destroy6 (void)
{
	NS_LOG_FUNCTION (this);
	m_endPoint6 = 0;
	if (m_tcp != 0)
	{
		std::vector<Ptr<TcpSocketBase> >::iterator it
		= std::find (m_tcp->m_sockets.begin (), m_tcp->m_sockets.end (), this);
		if (it != m_tcp->m_sockets.end ())
		{
			m_tcp->m_sockets.erase (it);
		}
	}
	NS_LOG_LOGIC (this << " Cancelled ReTxTimeout event which was set to expire at " <<
			(Simulator::Now () + Simulator::GetDelayLeft (m_retxEvent)).GetSeconds ());
	CancelAllTimers ();
}

/** Send an empty packet with specified TCP flags */
void
TcpSocketBase::SendEmptyPacket (uint8_t flags, uint8_t ecncount)
{
	NS_LOG_FUNCTION (this << (uint32_t)flags);
	Ptr<Packet> p = Create<Packet> ();
	TcpHeader header;
	SequenceNumber32 s = m_nextTxSequence;
	header.SetEcnCount(ecncount);
	/*
	 * Add tags for each socket option.
	 * Note that currently the socket adds both IPv4 tag and IPv6 tag
	 * if both options are set. Once the packet got to layer three, only
	 * the corresponding tags will be read.
	 */


	if (IsManualIpTos ())
	{
		SocketIpTosTag ipTosTag;
		ipTosTag.SetTos (GetIpTos ());
		p->AddPacketTag (ipTosTag);
	}

	if (IsManualIpv6Tclass ())
	{
		SocketIpv6TclassTag ipTclassTag;
		ipTclassTag.SetTclass (GetIpv6Tclass ());
		p->AddPacketTag (ipTclassTag);
	}

	if (IsManualIpTtl ())
	{
		SocketIpTtlTag ipTtlTag;
		ipTtlTag.SetTtl (GetIpTtl ());
		p->AddPacketTag (ipTtlTag);
	}

	if (IsManualIpv6HopLimit ())
	{
		SocketIpv6HopLimitTag ipHopLimitTag;
		ipHopLimitTag.SetHopLimit (GetIpv6HopLimit ());
		p->AddPacketTag (ipHopLimitTag);
	}

	if (m_endPoint == 0 && m_endPoint6 == 0)
	{
		NS_LOG_WARN ("Failed to send empty packet due to null endpoint");
		return;
	}
	if (flags & TcpHeader::FIN)
	{
		flags |= TcpHeader::ACK;
	}
	else if (m_state == FIN_WAIT_1 || m_state == LAST_ACK || m_state == CLOSING)
	{
		++s;
	}

	header.SetFlags (flags);
	header.SetSequenceNumber (s);
	header.SetAckNumber (m_rxBuffer.NextRxSequence ());
	if (m_endPoint != 0)
	{
		header.SetSourcePort (m_endPoint->GetLocalPort ());
		header.SetDestinationPort (m_endPoint->GetPeerPort ());
	}
	else
	{
		header.SetSourcePort (m_endPoint6->GetLocalPort ());
		header.SetDestinationPort (m_endPoint6->GetPeerPort ());
	}
	header.SetWindowSize (AdvertisedWindowSize ());
	AddOptions (header);
	m_rto = m_rtt->RetransmitTimeout ();
	bool hasSyn = flags & TcpHeader::SYN;
	bool hasFin = flags & TcpHeader::FIN;
	bool isAck = flags == TcpHeader::ACK;
	if (hasSyn)
	{
		if (m_cnCount == 0)
		{ // No more connection retries, give up
			NS_LOG_LOGIC ("Connection failed.");
			CloseAndNotify ();
			return;
		}
		else
		{ // Exponential backoff of connection time out
			int backoffCount = 0x1 << (m_cnRetries - m_cnCount);
			m_rto = m_cnTimeout * backoffCount;
			m_cnCount--;
		}
	}



	if (m_endPoint != 0)
	{
		SendPacket (p, header, m_endPoint->GetLocalAddress (),
				m_endPoint->GetPeerAddress (), m_boundnetdevice);
	}
	else
	{
		SendPacket (p, header, m_endPoint6->GetLocalAddress (),
				m_endPoint6->GetPeerAddress (), m_boundnetdevice);
	}
	if (m_retxEvent.IsExpired () && (hasSyn || hasFin) && !isAck )
	{ // Retransmit SYN / SYN+ACK / FIN / FIN+ACK to guard against lost
		NS_LOG_LOGIC ("Schedule retransmission timeout at time "
				<< Simulator::Now ().GetSeconds () << " to expire at time "
				<< (Simulator::Now () + m_rto.Get ()).GetSeconds ());
		m_retxEvent = Simulator::Schedule (m_rto, &TcpSocketBase::SendEmptyPacket, this, flags, ecncount);
	}
}
void
TcpSocketBase::SendPacket (Ptr<Packet> packet, const TcpHeader &outgoing,
		Address saddr, Address daddr, Ptr<NetDevice> oif)
{

	///////////////////////// Moji
//	MyTagHop tag;
//	tag.SetSimpleValue (0);
//	packet->AddPacketTag(tag);
//	MyTagQueue tag1;
//	tag1.SetSimpleValue (0);
//	packet->AddPacketTag(tag1);


	MyTagTenant tagTen;
	tagTen.SetSimpleValue(m_tenantid);
	packet->AddPacketTag(tagTen);
	MyTagFlow tagflow;
	tagflow.SetSimpleValue (m_flowid);
	packet->AddPacketTag(tagflow);

	///////////////////////// Moji End


	uint32_t MappedPriority = 39;
	PriorityTag priTag;
	uint32_t SocketPriority = 0;

	if (m_multiqueue)
	{ // Use Multiqueue
		if(m_windownum ==0)
		{

			MappedPriority = 39;

		}
		else{

			if(!m_incastserver){
				bool alreadyMapped = false;
				for (uint32_t i=0;i<8;i++)
				{
					if(m_node->m_node_Map_Flow[i] == m_endPoint->GetPeerAddress ().Get ())
					{
						alreadyMapped = true;
						MappedPriority = i;
					}

				}
				if(!alreadyMapped){
					uint32_t actualPriority = GetTupleValue (m_endPoint->GetLocalAddress (), m_endPoint->GetPeerAddress () ,6,outgoing);//%32;
					MappedPriority = actualPriority;
					m_node->m_node_Map_Flow[actualPriority] = m_endPoint->GetPeerAddress ().Get ();

				}
				if(m_windownum == 1)
					MappedPriority = MappedPriority;
				if(m_windownum == 2)
					MappedPriority = MappedPriority%4 + 10;
				if(m_windownum == 3)
					MappedPriority = MappedPriority%2 + 20;
				if(m_windownum >= 4)
					MappedPriority = 32;
			}
			else{

				bool alreadyMapped = false;
				for (uint32_t i=0;i<30;i++)
				{
					if(m_node->m_node_Map_Flow[i] == m_endPoint->GetPeerAddress ().Get ())
					{
						alreadyMapped = true;
						MappedPriority = i;
					}

				}
				if(!alreadyMapped){
					uint32_t actualPriority = GetTupleValue (m_endPoint->GetLocalAddress (), m_endPoint->GetPeerAddress () ,6,outgoing);//%32;
					MappedPriority = actualPriority;
					m_node->m_node_Map_Flow[actualPriority] = m_endPoint->GetPeerAddress ().Get ();

				}

			}
		}
	}

	MappedPriority = MappedPriority << 8;
//	if (m_priority > 0 )
//	{
//		SocketPriority = m_priority;
//		MappedPriority |= SocketPriority;
//	}

	priTag.SetPriority (MappedPriority);
	packet->AddPacketTag (priTag);
	NS_LOG_LOGIC ("TcpSocketBase: Sending packet with priority: " << MappedPriority);


	if (outgoing.GetFlags () & TcpHeader::ACK)
	{ // If sending an ACK, cancel the delay ACK as well
		m_delAckEvent.Cancel ();
		m_delAckCount = 0;
	}

	if (m_blocking && IsBlocked ())
	{
		NS_LOG_LOGIC ("Setting TCP socket to blocking state");
		//NS_LOG_UNCOND ("Setting TCP socket to blocking state");
		m_blocked = true;
	};

	//XXX: Ugly hack.

	if (m_endPoint)
	{
		m_tcp->SendPacket (packet, outgoing, m_endPoint->GetLocalAddress (),
				m_endPoint->GetPeerAddress (), oif);
	}
	else
	{
		m_tcp->SendPacket (packet, outgoing, m_endPoint6->GetLocalAddress (),
				m_endPoint6->GetPeerAddress (), oif);
	}
}

/** This function closes the endpoint completely. Called upon RST_TX action. */
void
TcpSocketBase::SendRST (void)
{
	NS_LOG_FUNCTION (this);
	SendEmptyPacket (TcpHeader::RST, 0);
	NotifyErrorClose ();
	DeallocateEndPoint ();
}

/** Deallocate the end point and cancel all the timers */
void
TcpSocketBase::DeallocateEndPoint (void)
{
	if (m_endPoint != 0)
	{
		m_endPoint->SetDestroyCallback (MakeNullCallback<void> ());
		m_tcp->DeAllocate (m_endPoint);
		m_endPoint = 0;
		std::vector<Ptr<TcpSocketBase> >::iterator it
		= std::find (m_tcp->m_sockets.begin (), m_tcp->m_sockets.end (), this);
		if (it != m_tcp->m_sockets.end ())
		{
			m_tcp->m_sockets.erase (it);
		}
		CancelAllTimers ();
	}
	if (m_endPoint6 != 0)
	{
		m_endPoint6->SetDestroyCallback (MakeNullCallback<void> ());
		m_tcp->DeAllocate (m_endPoint6);
		m_endPoint6 = 0;
		std::vector<Ptr<TcpSocketBase> >::iterator it
		= std::find (m_tcp->m_sockets.begin (), m_tcp->m_sockets.end (), this);
		if (it != m_tcp->m_sockets.end ())
		{
			m_tcp->m_sockets.erase (it);
		}
		CancelAllTimers ();
	}
}

/** Configure the endpoint to a local address. Called by Connect() if Bind() didn't specify one. */
int
TcpSocketBase::SetupEndpoint ()
{
	NS_LOG_FUNCTION (this);
	Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4> ();
	NS_ASSERT (ipv4 != 0);
	if (ipv4->GetRoutingProtocol () == 0)
	{
		NS_FATAL_ERROR ("No Ipv4RoutingProtocol in the node");
	}
	// Create a dummy packet, then ask the routing function for the best output
	// interface's address
	Ipv4Header header;
	header.SetDestination (m_endPoint->GetPeerAddress ());
	Socket::SocketErrno errno_;
	Ptr<Ipv4Route> route;
	Ptr<NetDevice> oif = m_boundnetdevice;
	route = ipv4->GetRoutingProtocol ()->RouteOutput (Ptr<Packet> (), header, oif, errno_);
	if (route == 0)
	{
		NS_LOG_LOGIC ("Route to " << m_endPoint->GetPeerAddress () << " does not exist");
		NS_LOG_ERROR (errno_);
		m_errno = errno_;
		return -1;
	}
	NS_LOG_LOGIC ("Route exists");
	m_endPoint->SetLocalAddress (route->GetSource ());
	return 0;
}

int
TcpSocketBase::SetupEndpoint6 ()
{
	NS_LOG_FUNCTION (this);
	Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol> ();
	NS_ASSERT (ipv6 != 0);
	if (ipv6->GetRoutingProtocol () == 0)
	{
		NS_FATAL_ERROR ("No Ipv6RoutingProtocol in the node");
	}
	// Create a dummy packet, then ask the routing function for the best output
	// interface's address
	Ipv6Header header;
	header.SetDestinationAddress (m_endPoint6->GetPeerAddress ());
	Socket::SocketErrno errno_;
	Ptr<Ipv6Route> route;
	Ptr<NetDevice> oif = m_boundnetdevice;
	route = ipv6->GetRoutingProtocol ()->RouteOutput (Ptr<Packet> (), header, oif, errno_);
	if (route == 0)
	{
		NS_LOG_LOGIC ("Route to " << m_endPoint6->GetPeerAddress () << " does not exist");
		NS_LOG_ERROR (errno_);
		m_errno = errno_;
		return -1;
	}
	NS_LOG_LOGIC ("Route exists");
	m_endPoint6->SetLocalAddress (route->GetSource ());
	return 0;
}

/** This function is called only if a SYN received in LISTEN state. After
   TcpSocketBase cloned, allocate a new end point to handle the incoming
   connection and send a SYN+ACK to complete the handshake. */
void
TcpSocketBase::CompleteFork (Ptr<Packet> p, const TcpHeader& h,
		const Address& fromAddress, const Address& toAddress)
{
	// Get port and address from peer (connecting host)
	if (InetSocketAddress::IsMatchingType (toAddress))
	{
		m_endPoint = m_tcp->Allocate (InetSocketAddress::ConvertFrom (toAddress).GetIpv4 (),
				InetSocketAddress::ConvertFrom (toAddress).GetPort (),
				InetSocketAddress::ConvertFrom (fromAddress).GetIpv4 (),
				InetSocketAddress::ConvertFrom (fromAddress).GetPort ());
		m_endPoint6 = 0;
	}
	else if (Inet6SocketAddress::IsMatchingType (toAddress))
	{
		m_endPoint6 = m_tcp->Allocate6 (Inet6SocketAddress::ConvertFrom (toAddress).GetIpv6 (),
				Inet6SocketAddress::ConvertFrom (toAddress).GetPort (),
				Inet6SocketAddress::ConvertFrom (fromAddress).GetIpv6 (),
				Inet6SocketAddress::ConvertFrom (fromAddress).GetPort ());
		m_endPoint = 0;
	}
	m_tcp->m_sockets.push_back (this);

	// Change the cloned socket from LISTEN state to SYN_RCVD
	NS_LOG_INFO ("LISTEN -> SYN_RCVD");
	m_state = SYN_RCVD;
	m_cnCount = m_cnRetries;
	SetupCallback ();
	// Set the sequence number and send SYN+ACK
	m_rxBuffer.SetNextRxSequence (h.GetSequenceNumber () + SequenceNumber32 (1));
	SendEmptyPacket (TcpHeader::SYN | TcpHeader::ACK , h.GetEcnCount ());
}

void
TcpSocketBase::ConnectionSucceeded ()
{ // Wrapper to protected function NotifyConnectionSucceeded() so that it can
	// be called as a scheduled event
	NotifyConnectionSucceeded ();
	// The if-block below was moved from ProcessSynSent() to here because we need
	// to invoke the NotifySend() only after NotifyConnectionSucceeded() to
	// reflect the behaviour in the real world.
	if (GetTxAvailable () > 0)
	{
		NotifySend (GetTxAvailable ());
	}
}

/** Extract at most maxSize bytes from the TxBuffer at sequence seq, add the
    TCP header, and send to TcpL4Protocol */
uint32_t
TcpSocketBase::SendDataPacket (SequenceNumber32 seq, uint32_t maxSize, bool withAck)
{
	NS_LOG_FUNCTION (this << seq << maxSize << withAck);

	Ptr<Packet> p = m_txBuffer.CopyFromSequence (maxSize, seq);
	uint32_t sz = p->GetSize (); // Size of packet
	uint8_t flags = withAck ? TcpHeader::ACK : 0;
	uint32_t remainingData = m_txBuffer.SizeFromSequence (seq + SequenceNumber32 (sz));

	/*
	 * Add tags for each socket option.
	 * Note that currently the socket adds both IPv4 tag and IPv6 tag
	 * if both options are set. Once the packet got to layer three, only
	 * the corresponding tags will be read.
	 */
	if (IsManualIpTos ())
	{
		SocketIpTosTag ipTosTag;
		ipTosTag.SetTos (GetIpTos ());
		p->AddPacketTag (ipTosTag);
	}

	if (IsManualIpv6Tclass ())
	{
		SocketIpv6TclassTag ipTclassTag;
		ipTclassTag.SetTclass (GetIpv6Tclass ());
		p->AddPacketTag (ipTclassTag);
	}

	if (IsManualIpTtl ())
	{
		SocketIpTtlTag ipTtlTag;
		ipTtlTag.SetTtl (GetIpTtl ());
		p->AddPacketTag (ipTtlTag);
	}

	if (IsManualIpv6HopLimit ())
	{
		SocketIpv6HopLimitTag ipHopLimitTag;
		ipHopLimitTag.SetHopLimit (GetIpv6HopLimit ());
		p->AddPacketTag (ipHopLimitTag);
	}

	if (m_closeOnEmpty && (remainingData == 0))
	{
		flags |= TcpHeader::FIN;
		if (m_state == ESTABLISHED)
		{ // On active close: I am the first one to send FIN
			NS_LOG_INFO ("ESTABLISHED -> FIN_WAIT_1");
			m_state = FIN_WAIT_1;
			//  flowendtime = Simulator::Now();
			// double duration = flowendtime.GetNanoSeconds() - flowstarttime.GetNanoSeconds();
			//          double rxMB = ((double)flowbytes) / (1024 * 1024);
			//double tputMb = 8.0 * m_totalRx * 1000 / duration;
			//	ofstream outfile;
			//	outfile.open(m_filename.c_str(),  ofstream::app);
			// 	outfile << setiosflags(ios::fixed);
			//outfile << "[" << setw(6) << setprecision(3) << Simulator::Now ().GetSeconds () << "]";
			//	outfile << "\tNode:" << setw(3) << m_node->GetId();
			// 	outfile << "\tFlow latency(ms):" << setw(6) << setprecision(3) << duration/1000000;
			// 	outfile << "\tsize(KB):"         << setw(5) << setprecision(0) << rxMB * 1024;
			//      outfile << "\ttimeouts:"         << setw(9) << setprecision(0) << timeouts;
			//	outfile << "\tEcnMarked:"        << setw(6) << setprecision(1) << m_totalmarkedPackets;
			//	outfile << "\tEcnCount:"         << setw(6) << setprecision(1) << m_totalEcnCount << endl;
			//      outfile.close();
		}
		else if (m_state == CLOSE_WAIT)
		{ // On passive close: Peer sent me FIN already
			NS_LOG_INFO ("CLOSE_WAIT -> LAST_ACK");
			m_state = LAST_ACK;
		}
		//XXX: I want to know when a socket I closed actualy gets closed
		//NotifyNormalClose ();
	}
	TcpHeader header;
	header.SetFlags (flags);
	header.SetSequenceNumber (seq);
	header.SetAckNumber (m_rxBuffer.NextRxSequence ());
	if (m_endPoint)
	{
		header.SetSourcePort (m_endPoint->GetLocalPort ());
		header.SetDestinationPort (m_endPoint->GetPeerPort ());
	}
	else
	{
		header.SetSourcePort (m_endPoint6->GetLocalPort ());
		header.SetDestinationPort (m_endPoint6->GetPeerPort ());
	}
	header.SetWindowSize (AdvertisedWindowSize ());
	AddOptions (header);
	if (m_retxEvent.IsExpired () )
	{ // Schedule retransmit
		m_rto = m_rtt->RetransmitTimeout ();
		NS_LOG_LOGIC (this << " SendDataPacket Schedule ReTxTimeout at time " <<
				Simulator::Now ().GetSeconds () << " to expire at time " <<
				(Simulator::Now () + m_rto.Get ()).GetSeconds () );
		timeouts++;
		m_retxEvent = Simulator::Schedule (m_rto, &TcpSocketBase::ReTxTimeout, this);
	}
	MyTagFlow ttback;
	ttback.SetSimpleValue (m_flowid);
	p->AddPacketTag(ttback);






	NS_LOG_LOGIC ("Send packet via TcpL4Protocol with flags 0x" << std::hex << static_cast<uint32_t> (flags) << std::dec);
	if (m_endPoint)
	{
		SendPacket (p, header, m_endPoint->GetLocalAddress (),
				m_endPoint->GetPeerAddress (), m_boundnetdevice);
	}
	else
	{
		SendPacket (p, header, m_endPoint6->GetLocalAddress (),
				m_endPoint6->GetPeerAddress (), m_boundnetdevice);
	}
	m_rtt->SentSeq (seq, sz);       // notify the RTT
	// Notify the application of the data being sent unless this is a retransmit
	if (seq == m_nextTxSequence)
	{
		Simulator::ScheduleNow (&TcpSocketBase::NotifyDataSent, this, sz);
	}
	// Update highTxMark
	m_highTxMark = std::max (seq + sz, m_highTxMark.Get ());
	return sz;
}

void
TcpSocketBase::DeviceUnblocked(Ptr<NetDevice> nd, uint32_t avail)
{
	NS_LOG_FUNCTION (this << nd << avail);
	//Ptr<QbbNetDevice> qbb = nd->GetObject<QbbNetDevice>();
	if (nd->GetTxAvailable () >= m_segmentSize) {
		m_blocked = false;
		Simulator::ScheduleNow(&TcpSocketBase::CancelNetDeviceCallback, this, nd);
		SendPendingData(m_connected);
	};
};

void
TcpSocketBase::CancelNetDeviceCallback(Ptr<NetDevice> nd)
{
	NS_LOG_FUNCTION (this << nd);
	nd->DisconnectWithoutContext (MakeCallback (&TcpSocketBase::DeviceUnblocked, this));
};

bool
TcpSocketBase::IsBlocked (void)
{
	NS_LOG_FUNCTION (this);
	//NS_LOG_UNCOND ("Checking if blocking");
	// Assumed we called SetupEndPoint, now we get the route to destination
	Ptr<Ipv4RoutingProtocol> rp = m_node->GetObject<Ipv4> ()->GetRoutingProtocol ();
	Ipv4Header header;
	header.SetDestination (m_endPoint->GetPeerAddress ());
	Socket::SocketErrno errno_;
	Ptr<Ipv4Route> route = m_node->GetObject<Ipv4> ()->GetRoutingProtocol ()->RouteOutput (Ptr<Packet> (), header, 0, errno_);
	// From the route, get the netdevice
	Ptr<NetDevice> nd = route->GetOutputDevice ();
	//NS_LOG_UNCOND ("nd GetTxAvailable: " << nd->GetTxAvailable ());
	if (nd != 0 && nd->GetTxAvailable () < m_segmentSize)
	{
		NS_LOG_LOGIC ("NetDevice buffer full. Device is blocked.");
		//NS_LOG_UNCOND ("NetDevice buffer full. Device is blocked.");
		nd->ConnectWithoutContext (MakeCallback(&TcpSocketBase::DeviceUnblocked, this));
		//XXX: Got wild with setting this variable
		m_blocked = true;
		return true;
	};

	return false;
}

/** Send as much pending data as possible according to the Tx window. Note that
 *  this function did not implement the PSH flag
 */
bool
TcpSocketBase::SendPendingData (bool withAck)
{
	NS_LOG_FUNCTION (this << withAck);
	if (m_txBuffer.Size () == 0)
	{
		return false;                           // Nothing to send

	}
	if (m_endPoint == 0 && m_endPoint6 == 0)
	{
		NS_LOG_INFO ("TcpSocketBase::SendPendingData: No endpoint; m_shutdownSend=" << m_shutdownSend);
		return false; // Is this the right way to handle this condition?
	}

	uint32_t nPacketsSent = 0;
	while (m_txBuffer.SizeFromSequence (m_nextTxSequence))
	{
		uint32_t w = AvailableWindow (); // Get available window size
		NS_LOG_LOGIC ("TcpSocketBase " << this << " SendPendingData" <<
				" w " << w <<
				" rxwin " << m_rWnd <<
				" segsize " << m_segmentSize <<
				" nextTxSeq " << m_nextTxSequence <<
				" highestRxAck " << m_txBuffer.HeadSequence () <<
				" pd->Size " << m_txBuffer.Size () <<
				" pd->SFS " << m_txBuffer.SizeFromSequence (m_nextTxSequence));

		m_windownum = m_seqnum / m_segmentSize ;
		if(m_windownum>=0 && m_windownum <10)
			m_windownum = 1;
		else if (m_windownum < 30)
			m_windownum = 2;
		else if (m_windownum < 70)
			m_windownum = 3;
		else
			m_windownum = 4;
		// Quit if send disallowed
		if (m_shutdownSend)
		{
			m_errno = ERROR_SHUTDOWN;
			return false;
		}
		// Stop sending if we need to wait for a larger Tx window (prevent silly window syndrome)
		if (w < m_segmentSize && m_txBuffer.SizeFromSequence (m_nextTxSequence) > w)
		{
			break; // No more
		}
		//XXX: This doesn't work if the RTO is too low
		if (m_blocking && IsBlocked ())
		{
			m_blocked = true;
			break;
		};
		// Nagle's algorithm (RFC896): Hold off sending if there is unacked data
		// in the buffer and the amount of data to send is less than one segment
		if (!m_noDelay && UnAckDataCount () > 0
				&& m_txBuffer.SizeFromSequence (m_nextTxSequence) < m_segmentSize)
		{
			NS_LOG_LOGIC ("Invoking Nagle's algorithm. Wait to send.");
			break;
		}
		uint32_t s = std::min (w, m_segmentSize);  // Send no more than window
		uint32_t sz = SendDataPacket (m_nextTxSequence, s, withAck);
		nPacketsSent++;
		m_seqnum += sz;                             // Count sent this loop
		m_nextTxSequence += sz;                     // Advance next tx sequence
	}
	NS_LOG_LOGIC ("SendPendingData sent " << nPacketsSent << " packets");
	return (nPacketsSent > 0);
}

uint32_t
TcpSocketBase::UnAckDataCount ()
{
	NS_LOG_FUNCTION (this);
	return m_nextTxSequence.Get () - m_txBuffer.HeadSequence ();
}

uint32_t
TcpSocketBase::BytesInFlight ()
{
	NS_LOG_FUNCTION (this);
	return m_highTxMark.Get () - m_txBuffer.HeadSequence ();
}

uint32_t
TcpSocketBase::Window ()
{
	NS_LOG_FUNCTION (this);
	return m_rWnd;
}

void 
TcpSocketBase::SetFilename (std::string filename)
{
	m_filename = filename;
}

uint32_t
TcpSocketBase::AvailableWindow ()
{
	NS_LOG_FUNCTION_NOARGS ();
	uint32_t unack = UnAckDataCount (); // Number of outstanding bytes
	uint32_t win = Window (); // Number of bytes allowed to be outstanding
	NS_LOG_LOGIC ("UnAckCount=" << unack << ", Win=" << win);
	return (win < unack) ? 0 : (win - unack);
}

uint16_t
TcpSocketBase::AdvertisedWindowSize ()
{
	return std::min (m_rxBuffer.MaxBufferSize () - m_rxBuffer.Size (), (uint32_t)m_maxWinSize);
}

// Receipt of new packet, put into Rx buffer
void
TcpSocketBase::ReceivedData (Ptr<Packet> p, const TcpHeader& tcpHeader)
{
	NS_LOG_FUNCTION (this << tcpHeader);
	NS_LOG_LOGIC ("seq " << tcpHeader.GetSequenceNumber () <<
			" ack " << tcpHeader.GetAckNumber () <<
			" pkt size " << p->GetSize () );


//	std::cout << " TIME: " << Simulator::Now ().GetSeconds () << " m_logger_packet " <<m_logger_packet << std::endl;
	  m_logger_packet += p->GetSize();


	//XXX
	PriorityTag tag;
	std::ostringstream oss;
	if (p->PeekPacketTag (tag))
	{
		oss << "TcpSocketBase: deque pri: " << tag.GetPriority ();
	}
	else
	{
		oss << "TcpSocketBase: Packet with no priority: ";
	}
	NS_LOG_LOGIC (oss.str ());

	// Put into Rx buffer
	SequenceNumber32 expectedSeq = m_rxBuffer.NextRxSequence ();
	if (!m_rxBuffer.Add (p, tcpHeader))
	{ // Insert failed: No data or RX buffer full
		SendEmptyPacket (TcpHeader::ACK, tcpHeader.GetEcnCount ());
		return;
	}
	// Now send a new ACK packet acknowledging all received and delivered data
	if (m_rxBuffer.Size () > m_rxBuffer.Available () || m_rxBuffer.NextRxSequence () > expectedSeq + p->GetSize ())
	{ // A gap exists in the buffer, or we filled a gap: Always ACK
		SendEmptyPacket (TcpHeader::ACK, tcpHeader.GetEcnCount ());
	}
	else
	{ // In-sequence packet: ACK if delayed ack count allows
		if (++m_delAckCount >= m_delAckMaxCount)
		{
			SendEmptyPacket (TcpHeader::ACK, tcpHeader.GetEcnCount ());
		}
		else if (m_delAckEvent.IsExpired ())
		{
			m_delAckEvent = Simulator::Schedule (m_delAckTimeout,
					&TcpSocketBase::DelAckTimeout, this);
			NS_LOG_LOGIC (this << " scheduled delayed ACK at " << (Simulator::Now () + Simulator::GetDelayLeft (m_delAckEvent)).GetSeconds ());
		}
	}
	// Notify app to receive if necessary
	if (expectedSeq < m_rxBuffer.NextRxSequence ())
	{ // NextRxSeq advanced, we have something to send to the app
		if (!m_shutdownRecv)
		{
			NotifyDataRecv ();
		}
		// Handle exceptions
		if (m_closeNotified)
		{
			NS_LOG_WARN ("Why TCP " << this << " got data after close notification?");
		}
		// If we received FIN before and now completed all "holes" in rx buffer,
		// invoke peer close procedure
		if (m_rxBuffer.Finished () && (tcpHeader.GetFlags () & TcpHeader::FIN) == 0)
		{
			DoPeerClose ();
		}
	}
}

/** Called by ForwardUp() to estimate RTT */
void
TcpSocketBase::EstimateRtt (const TcpHeader& tcpHeader)
{
	// Use m_rtt for the estimation. Note, RTT of duplicated acknowledgement
	// (which should be ignored) is handled by m_rtt. Once timestamp option
	// is implemented, this function would be more elaborated.
	Time nextRtt =  m_rtt->AckSeq (tcpHeader.GetAckNumber () );

	//nextRtt will be zero for dup acks.  Don't want to update lastRtt in that case
	//but still needed to do list clearing that is done in AckSeq.
	if(nextRtt != 0)
	{
		m_lastRtt = nextRtt;
		NS_LOG_FUNCTION(this << m_lastRtt);
	}

}

// Called by the ReceivedAck() when new ACK received and by ProcessSynRcvd()
// when the three-way handshake completed. This cancels retransmission timer
// and advances Tx window
void
TcpSocketBase::NewAck (SequenceNumber32 const& ack)
{
	NS_LOG_FUNCTION (this << ack);

	if (m_state != SYN_RCVD)
	{ // Set RTO unless the ACK is received in SYN_RCVD state
		NS_LOG_LOGIC (this << " Cancelled ReTxTimeout event which was set to expire at " <<
				(Simulator::Now () + Simulator::GetDelayLeft (m_retxEvent)).GetSeconds ());
		m_retxEvent.Cancel ();
		// On recieving a "New" ack we restart retransmission timer .. RFC 2988
		m_rto = m_rtt->RetransmitTimeout ();
		NS_LOG_LOGIC (this << " Schedule ReTxTimeout at time " <<
				Simulator::Now ().GetSeconds () << " to expire at time " <<
				(Simulator::Now () + m_rto.Get ()).GetSeconds ());
		m_retxEvent = Simulator::Schedule (m_rto, &TcpSocketBase::ReTxTimeout, this);
	}
	if (m_rWnd.Get () == 0 && m_persistEvent.IsExpired ())
	{ // Zero window: Enter persist state to send 1 byte to probe
		NS_LOG_LOGIC (this << "Enter zerowindow persist state");
		NS_LOG_LOGIC (this << "Cancelled ReTxTimeout event which was set to expire at " <<
				(Simulator::Now () + Simulator::GetDelayLeft (m_retxEvent)).GetSeconds ());
		m_retxEvent.Cancel ();
		NS_LOG_LOGIC ("Schedule persist timeout at time " <<
				Simulator::Now ().GetSeconds () << " to expire at time " <<
				(Simulator::Now () + m_persistTimeout).GetSeconds ());
		m_persistEvent = Simulator::Schedule (m_persistTimeout, &TcpSocketBase::PersistTimeout, this);
		NS_ASSERT (m_persistTimeout == Simulator::GetDelayLeft (m_persistEvent));
	}
	// Note the highest ACK and tell app to send more
	NS_LOG_LOGIC ("TCP " << this << " NewAck " << ack <<
			" numberAck " << (ack - m_txBuffer.HeadSequence ())); // Number bytes ack'ed
	m_ackdBytes += (ack - m_txBuffer.HeadSequence ());
	m_txBuffer.DiscardUpTo (ack);
	if (GetTxAvailable () > 0)
	{
		NotifySend (GetTxAvailable ());
	}
	if (ack > m_nextTxSequence)
	{
		m_nextTxSequence = ack; // If advanced
	}
	if (m_txBuffer.Size () == 0 && m_state != FIN_WAIT_1 && m_state != CLOSING)
	{ // No retransmit timer if no data to retransmit
		NS_LOG_LOGIC (this << " Cancelled ReTxTimeout event which was set to expire at " <<
				(Simulator::Now () + Simulator::GetDelayLeft (m_retxEvent)).GetSeconds ());
		m_retxEvent.Cancel ();
	}
//	std::cout << " =========== flowid " << m_flowid << " Tenant " << m_tenantid << std::endl;
	//XXX: Brent - hopefully this will work
	if (m_state == FIN_WAIT_1)
	{
		//NS_LOG_UNCOND ("FIN_WAIT_1: ack=" << ack << ", nextTx=" << m_nextTxSequence);
		if (ack == m_nextTxSequence)
		{ // This ACK corresponds to the FIN sent. This socket closed peacefully.
//			uint32_t a = Simulator::getTrafficClassNumber()-1;
//			Simulator::setTrafficClassNumber(a);
			flowendtime = Simulator::Now();
			double duration = flowendtime.GetNanoSeconds() - flowstarttime.GetNanoSeconds();
//			std::cout << "====  " << duration<< std::endl;

			double rxMB = ((double)flowbytes) / (1024 * 1024);
			//double tputMb = 8.0 * m_totalRx * 1000 / duration;
			ofstream outfile;
			outfile.open(m_filename.c_str(),  ofstream::app);
			outfile << setiosflags(ios::fixed);
			//outfile << "[" << setw(6) << setprecision(3) << Simulator::Now ().GetSeconds () << "]";

			///////////////////////////////////////////////////////////
//			outfile << "\tTenant:" << setw(3) << m_tenantid;
////			outfile << "\tFlow:" << setw(3) << m_flowid;
//			outfile << "\tNode:" << setw(3) << m_node->GetId();
//			outfile << "\tFlow latency(ms):" << setw(6) << setprecision(3) << duration/1000000;
//			outfile << "\tsize(KB):"         << setw(5) << setprecision(0) << rxMB * 1024;
//			outfile << "\ttimeouts:"         << setw(9) << setprecision(0) << timeouts;
//			outfile << "\tEcnMarked:"        << setw(6) << setprecision(1) << m_totalmarkedPackets;
//			outfile << "\tEcnCount:"         << setw(6) << setprecision(1) << m_totalEcnCount << endl;



			outfile << " tenant: "  << m_tenantid;
			outfile << " flow: " << m_flowid;
			outfile << " node: " << m_node->GetId();
			outfile << " latency(ms): " << setprecision(3) << duration/1000000;
			outfile << " size(KB): "          << setprecision(0) << rxMB * 1024;
			outfile << " timeouts: "          << setprecision(0) << timeouts;
			outfile << " EcnMarked: "        << setprecision(1) << m_totalmarkedPackets;
			outfile << " EcnCount: "  << setprecision(1) << m_totalEcnCount << endl;






			outfile.close();
			CloseAndNotify ();
		}
	}

	// Try to send more data
	SendPendingData (m_connected);
}

// Retransmit timeout
void
TcpSocketBase::ReTxTimeout ()
{
	NS_LOG_FUNCTION (this);
	NS_LOG_LOGIC (this << " ReTxTimeout Expired at time " << Simulator::Now ().GetSeconds ());
	//NS_LOG_UNCOND (this << " ReTxTimeout Expired at time " << Simulator::Now ().GetSeconds ());

	//XXX: If the socket is blocked, reschedule the ReTxTimeout ()
	if (m_blocking && IsBlocked ())
	{
		//NS_LOG_UNCOND (this << " ReTx is blocked");
		m_rto = m_rtt->RetransmitTimeout ();
		NS_LOG_LOGIC (this << " SendDataPacket Re-scheduling ReTxTimeout at time " <<
				Simulator::Now ().GetSeconds () << " to expire at time " <<
				(Simulator::Now () + m_rto.Get ()).GetSeconds () << " because the socket is blocking");
		m_retxEvent = Simulator::Schedule (m_rto, &TcpSocketBase::ReTxTimeout, this);
		return;
	}

	// If erroneous timeout in closed/timed-wait state, just return
	if (m_state == CLOSED || m_state == TIME_WAIT)
	{
		return;
	}
	// If all data are received (non-closing socket and nothing to send), just return
	if (m_state <= ESTABLISHED && m_txBuffer.HeadSequence () >= m_highTxMark)
	{
		return;
	}

	//NS_LOG_UNCOND ("Doing retransmit");
	Retransmit ();
}

void
TcpSocketBase::DelAckTimeout (void)
{
	SendEmptyPacket (TcpHeader::ACK, 0);
}

void
TcpSocketBase::LastAckTimeout (void)
{
	NS_LOG_FUNCTION (this);

	m_lastAckEvent.Cancel ();
	if (m_state == LAST_ACK)
	{
		CloseAndNotify ();
	}
	if (!m_closeNotified)
	{
		m_closeNotified = true;
	}
}

// Send 1-byte data to probe for the window size at the receiver when
// the local knowledge tells that the receiver has zero window size
// C.f.: RFC793 p.42, RFC1112 sec.4.2.2.17
void
TcpSocketBase::PersistTimeout ()
{
	NS_LOG_LOGIC ("PersistTimeout expired at " << Simulator::Now ().GetSeconds ());
	m_persistTimeout = std::min (Seconds (60), Time (2 * m_persistTimeout)); // max persist timeout = 60s
	Ptr<Packet> p = m_txBuffer.CopyFromSequence (1, m_nextTxSequence);
	TcpHeader tcpHeader;
	tcpHeader.SetSequenceNumber (m_nextTxSequence);
	tcpHeader.SetAckNumber (m_rxBuffer.NextRxSequence ());
	tcpHeader.SetWindowSize (AdvertisedWindowSize ());
	if (m_endPoint != 0)
	{
		tcpHeader.SetSourcePort (m_endPoint->GetLocalPort ());
		tcpHeader.SetDestinationPort (m_endPoint->GetPeerPort ());
	}
	else
	{
		tcpHeader.SetSourcePort (m_endPoint6->GetLocalPort ());
		tcpHeader.SetDestinationPort (m_endPoint6->GetPeerPort ());
	}
	AddOptions (tcpHeader);

	if (m_endPoint != 0)
	{
		SendPacket (p, tcpHeader, m_endPoint->GetLocalAddress (),
				m_endPoint->GetPeerAddress (), m_boundnetdevice);
	}
	else
	{
		SendPacket (p, tcpHeader, m_endPoint6->GetLocalAddress (),
				m_endPoint6->GetPeerAddress (), m_boundnetdevice);
	}
	NS_LOG_LOGIC ("Schedule persist timeout at time "
			<< Simulator::Now ().GetSeconds () << " to expire at time "
			<< (Simulator::Now () + m_persistTimeout).GetSeconds ());
	m_persistEvent = Simulator::Schedule (m_persistTimeout, &TcpSocketBase::PersistTimeout, this);
}

void
TcpSocketBase::Retransmit ()
{
	m_nextTxSequence = m_txBuffer.HeadSequence (); // Start from highest Ack
	m_rtt->IncreaseMultiplier (); // Double the timeout value for next retx timer
	m_dupAckCount = 0;
	DoRetransmit (); // Retransmit the packet
}

void
TcpSocketBase::DoRetransmit ()
{
	NS_LOG_FUNCTION (this);
	// Retransmit SYN packet
	if (m_state == SYN_SENT)
	{
		if (m_cnCount > 0)
		{
			SendEmptyPacket (TcpHeader::SYN, 0);
		}
		else
		{
			NotifyConnectionFailed ();
		}
		return;
	}
	// Retransmit non-data packet: Only if in FIN_WAIT_1 or CLOSING state
	if (m_txBuffer.Size () == 0)
	{
		if (m_state == FIN_WAIT_1 || m_state == CLOSING)
		{ // Must have lost FIN, re-send
			SendEmptyPacket (TcpHeader::FIN, 0);
		}
		return;
	}
	// Retransmit a data packet: Call SendDataPacket
	NS_LOG_LOGIC ("TcpSocketBase " << this << " retxing seq " << m_txBuffer.HeadSequence ());
	uint32_t sz = SendDataPacket (m_txBuffer.HeadSequence (), m_segmentSize, true);
	// In case of RTO, advance m_nextTxSequence
	m_nextTxSequence = std::max (m_nextTxSequence.Get (), m_txBuffer.HeadSequence () + sz);

}

void
TcpSocketBase::CancelAllTimers ()
{
	m_retxEvent.Cancel ();
	m_persistEvent.Cancel ();
	m_delAckEvent.Cancel ();
	m_lastAckEvent.Cancel ();
	m_timewaitEvent.Cancel ();
}

/** Move TCP to Time_Wait state and schedule a transition to Closed state */
void
TcpSocketBase::TimeWait ()
{
	NS_LOG_INFO (TcpStateName[m_state] << " -> TIME_WAIT");
	m_state = TIME_WAIT;
	CancelAllTimers ();
	// Move from TIME_WAIT to CLOSED after 2*MSL. Max segment lifetime is 2 min
	// according to RFC793, p.28
	m_timewaitEvent = Simulator::Schedule (Seconds (2 * m_msl),
			&TcpSocketBase::CloseAndNotify, this);
}

/** Below are the attribute get/set functions */

void
TcpSocketBase::SetSndBufSize (uint32_t size)
{
	m_txBuffer.SetMaxBufferSize (size);
}

uint32_t
TcpSocketBase::GetSndBufSize (void) const
{
	return m_txBuffer.MaxBufferSize ();
}

void
TcpSocketBase::SetRcvBufSize (uint32_t size)
{
	m_rxBuffer.SetMaxBufferSize (size);
}

uint32_t
TcpSocketBase::GetRcvBufSize (void) const
{
	return m_rxBuffer.MaxBufferSize ();
}

void
TcpSocketBase::SetSegSize (uint32_t size)
{
	m_segmentSize = size;
	NS_ABORT_MSG_UNLESS (m_state == CLOSED, "Cannot change segment size dynamically.");
}

uint32_t
TcpSocketBase::GetSegSize (void) const
{
	return m_segmentSize;
}

void
TcpSocketBase::SetConnTimeout (Time timeout)
{
	m_cnTimeout = timeout;
}

Time
TcpSocketBase::GetConnTimeout (void) const
{
	return m_cnTimeout;
}

void
TcpSocketBase::SetConnCount (uint32_t count)
{
	m_cnRetries = count;
}

uint32_t
TcpSocketBase::GetConnCount (void) const
{
	return m_cnRetries;
}

void
TcpSocketBase::SetDelAckTimeout (Time timeout)
{
	m_delAckTimeout = timeout;
}

Time
TcpSocketBase::GetDelAckTimeout (void) const
{
	return m_delAckTimeout;
}

void
TcpSocketBase::SetDelAckMaxCount (uint32_t count)
{
	m_delAckMaxCount = count;
}

uint32_t
TcpSocketBase::GetDelAckMaxCount (void) const
{
	return m_delAckMaxCount;
}

void
TcpSocketBase::SetTcpNoDelay (bool noDelay)
{
	m_noDelay = noDelay;
}

bool
TcpSocketBase::GetTcpNoDelay (void) const
{
	return m_noDelay;
}

void
TcpSocketBase::SetPersistTimeout (Time timeout)
{
	m_persistTimeout = timeout;
}

Time
TcpSocketBase::GetPersistTimeout (void) const
{
	return m_persistTimeout;
}

bool
TcpSocketBase::SetAllowBroadcast (bool allowBroadcast)
{
	// Broadcast is not implemented. Return true only if allowBroadcast==false
	return (!allowBroadcast);
}

bool
TcpSocketBase::GetAllowBroadcast (void) const
{
	return false;
}

/** Placeholder function for future extension that reads more from the TCP header */
void
TcpSocketBase::ReadOptions (const TcpHeader&)
{
}

/** Placeholder function for future extension that changes the TCP header */
void
TcpSocketBase::AddOptions (TcpHeader&)
{
}



void TcpSocketBase::setTenant(uint32_t t_id){
	//	tenant_id = t_id;
}


void
TcpSocketBase::logger (void)
{
	//	std::cout << " TIME: " << Simulator::Now ().GetSeconds () << " =================== "  << m_logger_packet << " " <<  samplingInterval.GetNanoSeconds() <<  std::endl;//

//	if (Simulator::Now() > m_flowid * Seconds(0.5) + Seconds(2.5)) return;
	//	if( m_tenant < 100 && m_flowId < 100 ){
	double rr = (double) m_logger_packet / samplingInterval.GetSeconds();
	//		rr = rr / 1000000 ;
			if(rr > 0){
	//			if(Simulator::Now () > Seconds(0.1) && Simulator::Now () < Seconds(0.3) ){
	std::cout << " TIME: " << Simulator::Now ().GetSeconds () <<  " TENATNT: " << m_tenantid <<  " ID: "  << m_flowid<< " RATE: "  << 8*rr << " bps " << rr << " Bps "  << 8*rr/1000000000 << " Gbps "<< std::endl;//
	m_logger_packet=0;
//				}


	//		}
	Simulator::Schedule(samplingInterval, &TcpSocketBase::logger, this);
		}
}


} // namespace ns3



