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

namespace ns3 {

class TraceContainer;

/**
 * \ingroup queue
 *
 * \brief A FIFO packet queue that drops tail-end packets on overflow
 */
class SimplePriorityEcnQueue: public Queue{
public:
  static TypeId GetTypeId (void);
//int Getqueuesize();
  /**
   * \brief DropTailQueue Constructor
   *
   * Creates a droptail queue with a maximum size of 100 packets by default
   */
  SimplePriorityEcnQueue ();

  virtual ~SimplePriorityEcnQueue();

  /*
   * \brief Set the thresh limits of RED.
   *
   * \param min Minimum thresh in bytes or packets.
   * \param max Maximum thresh in bytes or packets.
   */
  void SetMode (SimplePriorityEcnQueue::QueueMode mode);
  int Getqueuesize (void);
  int Getincast (int,int);

  bool IsqueueEmpty (void);
  bool IsLevelqueueEmpty (void);

  /**
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  SimplePriorityEcnQueue::QueueMode GetMode (void);

  void SetTh (uint32_t th);

  uint32_t m_bytesInQueue;
  uint32_t m_bytesInQueue1;
  uint32_t m_bytesInQueue2;
  uint32_t m_bytesInQueuePERQ[1001];
protected:
  friend class PointToPointNetDevice;
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;
  void fairQueue(Ptr<Packet> p);
  double m_ECN_active;
  int m_maxL2Qsize;

  std::list<Ptr<Packet> > m_AckQueue;
  std::list<Ptr<Packet> > m_Q2packets;					//Needs to improve use a vector here
  std::queue<Ptr<Packet> > m_T1packets;
  std::queue<Ptr<Packet> > m_T2packets;
  int m_max_tocken;
  std::queue<Ptr<Packet> > m_packets [1001];
  double m_prevLoad [1001];
 bool isset;

  uint32_t m_maxPackets;
  uint32_t m_maxBytes;
 int nodenumber;
 int nodeport; 
  QueueMode m_mode;
  void Mark (Ptr<Packet> p);
  int CheckPriority (Ptr<Packet> p);
  int dequeueturn;
  uint32_t m_th;
};

} // namespace ns3

#endif /* SIMPLE_PRIORITY_ECN_H */
