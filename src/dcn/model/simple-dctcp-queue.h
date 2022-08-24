/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef SIMPLE_PRIORITY_ECN_H
#define SIMPLE_PRIORITY_ECN_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue.h"
#include <iterator>
#include <list>
#include <fstream>
#include <iostream>

namespace ns3 {

class TraceContainer;

struct BinaryCounter{
	uint32_t m_mutex_1=0;
	uint32_t m_mutex_2=0;
	int32_t counter=0;
};
/**
 * \ingroup queue
 *
 * \brief A FIFO packet queue that drops tail-end packets on overflow
 */
class SimpleDCTCPQueue: public Queue{
public:
  static TypeId GetTypeId (void);
  /**
   * \brief DropTailQueue Constructor
   *
   * Creates a droptail queue with a maximum size of 100 packets by default
   */
  SimpleDCTCPQueue ();

  virtual ~SimpleDCTCPQueue();

  /*
   * \brief Set the thresh limits of RED.
   *
   * \param min Minimum thresh in bytes or packets.
   * \param max Maximum thresh in bytes or packets.
   */
  void SetMode (SimpleDCTCPQueue::QueueMode mode);
  int Getqueuesize (void);
  int Getincast (int,int);

  bool IsqueueEmpty (void);
  bool IsLevelqueueEmpty (void);

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  SimpleDCTCPQueue::QueueMode GetMode (void);

  void SetTh (uint32_t th);

  uint32_t m_bytesInQueue;

//  std::list <uint32_t> m_activityMonitor;
//  uint32_t m_bytesInQueue1;
//  uint32_t m_bytesInQueue2;
//  uint32_t m_bytesInQueuePERQ[1001];
//  void setBinaryCounter(int x);
protected:
  int bottleneck;
  friend class PointToPointNetDevice;
  virtual bool DoEnqueue (Ptr<Packet> p);
//  void ResetActivityMonitor(void);

  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;
  double m_ECN_active;
  int m_maxlevels;

//  std::list<Ptr<Packet> > m_Q1packets;
//  std::list<Ptr<Packet> > m_Qpackets;					//Needs to improve use a vector here
  std::queue<Ptr<Packet> > m_Qpackets;
//  std::queue<Ptr<Packet> > m_T1packets;
//  std::queue<Ptr<Packet> > m_T2packets;
  int m_max_tocken;

//  std::queue<Ptr<Packet> > m_packets [1001];
//  double m_prevLoad [1001];
// bool isset;

  uint32_t m_maxPackets;
  uint32_t m_maxBytes;
 int nodenumber;
 int nodeport; 
  QueueMode m_mode;
  void Mark (Ptr<Packet> p);
  uint32_t m_th;
  uint32_t m_isServer;

  	BinaryCounter m_binaryCounter[10];
  	uint32_t inBount;
  	std::ofstream myfile;

};

} // namespace ns3

#endif /* SIMPLE_PRIORITY_ECN_H */
