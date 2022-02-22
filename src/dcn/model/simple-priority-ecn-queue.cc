
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
#include "simple-priority-ecn-queue.h"
//#include "ns3/random-variable-stream.h"


NS_LOG_COMPONENT_DEFINE ("SimplePriorityEcnQueue");

using namespace std;
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SimplePriorityEcnQueue);

TypeId SimplePriorityEcnQueue::GetTypeId (void) 
{
	static TypeId tid = TypeId ("ns3::SimplePriorityEcnQueue")
		.SetParent<Queue> ()
		.AddConstructor<SimplePriorityEcnQueue> ()
		.AddAttribute ("Mode",
				"Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
				EnumValue (QUEUE_MODE_PACKETS),
				MakeEnumAccessor (&SimplePriorityEcnQueue::SetMode),
				MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
					QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
		.AddAttribute ("MaxPackets",
				"The maximum number of packets accepted by this DropTailQueue.",
				UintegerValue (100),
				MakeUintegerAccessor (&SimplePriorityEcnQueue::m_maxPackets),
				MakeUintegerChecker<uint32_t> ())
		.AddAttribute ("MaxBytes",
				"The maximum number of bytes accepted by this DropTailQueue.",
				UintegerValue (100 * 65535),
				MakeUintegerAccessor (&SimplePriorityEcnQueue::m_maxBytes),
				MakeUintegerChecker<uint32_t> ())
		.AddAttribute ("Th",
				"Instantaneous mark length threshold in packets/bytes",
				UintegerValue (5),
				MakeUintegerAccessor (&SimplePriorityEcnQueue::m_th),
				MakeUintegerChecker<uint32_t> ())
		;

	return tid;
}

SimplePriorityEcnQueue::SimplePriorityEcnQueue () :
	Queue (),
	m_AckQueue(),
	m_Q2packets(),
	m_T1packets(),
	m_T2packets(),
	m_bytesInQueue (0),
	m_ECN_active (-1),
	m_maxL2Qsize(1000),
	isset(false),
	nodenumber(0),
	m_max_tocken(1),
	nodeport(0),
	m_bytesInQueue1(0),
	m_bytesInQueue2(0),
	 dequeueturn(1)

{
	NS_LOG_FUNCTION_NOARGS ();
	for (int i = 0; i < m_maxL2Qsize; ++i) {
		m_packets[i] = queue<Ptr<Packet> >();
		m_bytesInQueuePERQ[i] = 0;
	}

}

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


///////////////////////////////////////////

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


///////////////////////////////////////////


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



///////////////////////////////////////////////

class MyTag : public Tag
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
MyTag::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::MyTag")
						.SetParent<Tag> ()
						.AddConstructor<MyTag> ()
						.AddAttribute ("SimpleValue",
								"A simple value",
								EmptyAttributeValue (),
								MakeUintegerAccessor (&MyTag::GetSimpleValue),
								MakeUintegerChecker<uint32_t> ())
								;
	return tid;
}
TypeId
MyTag::GetInstanceTypeId (void) const
{
	return GetTypeId ();
}
uint32_t
MyTag::GetSerializedSize (void) const
{
	return 1;
}
void
MyTag::Serialize (TagBuffer i) const
{
	i.WriteU32 (m_flowNumber);
}
void
MyTag::Deserialize (TagBuffer i)
{
	m_flowNumber = i.ReadU32 ();
}
void
MyTag::Print (std::ostream &os) const
{
	os << "v=" << (uint32_t)m_flowNumber;
}
void
MyTag::SetSimpleValue (uint32_t value)
{
	m_flowNumber = value;
}
uint32_t
MyTag::GetSimpleValue (void) const
{
	return m_flowNumber;
}

/////////////////////////////////////////////////////////

SimplePriorityEcnQueue::~SimplePriorityEcnQueue ()
{
	NS_LOG_FUNCTION_NOARGS ();
}

void
SimplePriorityEcnQueue::SetMode (SimplePriorityEcnQueue::QueueMode mode)
{
	NS_LOG_FUNCTION (this << mode);
	m_mode = mode;
}

SimplePriorityEcnQueue::QueueMode
SimplePriorityEcnQueue::GetMode (void)
{
	NS_LOG_FUNCTION (this);
	return m_mode;
}



bool
SimplePriorityEcnQueue::DoEnqueue (Ptr<Packet> p)
{

//	std::cout<< " ============ enqueue "  <<std::endl;

//	PriorityTag ptag;
//	bool found = p->PeekPacketTag (ptag);
//	uint32_t pTag =  (found) ? ptag.GetPriority () : 0;
//	pTag = pTag & 0x000F;
//	pTag == 3 ||
	if( p->GetSize() < 100){
		m_bytesInQueue += p->GetSize ();
		m_AckQueue.push_back (p);
		return true;
	}


	MyTagTenant tagCopyten;
	p->PeekPacketTag (tagCopyten);
	uint32_t receivedTenant=tagCopyten.GetSimpleValue();

//	MyTag tagCopy;
//    p->PeekPacketTag (tagCopy);
//    uint32_t flow=tagCopy.GetSimpleValue();




//    std::cout<< " tenant  "  << receivedTenant<< " " << flow <<" " << flow << std::endl;







			if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_th))
			{

				Mark (p);
			}
			if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes))
			{

				Drop (p);
				return false;
			}


	if (receivedTenant == 1){
//		if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue1 + p->GetSize () >= m_th/2))
//		{
//			std::cout<< " Ten 1 mark"  << m_bytesInQueue1 + p->GetSize () << std::endl;
//			Mark (p);
//		}
//		if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue1 + p->GetSize () >= m_maxBytes/2))
//		{
//			std::cout<< " Ten 1 drop"  << m_bytesInQueue1 + p->GetSize () << std::endl;
//			Drop (p);
//			return false;
//		}
	

		m_T1packets.push(p);
		m_bytesInQueue1 += p->GetSize ();
	}
	else{
//		if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue2 + p->GetSize () >= m_th/2))
//		{
//			std::cout<< " Ten 2 mark"  << m_bytesInQueue2 + p->GetSize () << std::endl;
//			Mark (p);
//		}
//
//		if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue2 + p->GetSize () >= m_maxBytes/2))
//		{
//			std::cout<< " Ten 2 drop"  << m_bytesInQueue2 + p->GetSize () <<std::endl;
//			Drop (p);
//			return false;
//		}

		m_T2packets.push(p);
		m_bytesInQueue2 += p->GetSize ();
	}





	m_bytesInQueue += p->GetSize ();


	return true;

}

void
SimplePriorityEcnQueue::Mark (Ptr<Packet> p)
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

int
SimplePriorityEcnQueue::Getqueuesize (void){
	int len = 0;
	for(int i=0;i< m_maxL2Qsize;i++){
		len += m_packets[i].size();
	}
	return len + m_AckQueue.size();
}

bool
SimplePriorityEcnQueue::IsLevelqueueEmpty (void){
	for(int i=0;i< m_maxL2Qsize;i++){
		if(!m_packets[i].empty())
			return false;
	}
	return true;
}

bool
SimplePriorityEcnQueue::IsqueueEmpty (void){
	for(int i=0;i< m_maxL2Qsize;i++){
		if(!m_packets[i].empty())
			return false;
	}
	if (!m_AckQueue.empty())
		return false;
	if (!m_T1packets.empty() || !m_T2packets.empty())
			return false;
	return true;
}

void SimplePriorityEcnQueue::fairQueue(Ptr<Packet> p){
	MyTag tagCopy;
	p->PeekPacketTag (tagCopy);
	uint32_t flow=tagCopy.GetSimpleValue();

//	MyTagQueue tagCopyQ;
//	p->PeekPacketTag (tagCopyQ);
//	uint32_t prev_delay=tagCopyQ.GetSimpleValue();

//
//	MyTagTenant tagCopyten;
//	p->PeekPacketTag (tagCopyten);
//	uint32_t receivedTenant=tagCopyten.GetSimpleValue();




	int level=flow % m_maxL2Qsize;

//	m_bytesInQueuePERQ[level] += p->GetSize ();
//	m_prevLoad[level]=prev_delay+m_bytesInQueue;
//	MyTagQueue tagQ;
//	tagQ.SetSimpleValue (m_prevLoad[level]);
//	p->AddPacketTag(tagQ);

//	uint32_t sst=0;
//	for (int i = 0; i < m_maxL2Qsize; ++i) {
//		if(m_packets[i].size()!=0){
//			sst+=m_packets[i].size();
//		}
//	}
//	std::cout << " size "  << sst << std::endl;


	m_packets[level].push(p);

}

Ptr<Packet>
SimplePriorityEcnQueue::DoDequeue (void)
{
	NS_LOG_FUNCTION (this);
//	if (IsEmpty() )
//	{
//		NS_LOG_LOGIC ("Queue empty");
//		return 0;
//	}


//	std::cout<< " ============ dequeue "  <<std::endl;

//
//	if(IsqueueEmpty()){
//		return 0;
//	}

    if(m_T1packets.empty() && m_T2packets.empty() && m_AckQueue.empty()){
                return 0;
        }




	/////////////////////// ACK
	if(m_AckQueue.size () > 0)
	{
		Ptr<Packet> p = m_AckQueue.front ();
		m_AckQueue.pop_front ();
		m_bytesInQueue -= p->GetSize ();
		return p;
	}
	////////////////////////
    Ptr<Packet> pp;
    if(m_T1packets.empty()){
            pp = m_T2packets.front();
            m_T2packets.pop();
            dequeueturn=1;
            m_bytesInQueue2 -= pp->GetSize ();
    }else if(m_T2packets.empty()){
            pp = m_T1packets.front();
            m_T1packets.pop();
            dequeueturn=2;
            m_bytesInQueue1 -= pp->GetSize ();
    }else{
            if (dequeueturn==1){
                    pp = m_T1packets.front();
                    m_T1packets.pop();
                    dequeueturn=2;
                    m_bytesInQueue1 -= pp->GetSize ();
            }else{
                    pp = m_T2packets.front();
                    m_T2packets.pop();
                    dequeueturn=1;
                    m_bytesInQueue2 -= pp->GetSize ();
            }
    }
    m_bytesInQueue -= pp->GetSize ();
    return pp;


}

Ptr<const Packet>
SimplePriorityEcnQueue::DoPeek (void) const
{
	NS_LOG_FUNCTION (this);

	if (m_AckQueue.empty () && m_Q2packets.empty ())
	{
		NS_LOG_LOGIC ("Queue empty");
		return 0;
	}
	if(m_AckQueue.size () > 0)
		{
			Ptr<Packet> p = m_AckQueue.front ();
			return p;
		}
	else
	{


		int mylevel=-1;
		uint32_t val=0;

		for (int level=0;level< m_maxL2Qsize ;level++){
			if(m_packets[level].size() > 0){
				if(val == 0 || val > m_packets[level].size()){
					val=m_packets[level].size();
					mylevel=level;
				}
			}
		}

		Ptr<Packet> pp = m_packets[mylevel].front();
	}

}
} // namespace ns3

