
/*
 * Copyright (c) 2007 University of Washington
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
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "priority-tag.h"
#include "dctcp-queue.h"
#include <cstdlib>
#include <algorithm>.
#include "ns3/simulator.h"

//#include "ns3/random-variable-stream.h"


NS_LOG_COMPONENT_DEFINE ("DCTCPQueue");

using namespace std;
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DCTCPQueue);

TypeId DCTCPQueue::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::DCTCPQueue")
		.SetParent<Queue> ()
		.AddConstructor<DCTCPQueue> ()
		.AddAttribute ("Mode",
				"Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
				EnumValue (QUEUE_MODE_PACKETS),
				MakeEnumAccessor (&DCTCPQueue::SetMode),
				MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
				QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
		.AddAttribute ("MaxPackets",
				"The maximum number of packets accepted by this DropTailQueue.",
				UintegerValue (1000),
				MakeUintegerAccessor (&DCTCPQueue::m_maxPackets),
				MakeUintegerChecker<uint32_t> ())
		.AddAttribute ("MaxBytes",
				"The maximum number of bytes accepted by this DropTailQueue.",
				UintegerValue (400),
				MakeUintegerAccessor (&DCTCPQueue::m_maxBytes),
				MakeUintegerChecker<uint32_t> ())
		.AddAttribute ("Th",
				"Instantaneous mark length threshold in packets/bytes",
				UintegerValue (40),
				MakeUintegerAccessor (&DCTCPQueue::m_th),
				MakeUintegerChecker<uint32_t> ());

	return tid;
}

DCTCPQueue::DCTCPQueue () :
			Queue (),
			m_Qpackets(),
			m_bytesInQueue (0),
			m_ECN_active (-1),
			m_maxlevels(97),
			nodenumber(0),
			m_max_tocken(5),
			nodeport(0)
{
	NS_LOG_FUNCTION_NOARGS ();
	Simulator::Schedule(MilliSeconds(1),&DCTCPQueue::ResetActivityMonitor,this);
}


DCTCPQueue::~DCTCPQueue ()
{
	NS_LOG_FUNCTION_NOARGS ();
}

void
DCTCPQueue::SetMode (DCTCPQueue::QueueMode mode)
{
	NS_LOG_FUNCTION (this << mode);
	m_mode = mode;
}

DCTCPQueue::QueueMode
DCTCPQueue::GetMode (void)
{
	NS_LOG_FUNCTION (this);
	return m_mode;
}



void
DCTCPQueue::ResetActivityMonitor(void)
{
	m_activityMonitor.clear();
	Simulator::Schedule(MilliSeconds(1),&DCTCPQueue::ResetActivityMonitor,this);
}

bool
DCTCPQueue::DoEnqueue (Ptr<Packet> p)
{

	if (m_bytesInQueue + p->GetSize () >= m_th)
	{
		Mark(p);

	}
	if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes))
	{

		Drop (p);
		return false;
	}
	m_Qpackets.push(p);
	m_bytesInQueue += p->GetSize ();
	return true;
}

Ptr<Packet>
DCTCPQueue::DoDequeue (void)
{
	NS_LOG_FUNCTION (this);
	Ptr<Packet> p;
	if(m_Qpackets.empty()){
		return 0;
	}
	p = m_Qpackets.front();
	m_Qpackets.pop();
	m_bytesInQueue -= p->GetSize ();
	return p;
}




void
DCTCPQueue::Mark (Ptr<Packet> p)
{
	NS_LOG_FUNCTION(this);
	PacketMetadata::ItemIterator metadataIterator = p->BeginItem();
	PacketMetadata::Item item;
	while (metadataIterator.HasNext())
	{
		item = metadataIterator.Next();
		NS_LOG_DEBUG(this << "item name: " << item.tid.GetName());
		if(item.tid.GetName() == "ns3::Ipv4Header")
		{
			NS_LOG_DEBUG("IP packet found");
			Buffer::Iterator i = item.current;
			i.Next (1); // Move to TOS byte
			uint8_t tos = i.ReadU8 ();
			uint8_t ecn = tos & 0x3;
			uint8_t ecncount = tos & 0x1C;
			ecncount = ecncount >> 2;
			if (ecn == 1 || ecn == 2 || ecn == 3)
			{
				ecncount++;
				ecncount = ecncount << 2;
				tos |= 0xF;
				tos &= ecncount;
				tos |= 0x3;
				i.Prev (1); // Go back to the TOS byte after ReadU8()
				i.WriteU8 (tos);
			}
			if ((tos & 0x3) == 0x3)
			{
				NS_LOG_LOGIC ("Marking IP packet");
			}
			else
			{
				NS_LOG_LOGIC ("Unable to mark packet");
			}
			break;
		}
	}
}




Ptr<const Packet>
DCTCPQueue::DoPeek (void) const
{
	NS_LOG_FUNCTION (this);

	Ptr<Packet> p;


	if(m_Qpackets.empty()){
		return 0;
	}
	p = m_Qpackets.front();
	return p;

}
} // namespace ns3

