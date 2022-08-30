
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
#include "simple-dctcp-queue.h"
#include <cstdlib>
#include <algorithm>.
#include "ns3/simulator.h"

//#include "ns3/random-variable-stream.h"


NS_LOG_COMPONENT_DEFINE ("SimpleDCTCPQueue");

using namespace std;
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SimpleDCTCPQueue);

TypeId SimpleDCTCPQueue::GetTypeId (void) 
{
	static TypeId tid = TypeId ("ns3::SimpleDCTCPQueue")
				.SetParent<Queue> ()
				.AddConstructor<SimpleDCTCPQueue> ()
				.AddAttribute ("Mode",
				"Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
				EnumValue (QUEUE_MODE_PACKETS),
				MakeEnumAccessor (&SimpleDCTCPQueue::SetMode),
				MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
				QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
				.AddAttribute ("MaxPackets",
				"The maximum number of packets accepted by this DropTailQueue.",
				UintegerValue (1000),
				MakeUintegerAccessor (&SimpleDCTCPQueue::m_maxPackets),
				MakeUintegerChecker<uint32_t> ())
				.AddAttribute ("MaxBytes",
				"The maximum number of bytes accepted by this DropTailQueue.",
				UintegerValue (400),
				MakeUintegerAccessor (&SimpleDCTCPQueue::m_maxBytes),
				MakeUintegerChecker<uint32_t> ())
				.AddAttribute ("Th",
				"Instantaneous mark length threshold in packets/bytes",
				UintegerValue (40),
				MakeUintegerAccessor (&SimpleDCTCPQueue::m_th),
				MakeUintegerChecker<uint32_t> ())
				.AddAttribute ("IsServer",
				"Instantaneous mark length threshold in packets/bytes",
				UintegerValue (0),
				MakeUintegerAccessor (&SimpleDCTCPQueue::m_isServer),
				MakeUintegerChecker<uint32_t> ())
				;

	return tid;
}

SimpleDCTCPQueue::SimpleDCTCPQueue () :
			Queue (),
			m_Qpackets(),
			m_bytesInQueue (0),
			m_ECN_active (-1),
			m_maxlevels(97),
			nodenumber(0),
			m_max_tocken(5),
			nodeport(0),
			inBount(0)
{
	NS_LOG_FUNCTION_NOARGS ();
	for (int i = 0; i < 10; ++i) {
		m_binaryCounter[i].counter=0;
	}
	myfile.open ("log");
//	Simulator::Schedule(MilliSeconds(1),&SimpleDCTCPQueue::ResetActivityMonitor,this);
//	Simulator::Schedule(MicroSeconds(1),&SimpleDCTCPQueue::ResetBinaryCounter,this);
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

SimpleDCTCPQueue::~SimpleDCTCPQueue ()
{
	myfile.close();
	NS_LOG_FUNCTION_NOARGS ();
}

void
SimpleDCTCPQueue::SetMode (SimpleDCTCPQueue::QueueMode mode)
{
	NS_LOG_FUNCTION (this << mode);
	m_mode = mode;
}

SimpleDCTCPQueue::QueueMode
SimpleDCTCPQueue::GetMode (void)
{
	NS_LOG_FUNCTION (this);
	return m_mode;
}

//
//
//void
//SimpleDCTCPQueue::ResetActivityMonitor(void)
//{
//	m_activityMonitor.clear();
//	Simulator::Schedule(MilliSeconds(1),&SimpleDCTCPQueue::ResetActivityMonitor,this);
//}
//

//void
//SimpleDCTCPQueue::setBinaryCounter(int x)
//{
//	bottleneck=5;
//}




bool
SimpleDCTCPQueue::DoEnqueue (Ptr<Packet> p)
{

	MyTagTenant tagCopyTenant;
	p->PeekPacketTag (tagCopyTenant);
	uint32_t TenID=tagCopyTenant.GetSimpleValue();

	MyTagFlow tagCopyFlow;
	p->PeekPacketTag (tagCopyFlow);
	uint32_t floID=tagCopyFlow.GetSimpleValue();



	uint32_t classID=TenID;




	int32_t DropThresh=m_maxBytes;
	int32_t thresh=m_th;
	int booli=0;

//if (m_bytesInQueue + p->GetSize () >= m_th)

	if(classID < 100 && p->GetSize ()!=42  && m_isServer == 0){
		inBount+=1;
		if(inBount >1000){
			inBount=0;
			m_binaryCounter[1].counter=0;
                        m_binaryCounter[2].counter=0;
                        m_binaryCounter[3].counter=0;
		}


		m_binaryCounter[classID].counter+=1;


//		if(Simulator::Now ().GetDouble() > Seconds(0.05).GetDouble()){
//			cout<< " TIME: " << Simulator::Now ().GetSeconds ()<< " inBount: " << inBount <<  "  BinaryCounter1: " << m_binaryCounter[1].counter << "  BinaryCounter2: " << m_binaryCounter[2].counter << "  BinaryCounter3: " << m_binaryCounter[3].counter <<  endl;
//		}

		if(inBount > 20 ){
			if(inBount == m_binaryCounter[classID].counter){
				inBount=0;
				m_binaryCounter[1].counter=0;
				m_binaryCounter[2].counter=0;
				m_binaryCounter[3].counter=0;
			}else{
				double d=(double)m_binaryCounter[classID].counter/(double)inBount;
//				cout<< " =========== " << classID << " cntr " << m_binaryCounter[classID].counter << " inb " << (double)inBount << " d " << d << endl; 
				if(d > 0.47){
					Mark (p);
// cout<< " MARK: " << Simulator::Now ().GetSeconds ()<< " classID " << classID <<" inBount: " << inBount <<  "  BinaryCounter1: " << m_binaryCounter[1].counter << "  BinaryCounter2: " << m_binaryCounter[2].counter << "  BinaryCounter3: " << m_binaryCounter[3].counter <<  endl;
//					m_binaryCounter[classID].counter=0;
//				inBount=0;
                                  m_binaryCounter[classID].counter=0;
                           //     m_binaryCounter[1].counter=0;
                           //     m_binaryCounter[2].counter=0;
                           //     m_binaryCounter[3].counter=0;
				}


				if(inBount-m_binaryCounter[classID].counter < 50){
  //      	              		Drop (p);
//	                	      	m_binaryCounter[classID].counter=0;
	                	}	
			}

		}

		
		
		
		
		
		
//		myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[1].m_mutex_1<< ":" << m_binaryCounter[1].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[1].counter << " MARK " << endl;

//		m_binaryCounter[1].counter+=p->GetSize();
//		m_binaryCounter[2].counter+=p->GetSize();
//		m_binaryCounter[3].counter+=p->GetSize();

//		int x=1;
//		if(Simulator::Now ().GetDouble() > Seconds(0.05).GetDouble()){
//			x=2;
//		}
//		if(Simulator::Now ().GetDouble() > Seconds(0.1).GetDouble()){
//			x=3;
//		}

//
//		if(Simulator::Now ().GetDouble() > Seconds(0.1).GetDouble()){
//			myfile <<  " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID <<  "  BinaryCounter1: " << m_binaryCounter[1].counter << "  BinaryCounter2: " << m_binaryCounter[2].counter << "  BinaryCounter3: " << m_binaryCounter[3].counter << " Tresh " << thresh <<  " bytesInQueue " << m_bytesInQueue <<  endl;
//		}
//
//
//		if(m_binaryCounter[1].counter > thresh){
//			m_binaryCounter[1].counter=0;
//		}
//
//		if(m_binaryCounter[2].counter > thresh){
//			m_binaryCounter[2].counter=0;
//		}
//
//		if(m_binaryCounter[3].counter > thresh){
//			m_binaryCounter[3].counter=0;
//		}
//
//
//
//
//
//
////					if(m_binaryCounter[1].counter < (0-DropThresh)){
////						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[1].m_mutex_1<< ":" << m_binaryCounter[1].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[1].counter << " MARK " << endl;
////						Drop (p);
////						m_binaryCounter[1].counter=0;
////					}
////
////					if(m_binaryCounter[2].counter < (0-DropThresh)){
////						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[2].m_mutex_1<< ":" << m_binaryCounter[2].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[2].counter << " MARK " << endl;
////						Drop (p);
////						m_binaryCounter[2].counter=0;
////					}
////					if(m_binaryCounter[3].counter < (0-DropThresh)){
////						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[3].m_mutex_1<< ":" << m_binaryCounter[2].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[2].counter << " MARK " << endl;
////						Drop (p);
////						m_binaryCounter[3].counter=0;
////					}
//
//
//		if(m_binaryCounter[1].counter < (0-thresh)){
////			myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << "  BinaryCounter: " << m_binaryCounter[classID].counter << " MARK " << endl;
//			booli=1;
//			Mark (p);
//			m_binaryCounter[1].counter=0;
//		}else if(m_binaryCounter[2].counter < (0-thresh)){
////			myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << "  BinaryCounter: " << m_binaryCounter[classID].counter << " MARK " << endl;
//			booli=1;
//			Mark (p);
//			m_binaryCounter[2].counter=0;
//		}else if(m_binaryCounter[3].counter < (0-thresh)){
////			myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << "  BinaryCounter: " << m_binaryCounter[classID].counter << " MARK " << endl;
//			booli=1;
//			Mark (p);
//			m_binaryCounter[3].counter=0;
//		}

	}

//
//	if (m_bytesInQueue + p->GetSize () >= m_th)
//	{
//		Mark (p);
//	}

//	if(classID < 100 && p->GetSize ()!=42 && m_isServer == 0){
//
//
//		if(classID == 1){
//			m_binaryCounter[1].counter+=p->GetSize();
//			m_binaryCounter[1].m_mutex_1=1;
//		}
//		if(classID == 2){
//			m_binaryCounter[1].counter-=p->GetSize();
//			m_binaryCounter[1].m_mutex_2=1;
//		}
//
//		if(classID == 2){
//			m_binaryCounter[2].counter+=p->GetSize();
//			m_binaryCounter[2].m_mutex_1=1;
//		}
//		if(classID == 3){
//			m_binaryCounter[2].counter-=p->GetSize();
//			m_binaryCounter[2].m_mutex_2=1;
//		}
//
//		if(classID == 1){
//			m_binaryCounter[3].counter+=p->GetSize();
//			m_binaryCounter[3].m_mutex_1=1;
//		}
//		if(classID == 3){
//			m_binaryCounter[3].counter-=p->GetSize();
//			m_binaryCounter[3].m_mutex_2=1;
//		}
//		//			cout<< " ClassID " << classID <<  " IsServer " << m_isServer << "  BinaryCounter: " << m_binaryCounter[1] << endl;
//
//
//
//
//		if(m_binaryCounter[1].counter > thresh && m_binaryCounter[1].m_mutex_1==1 && m_binaryCounter[1].m_mutex_2==0){
//			m_binaryCounter[1].counter=0;
//			m_binaryCountem_mutex_1=0;
//			m_binaryCounter[1].m_mutex_2=0;r[1].
//		}
//		if(m_binaryCounter[1].counter < (0-thresh) && m_binaryCounter[1].m_mutex_1==0 && m_binaryCounter[1].m_mutex_2==1){
//			m_binaryCounter[1].counter=0;
//			m_binaryCounter[1].m_mutex_1=0;
//			m_binaryCounter[1].m_mutex_2=0;
//		}
//
//		////////////////////////////////////////////////////
//
//		if(m_binaryCounter[2].counter > thresh && m_binaryCounter[2].m_mutex_1==1 && m_binaryCounter[2].m_mutex_2==0){
//			m_binaryCounter[2].counter=0;
//			m_binaryCounter[2].m_mutex_1=0;
//			m_binaryCounter[2].m_mutex_2=0;
//		}
//		if(m_binaryCounter[2].counter < (0-thresh) && m_binaryCounter[2].m_mutex_1==0 && m_binaryCounter[2].m_mutex_2==1){
//			m_binaryCounter[2].counter=0;
//			m_binaryCounter[2].m_mutex_1=0;
//			m_binaryCounter[2].m_mutex_2=0;
//		}
//
//
//		if(Simulator::Now ().GetDouble() > Seconds(0.048).GetDouble())
//		{
//			myfile <<  " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID <<  " mutex1: " << m_binaryCounter[1].m_mutex_1 <<  ":" << m_binaryCounter[1].m_mutex_2 << " mutex2: " << m_binaryCounter[2].m_mutex_1 <<  ":" << m_binaryCounter[2].m_mutex_2 << "  BinaryCounter1: " << m_binaryCounter[1].counter<< "  BinaryCounter2: " << m_binaryCounter[2].counter << " Tresh " << thresh <<  " bytesInQueue " << m_bytesInQueue <<  endl;
//		}
//		if (m_bytesInQueue + p->GetSize () >= m_th*1000)
//		{
//Mark (p);
//		}
//			if((m_binaryCounter[1].m_mutex_2==1 && m_binaryCounter[1].m_mutex_1==1) || (m_binaryCounter[2].m_mutex_2==1 && m_binaryCounter[2].m_mutex_1==1)){
//				if(m_binaryCounter[1].m_mutex_2==1 && m_binaryCounter[1].m_mutex_1==1){
//					if(m_binaryCounter[1].counter > thresh && classID == 1 ){
//						Mark (p);
//						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[1].m_mutex_1<< ":" << m_binaryCounter[1].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[1].counter << " MARK " << endl;
//						m_binaryCounter[1].counter=0;
//						m_binaryCounter[1].m_mutex_1=0;
//						m_binaryCounter[1].m_mutex_2=0;
//					}
//
//					if(m_binaryCounter[1].counter < (0-thresh) && classID == 2){
//						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[1].m_mutex_1<< ":" << m_binaryCounter[1].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[1].counter << " MARK " << endl;
//						Mark (p);
//						m_binaryCounter[1].counter=0;
//						m_binaryCounter[1].m_mutex_1=0;
//						m_binaryCounter[1].m_mutex_2=0;
//					}
//
//
//					if(m_binaryCounter[1].counter > DropThresh && classID == 1 ){
//						Drop (p);
//						m_binaryCounter[1].counter=0;
//						m_binaryCounter[1].m_mutex_1=0;
//						m_binaryCounter[1].m_mutex_2=0;
//						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[1].m_mutex_1<< ":" << m_binaryCounter[1].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[1].counter << " DROP " << endl;
//						return false;
//					}
//
//					if(m_binaryCounter[1].counter < (0-DropThresh) && classID == 2){
//						Drop (p);
//						m_binaryCounter[1].counter=0;
//						m_binaryCounter[1].m_mutex_1=0;
//						m_binaryCounter[1].m_mutex_2=0;
//						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[1].m_mutex_1<< ":" << m_binaryCounter[1].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[1].counter << " DROP " << endl;
//						return false;
//					}
//
//				}else if(m_binaryCounter[2].m_mutex_2==1 && m_binaryCounter[2].m_mutex_1==1){
//					if(m_binaryCounter[2].counter > thresh && classID == 2){
//						Mark (p);
//						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[2].m_mutex_1<< ":" << m_binaryCounter[2].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[2].counter << " MARK " << endl;
//						m_binaryCounter[2].counter=0;
//						m_binaryCounter[2].m_mutex_1=0;
//						m_binaryCounter[2].m_mutex_2=0;
//					}
//
//					if(m_binaryCounter[2].counter < (0-thresh) && classID == 3){
//						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[2].m_mutex_1<< ":" << m_binaryCounter[2].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[2].counter << " MARK " << endl;
//						Mark (p);
//						m_binaryCounter[2].counter=0;
//						m_binaryCounter[2].m_mutex_1=0;
//						m_binaryCounter[2].m_mutex_2=0;
//					}
//
//
//					if(m_binaryCounter[2].counter > DropThresh && classID == 2 ){
//						Drop (p);
//						m_binaryCounter[2].counter=0;
//						m_binaryCounter[2].m_mutex_1=0;
//						m_binaryCounter[2].m_mutex_2=0;
//						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[2].m_mutex_1<< ":" << m_binaryCounter[2].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[2].counter << " DROP " << endl;
//						return false;
//					}
//
//					if(m_binaryCounter[2].counter < (0-DropThresh) && classID == 3){
//						Drop (p);
//						m_binaryCounter[2].counter=0;
//						m_binaryCounter[2].m_mutex_1=0;
//						m_binaryCounter[2].m_mutex_2=0;
//						myfile << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID << " mutex_1:2 " << m_binaryCounter[2].m_mutex_1<< ":" << m_binaryCounter[2].m_mutex_2 <<  "  BinaryCounter: " << m_binaryCounter[2].counter << " DROP " << endl;
//						return false;
//					}
//
//				}
//
//
//			}
//
//			else{
//				Mark (p);
//			}
//			//		if(m_binaryCounter[2] < thresh/2 && m_binaryCounter[1] > (0-thresh)/2){
//			//			m_binaryCounter[1].m_mutex_1=0;
//			//			m_binaryCounter[1].m_mutex_2=0;
//			//		}
//			//		cout << " TIME: " << Simulator::Now ().GetSeconds () <<   " ClassID " << classID <<  " mutex: " << m_binaryCounter[1].m_mutex_1 <<  ":" << m_binaryCounter[1].m_mutex_2 << "  BinaryCounter: " << m_binaryCounter[1] << endl;
//
//		}



//	if (m_mode == QUEUE_MODE_BYTES && (m_bytesInQueue + p->GetSize () >= m_maxBytes))
//	{
//		cout<< " DROP " << endl;
//		Drop (p);
//		return false;
//	}


	m_Qpackets.push(p);
	m_bytesInQueue += p->GetSize ();
	return true;
}

Ptr<Packet>
SimpleDCTCPQueue::DoDequeue (void)
{
	NS_LOG_FUNCTION (this);
	Ptr<Packet> p;
	if(m_Qpackets.empty()){
		return 0;
	}
	p = m_Qpackets.front();
	m_Qpackets.pop();
	m_bytesInQueue -= p->GetSize ();

	//	std::cout << " m_bytesInQueue " << m_bytesInQueue << std::endl;

	return p;
}




void
SimpleDCTCPQueue::Mark (Ptr<Packet> p)
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
SimpleDCTCPQueue::DoPeek (void) const
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

