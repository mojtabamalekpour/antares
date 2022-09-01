#include <vector>
#include <iomanip>


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/simple-dctcp-queue.h"
#include "ns3/dctcp-queue.h"


#include <fstream>
#include <iostream>






using namespace ns3;
using namespace std;

// Parameters
double          d_LinkSpeed               = 100.0;
string          LinkSpeed                 = "100Gbps";
string          LinkDelay                 = "1us";
double          linkDelay                 = 1e-6;
uint32_t        MachinesPerRack           = 2;
uint32_t        NumberofRacks             = 2;
uint32_t        NumberofSpineSwitches     = 1;
uint32_t        Scale                     = 1;
double          load                      = 0.6;
uint32_t        threshold                 = 4e6;
uint32_t        queueBytes                = 4e7;      // Number of bytes per queue port
uint32_t        initCwnd                  = 2;         // TCP Initial Congestion Window
double          minRto                    = 10000e-6;
uint32_t        segmentSize               = 1460;
bool            LinkUtilization           = 0;
string          StatsFileName             = "dummy.txt";
double          interval                  = 0.050;

uint32_t        incastDegree              = 32;
double          incastFactor              = 0.5;

double          longLoadFactor            = 0.8;
double          longNodeFactor            = 0.3;

uint32_t        FlowSizeShort             = 16*1024;
uint32_t        FlowSizeLong              = 1024 * 1024;

bool            pktspray                  = 0;
bool            dctcp                     = 1;
double          testruntime               = 2;
uint32_t        tenantNumber             = 2;

// Derived Parameters
uint32_t	LongRequestsPerNode;
uint32_t	ShortRequestsPerNode;
uint32_t        IncastRequestsPerNode;
uint32_t        NumberofEndPoints;
uint32_t        NumberofEndPointsLong;
uint32_t	NumberofIncastPoints;
uint32_t        NumberofEndPointsShort;
uint32_t	NumberofEndPointsIncast;
double          InterArrival_long;
double          InterArrival_short;
double          InterArrival_incast;
uint32_t        NumShortFlows;
uint32_t        NumIncastFlows;
uint32_t        NumLongFlows;
double          shortLoadFactor;
double		incastLoadFactor;
uint32_t	NumberofIncastSenders;
uint32_t	NumberofIncastAggregators;

std::ostringstream oss;

// declare the node pointers
vector < Ptr <Node> > nEnd;
vector < Ptr <Node> > nSpine;
vector < Ptr <Node> > nLeaf;

NS_LOG_COMPONENT_DEFINE ("DataCenterSimulator");

std::map<Ptr<Socket>, Time> sockets;

ofstream outfile;

void QueuedPackets(uint32_t oldValue, uint32_t newValue)
{
	NS_LOG_INFO ("Packets in Queue at " << Simulator::Now ().GetSeconds ()<<"are \t"<<newValue);

}


void
QueueStat ()
{
	Config::ConnectWithoutContext ("/NodeList/1/DeviceList/1/$ns3::PointToPointNetDevice/TxQueue/$ns3::DropTailQueue/PacketsInQueue", MakeCallback (&QueuedPackets));
	//  Simulator::Schedule (Seconds(0.1), &QueueStat);

}

void GetQueue(Ptr<SimpleDCTCPQueue> q)
{
	Ptr<Node> rnode  = nLeaf[0];
	Ptr<Ipv4L3Protocol> rnodeip = rnode->GetObject<Ipv4L3Protocol>();

	Ptr<NetDevice> netdev = rnodeip->GetNetDevice(1);
	Ptr<PointToPointNetDevice> ptpnetdev = netdev->GetObject<PointToPointNetDevice>();
	Ptr<Queue> queue = ptpnetdev->GetQueue();
	Ptr<SimpleDCTCPQueue> dtqueue = queue->GetObject<SimpleDCTCPQueue>();
	outfile << " --------------TIME: " << Simulator::Now ().GetSeconds ()  << " Address: " <<  ptpnetdev->GetAddress()<< " PhysicalQLen " << dtqueue->m_bytesInQueue << " m_binaryCounter1 " << dtqueue->m_binaryCounter[1].counter << " m_binaryCounter2 " << dtqueue->m_binaryCounter[2].counter<< " m_binaryCounter3 " << dtqueue->m_binaryCounter[3].counter << std::endl;
	Simulator::Schedule(Seconds(0.005),&GetQueue,q);
}




void Configure_Simulator(){
	bool status = true;
	NS_LOG_INFO ("Configuring the DataCenter Simulator");

	Packet::EnablePrinting ();

	NumberofRacks             =  NumberofRacks         * Scale;
	NumberofSpineSwitches      = NumberofSpineSwitches  * Scale;
	NumberofEndPoints         =  60;//MachinesPerRack       * NumberofRacks;

	NumberofEndPointsLong  = longNodeFactor * NumberofEndPoints; // each rack has one sender and receiver
	NumberofEndPointsShort = (NumberofEndPoints - NumberofEndPointsLong)*(1.0-incastFactor);
	NumberofEndPointsIncast = NumberofEndPoints - NumberofEndPointsLong - NumberofEndPointsShort;
	//  cout<<"Number of Long EndPoints :"<<NumberofEndPointsLong<<endl;
	//  cout<<"Number of Short EndPoints :"<<NumberofEndPointsShort<<endl;
	//  cout<<"Number of Incasts EndPoints :"<<NumberofEndPointsIncast<<endl;


	shortLoadFactor = (1.0 - longLoadFactor)*(1.0 - incastFactor);
	incastLoadFactor = 1.0 - longLoadFactor - shortLoadFactor;
	//  cout<<"Short Load Factor :"<<shortLoadFactor<<endl;
	//  cout<<"Long Load Factor :"<<longLoadFactor<<endl;
	//  cout<<"incast Load Factor :"<<incastLoadFactor<<endl;

	NumberofIncastSenders = NumberofEndPointsIncast - 32;//incastDegree * ((int) (NumberofEndPointsIncast/incastDegree));
	if(NumberofIncastSenders == NumberofEndPointsIncast){
		NumberofIncastSenders = incastDegree * (((int)(NumberofEndPointsIncast/incastDegree)) - 1 );
	}
	NumberofIncastAggregators = NumberofEndPointsIncast - NumberofIncastSenders;
	//  cout<<"Incast Senders are :"<<NumberofIncastSenders<<endl;
	//  cout<<"Incast Aggregators are :"<<NumberofIncastAggregators<<endl;
	if(longLoadFactor > 0)
	{
		InterArrival_long = (NumberofEndPointsLong*FlowSizeLong*8) / (load * longLoadFactor * d_LinkSpeed * NumberofSpineSwitches*NumberofRacks *1000000000);
	}
	else{
		InterArrival_long = 0;
	}
	InterArrival_short = (NumberofEndPointsShort*FlowSizeShort*8) / (load * shortLoadFactor * d_LinkSpeed * NumberofSpineSwitches*NumberofRacks *1000000000);
	InterArrival_incast = (NumberofIncastAggregators*FlowSizeShort*8*incastDegree) / (load * incastLoadFactor * d_LinkSpeed * NumberofRacks * NumberofSpineSwitches *1000000000); // TODO: Include incast degree in the formula

	//  cout<<"Short Flow InterArrival Time :"<<InterArrival_short<<endl;
	//  cout<<"Long Flow InterArrival Time :"<<InterArrival_long<<endl;
	//  cout<<"Incast Flow InterArrival Time :"<<InterArrival_incast<<endl;

	ShortRequestsPerNode = int ((1/InterArrival_short)*testruntime);
	if(InterArrival_long > 0)
	{
		LongRequestsPerNode = int ((1/InterArrival_long)*testruntime);
	}
	else{
		LongRequestsPerNode = 0;
	}
	IncastRequestsPerNode = int ((1/InterArrival_incast)*testruntime);
	//  cout<<"Short Requests Per Node :"<<ShortRequestsPerNode<<endl;
	//  cout<<"Long Requests Per Node :"<<LongRequestsPerNode<<endl;
	//  cout<<"Incast Requests Per Node :"<<IncastRequestsPerNode<<endl;
	//
	//  cout<<"Short Flow Size :"<<FlowSizeShort<<endl;
	//  cout<<"Long Flow Size :"<<FlowSizeLong<<endl;


	double minRtt = (8 * linkDelay);

	/* Set the simple parameters */
	status = Config::SetDefaultFailSafe ("ns3::TcpSocket::SegmentSize", UintegerValue (segmentSize));
	status &= Config::SetDefaultFailSafe ("ns3::TcpSocketBase::FCTFileName", StringValue  ( StatsFileName));
	status &= Config::SetDefaultFailSafe ("ns3::TcpSocketBase::TotalTenants",UintegerValue (tenantNumber));
	status &= Config::SetDefaultFailSafe ("ns3::RttEstimator::InitialEstimation", TimeValue ( Seconds (minRtt)));
	status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::SndBufSize", UintegerValue (1e9)); //Large value
	status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::RcvBufSize", UintegerValue (1e9)); //Large value
	status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::InitialCwnd", UintegerValue (initCwnd));
	status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::ConnTimeout", TimeValue (Seconds (minRtt)));
	status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::ConnCount", UintegerValue (33));
	status &= Config::SetDefaultFailSafe ("ns3::TcpSocket::DelAckCount", UintegerValue (1));
	if(!pktspray)
	{
		status &= Config::SetDefaultFailSafe ("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
	}
	else
	{
		status &= Config::SetDefaultFailSafe ("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
		status &= Config::SetDefaultFailSafe ("ns3::TcpNewReno::ReTxThreshold", UintegerValue(20));
	}
	status &= Config::SetDefaultFailSafe ("ns3::RttEstimator::MinRTO", TimeValue ( Seconds (minRto)));
	if(!dctcp)
	{
		status &= Config::SetDefaultFailSafe ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
	}
	else
	{
		status &= Config::SetDefaultFailSafe ("ns3::Ipv4L3Protocol::ECN", BooleanValue (true));
		status &= Config::SetDefaultFailSafe ("ns3::TcpL4Protocol::SocketType", TypeIdValue (Dctcp::GetTypeId ()));
	}
	NS_ASSERT(status);
}

void
PrintTime (void)
{
	NS_LOG_UNCOND (Simulator::Now ());
	Simulator::Schedule (Seconds(0.005), &PrintTime);
}

void Build_Topology()
{
	NS_LOG_INFO ("Building the Topology");
	for (uint32_t i=0;i<NumberofEndPoints;i++)
		nEnd.push_back(CreateObject<Node> ());
	NS_LOG_INFO("Allocated " << nEnd.size() << " EndPoint nodes..");
	for (uint32_t i=0;i<NumberofSpineSwitches;i++)
	{
		nSpine.push_back(CreateObject<Node> ());
	}
	NS_LOG_INFO("Allocated " << nSpine.size() << " Spine switches..");
	for (uint32_t i=0;i<NumberofRacks;i++)
		nLeaf.push_back(CreateObject<Node> ());
	NS_LOG_INFO("Allocated " << nLeaf.size() << " Leaf switches..");

	NS_LOG_INFO("Installing IP stack on the nodes ..");
	InternetStackHelper internet;
	for (uint32_t i=0;i<NumberofRacks;i++)
	{
		internet.Install (nLeaf[i]);
	}
	for (uint32_t i=0;i<NumberofSpineSwitches;i++)
		internet.Install (nSpine[i]);
	NS_LOG_INFO("Installed IP stack on all switches...");
	for (uint32_t i=0;i<NumberofEndPoints;i++)
		internet.Install(nEnd[i]);
	NS_LOG_INFO("Installed TCP/IP stack on all servers...");

	NS_LOG_INFO ("Create channels.");
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute	("DataRate",  StringValue (LinkSpeed));
	p2p.SetChannelAttribute	("Delay",     StringValue (LinkDelay));
	std::string queueType = "ns3::DropTailQueue";
	std::string n1 = "Mode";
	Ptr<AttributeValue> v1 = Create<EnumValue> (DropTailQueue::QUEUE_MODE_BYTES);
	std::string n2 = "MaxBytes";
	Ptr<AttributeValue> v2 = Create<UintegerValue> (queueBytes);
	std::string n3 = "";
	Ptr<AttributeValue> v3 = Create<EmptyAttributeValue> ();

	if(dctcp) {
		n3 = "Th";
		queueType = "ns3::SimpleDCTCPQueue";
		v3 = Create<UintegerValue> (threshold);
	}

	p2p.SetQueue  (queueType,
			n1, *v1,
			n2, *v2,
			n3, *v3);




	Ipv4AddressHelper ipv4;
	ipv4.SetBase ("10.0.0.0", "255.255.255.0");
	NetDeviceContainer devices;
	Ipv4InterfaceContainer interfaces;


	for (uint32_t i=0;i<20;i++){
		devices = p2p.Install (nEnd[i],nLeaf[0]);
		interfaces = ipv4.Assign(devices);
		ipv4.NewNetwork();
	}






	Ptr<Ipv4L3Protocol> rnodeip = nLeaf[0]->GetObject<Ipv4L3Protocol>();
	Ptr<NetDevice> netdev = rnodeip->GetNetDevice(1);
	Ptr<PointToPointNetDevice> ptpnetdev = netdev->GetObject<PointToPointNetDevice>();
	Ptr<Queue> queue = ptpnetdev->GetQueue();
	Ptr<SimpleDCTCPQueue> dtqueue = queue->GetObject<SimpleDCTCPQueue>();
	Simulator::Schedule(Seconds(0),&GetQueue,dtqueue);



	NS_LOG_INFO	("Populate routing tables.");
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

}
void SetupServerTraffic (Ptr<Node> Nd, uint16_t appPort, Time startTime,Time endTime ,uint8_t tenant,uint8_t flow)
{
	PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), appPort));
	sink.SetAttribute ("FlowID", UintegerValue (flow));
	sink.SetAttribute ("TenantID", UintegerValue (tenant));
	ApplicationContainer sinkApps = sink.Install (Nd);
	sinkApps.Start(startTime);
	sinkApps.Stop(endTime);
}

void SetupClientTraffic(Ptr<Node> Nd,Ptr<Node> ServerNode, uint32_t txsize, uint16_t appPort, Time startTime,Time endTime,uint8_t tenant,uint8_t flow){

	BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress(ServerNode->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), appPort));
	source.SetAttribute ("MaxBytes", UintegerValue (0)); //txsize
	source.SetAttribute ("SendSize", UintegerValue (txsize));
	source.SetAttribute ("FlowID", UintegerValue (flow));
	source.SetAttribute ("TenantID", UintegerValue (tenant));
	ApplicationContainer sourceApps = source.Install (Nd);
	sourceApps.Start(startTime);
	sourceApps.Stop(endTime);
}

double  AppDelay(){

	Ptr<UniformRandomVariable> ApplicationDelay = CreateObject<UniformRandomVariable> ();
	ApplicationDelay->SetAttribute ("Min", DoubleValue (1));
	ApplicationDelay->SetAttribute ("Max", DoubleValue (100));
	double  delay;
	uint8_t chance = ApplicationDelay->GetInteger(1,100);
	if (chance < 80)
	{
		delay =  0.00;
	}
	else
	{
		delay = 0.000082;
	}
	return delay;
}

void Setup_Workload(){
	vector <uint32_t> AppIdxLong;
	vector <uint32_t> AppIdxShort;
	vector <uint32_t> AppIdxIncast;
	vector <uint32_t> IncastSenders;
	vector <uint32_t> IncastAggregators;
	map<uint32_t,vector <uint16_t> > Map_Port;
	map<uint32_t,vector <uint32_t> > Map_Client;
	Map_Port.clear();
	Map_Client.clear();
	uint16_t appPort;
	AppIdxShort.clear();
	AppIdxLong.clear();
	AppIdxIncast.clear();
	IncastSenders.clear();
	IncastAggregators.clear();
	// Ptr<ExponentialRandomVariable> ApplicationDelay = CreateObject<ExponentialRandomVariable> ();
	//ApplicationDelay->SetAttribute ("Mean", DoubleValue (0.000082));
	// ApplicationDelay->SetAttribute ("Bound", DoubleValue (0.0012));
	Ptr<UniformRandomVariable> UniformlyRandomly = CreateObject<UniformRandomVariable> ();
	UniformlyRandomly->SetAttribute ("Min", DoubleValue (0));
	UniformlyRandomly->SetAttribute ("Max", DoubleValue (65535));
	uint32_t ServerNodeIdx,ClientNodeIdx;
	vector<Time> Prev_Start;
	Prev_Start.clear();
	Ptr<ExponentialRandomVariable> DelayRandomlyShort = CreateObject<ExponentialRandomVariable> ();
	Ptr<ExponentialRandomVariable> DelayRandomlyLong = CreateObject<ExponentialRandomVariable> ();
	Ptr<ExponentialRandomVariable> DelayRandomlyIncast = CreateObject<ExponentialRandomVariable> ();
	DelayRandomlyShort->SetAttribute ("Mean", DoubleValue(InterArrival_short));
	DelayRandomlyLong->SetAttribute ("Mean", DoubleValue(InterArrival_long));
	DelayRandomlyIncast->SetAttribute ("Mean", DoubleValue(InterArrival_incast));


	Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
	uint32_t min = 0.0;
	uint32_t max = MachinesPerRack-1;
	uint32_t appNode = 0.0;
	for(uint32_t i=0;i<NumberofRacks;i++)
	{

		x->SetAttribute ("Min", DoubleValue (min));
		x->SetAttribute ("Max", DoubleValue (max));
		for(uint32_t j=0;j<MachinesPerRack*longNodeFactor;j++)
		{
			do {
				appNode = x->GetInteger ();
			} while ((find(AppIdxShort.begin(),AppIdxShort.end(),appNode) != AppIdxShort.end()) || (find(AppIdxLong.begin(),AppIdxLong.end(),appNode) != AppIdxLong.end()));
			AppIdxLong.push_back(appNode);
		}
		for(uint32_t j=0;j<MachinesPerRack*(1.0-longNodeFactor)*(1.0 - incastFactor);j++)
		{
			do {
				appNode = x->GetInteger ();
			} while ((find(AppIdxShort.begin(),AppIdxShort.end(),appNode) != AppIdxShort.end()) || (find(AppIdxLong.begin(),AppIdxLong.end(),appNode) != AppIdxLong.end()));
			AppIdxShort.push_back(appNode);
		}
		min = min + MachinesPerRack;
		max = max + MachinesPerRack;
	}

	for (uint32_t i=0;i<NumberofEndPoints;i++)
	{
		bool IncastId = true;
		Prev_Start.push_back(Seconds(0.0));
		for(uint32_t j=0;j<AppIdxLong.size();j++)
		{
			if(AppIdxLong[j] == i)
			{
				IncastId = false;
			}
		}
		for(uint32_t j=0;j<AppIdxShort.size();j++)
		{
			if(AppIdxShort[j] == i)
			{
				IncastId = false;
			}
		}
		if(IncastId)
		{

			AppIdxIncast.push_back(i);
		}

	}
	NS_ASSERT(AppIdxLong.size() == NumberofEndPointsLong);
	NS_ASSERT(AppIdxShort.size() == NumberofEndPointsShort);
	NS_ASSERT(AppIdxIncast.size() == NumberofEndPointsIncast);


	// dumbbell

	int m_tenant = 1;
	int m_flow_counter=1;

	////////////////////////////////////
	double startTime=0;
	double endTime=2.5;
	SetupServerTraffic(nEnd[0],2000+m_flow_counter,Seconds(startTime),Seconds(endTime), m_tenant,m_flow_counter);
	SetupClientTraffic(nEnd[m_flow_counter],nEnd[0],FlowSizeLong,2000+m_flow_counter,Seconds(startTime),Seconds(endTime),m_tenant,m_flow_counter);

	////////////////////////////////////
	startTime=0.5;
	endTime=2;
	m_tenant++;

	m_flow_counter++;
	SetupServerTraffic(nEnd[0],2000+m_flow_counter,Seconds(startTime),Seconds(endTime), m_tenant,m_flow_counter);
	SetupClientTraffic(nEnd[m_flow_counter],nEnd[0],FlowSizeLong,2000+m_flow_counter,Seconds(startTime),Seconds(endTime),m_tenant,m_flow_counter);

	m_flow_counter++;
	SetupServerTraffic(nEnd[0],2000+m_flow_counter,Seconds(startTime),Seconds(endTime), m_tenant,m_flow_counter);
	SetupClientTraffic(nEnd[m_flow_counter],nEnd[0],FlowSizeLong,2000+m_flow_counter,Seconds(startTime),Seconds(endTime),m_tenant,m_flow_counter);

	////////////////////////////////////
	startTime=1;
	endTime=1.5;
	m_tenant++;
	m_flow_counter++;
	SetupServerTraffic(nEnd[0],2000+m_flow_counter,Seconds(startTime),Seconds(endTime), m_tenant,m_flow_counter);
	SetupClientTraffic(nEnd[m_flow_counter],nEnd[0],FlowSizeLong,2000+m_flow_counter,Seconds(startTime),Seconds(endTime),m_tenant,m_flow_counter);


	m_flow_counter++;
	SetupServerTraffic(nEnd[0],2000+m_flow_counter,Seconds(startTime),Seconds(endTime), m_tenant,m_flow_counter);
	SetupClientTraffic(nEnd[m_flow_counter],nEnd[0],FlowSizeLong,2000+m_flow_counter,Seconds(startTime),Seconds(endTime),m_tenant,m_flow_counter);


	m_flow_counter++;
	SetupServerTraffic(nEnd[0],2000+m_flow_counter,Seconds(startTime),Seconds(endTime), m_tenant,m_flow_counter);
	SetupClientTraffic(nEnd[m_flow_counter],nEnd[0],FlowSizeLong,2000+m_flow_counter,Seconds(startTime),Seconds(endTime),m_tenant,m_flow_counter);

//	m_tenant = 3;
//	m_flow_counter++;
//	i++;
//
//	SetupServerTraffic(nEnd[0],2000+m_flow_counter,Seconds(0), m_tenant,m_flow_counter);
//	SetupClientTraffic(nEnd[i],nEnd[0],FlowSizeLong,2000+m_flow_counter,Seconds(0),m_tenant,m_flow_counter);
//


//	m_tenant = 3;
//	m_flow_counter++;
//	i++;
//
//	SetupServerTraffic(nEnd[0],2000+m_flow_counter,Seconds(0), m_tenant,m_flow_counter);
//	SetupClientTraffic(nEnd[i],nEnd[0],FlowSizeLong,2000+m_flow_counter,Seconds(0.4),m_tenant,m_flow_counter);
//
//	m_tenant = 4;
//	m_flow_counter++;
//	i++;
//
//	SetupServerTraffic(nEnd[0],2000+m_flow_counter,Seconds(0), m_tenant,m_flow_counter);
//	SetupClientTraffic(nEnd[i],nEnd[0],FlowSizeLong,2000+m_flow_counter,Seconds(0.6),m_tenant,m_flow_counter);



//	m_tenant = 2;
//	m_flow_counter++;
//	i++;
//
//	SetupServerTraffic(nEnd[0],2000+m_flow_counter,Seconds(0), m_tenant,m_flow_counter);
//	SetupClientTraffic(nEnd[i],nEnd[0],FlowSizeLong,2000+m_flow_counter,Seconds(0),m_tenant,m_flow_counter);



//	m_tenant = 1;
//	m_flow_counter++;
//	i++;
//
//	SetupServerTraffic(nEnd[i+10],2000+m_flow_counter,Seconds(0), m_tenant,m_flow_counter);
//	SetupClientTraffic(nEnd[i],nEnd[i+10],FlowSizeLong,2000+m_flow_counter,Seconds(0),m_tenant,m_flow_counter);






//	double interv=0;
//	int des = 9;
//	int aha;
//
//
//	aha = 0;
//	SetupServerTraffic(nEnd[des],2000+aha,Seconds(1*interv), 1,aha);
//	SetupClientTraffic(nEnd[1],nEnd[des],FlowSizeLong,2000+aha,Seconds(1*interv),1,aha);
//
//	aha = 1;
//	SetupServerTraffic(nEnd[des],2000+aha,Seconds(1*interv), 2,aha);
//	SetupClientTraffic(nEnd[2],nEnd[des],FlowSizeLong,2000+aha,Seconds(1*interv),2,aha);
//
//	aha = 2;
//	SetupServerTraffic(nEnd[des],2000+aha,Seconds(1*interv), 3,aha);
//	SetupClientTraffic(nEnd[3],nEnd[des],FlowSizeLong,2000+aha,Seconds(1*interv),3,aha);


//	interv=0.5;
//
//	aha = 3;
//	SetupServerTraffic(nEnd[des],2000+aha,Seconds(1*interv), 2,aha);
//	SetupClientTraffic(nEnd[4],nEnd[des],FlowSizeLong,2000+aha,Seconds(1*interv),2,aha);




	//
	//
	//			aha = 4;
	//			SetupServerTraffic(nEnd[des],2000+aha,Seconds(2*interv), 3,aha);
	//			SetupClientTraffic(nEnd[4],nEnd[des],FlowSizeLong,2000+aha,Seconds(2*interv),3,aha);
	//
	//
	//			aha = 5;
	//			SetupServerTraffic(nEnd[des],2000+aha,Seconds(2*interv),3,aha);
	//			SetupClientTraffic(nEnd[5],nEnd[des],FlowSizeLong,2000+aha,Seconds(2*interv),3,aha);
	//
	//			aha = 6;
	//			SetupServerTraffic(nEnd[des],2000+aha,Seconds(3*interv), 4,aha);
	//			SetupClientTraffic(nEnd[6],nEnd[des],FlowSizeLong,2000+aha,Seconds(3*interv),4,aha);
	//
	//
	//			aha = 7;
	//			SetupServerTraffic(nEnd[des],2000+aha,Seconds(3*interv), 4,aha);
	//			SetupClientTraffic(nEnd[7],nEnd[des],FlowSizeLong,2000+aha,Seconds(3*interv),4,aha);
	//
	//
	//			aha = 8;
	//			SetupServerTraffic(nEnd[des],2000+aha,Seconds(3*interv), 4,aha);
	//			SetupClientTraffic(nEnd[8],nEnd[des],FlowSizeLong,2000+aha,Seconds(3*interv),4,aha);
	//
	//
	//			aha = 9;
	//			SetupServerTraffic(nEnd[des],2000+aha,Seconds(3*interv), 4,aha);
	//			SetupClientTraffic(nEnd[8],nEnd[des],FlowSizeLong,2000+aha,Seconds(3*interv),4,aha);
	//



}





void SetupServer(Ptr<Node> rnode)
{
	bool set_successful = true;
	Ptr<Ipv4L3Protocol> rnodeip = rnode->GetObject<Ipv4L3Protocol>();

	//Configure output queues
	for(uint32_t i = 0; i < rnodeip->GetNInterfaces(); i++) {
		Ptr<NetDevice> netdev = rnodeip->GetNetDevice(i);
		Ptr<PointToPointNetDevice> ptpnetdev = netdev->GetObject<PointToPointNetDevice>();
		if(!ptpnetdev) {
			continue;
		}

		Ptr<Queue> queue = ptpnetdev->GetQueue();

		if(!dctcp){
			Ptr<DropTailQueue> dtqueue = queue->GetObject<DropTailQueue>();
			set_successful &= dtqueue->SetAttributeFailSafe("Mode",      EnumValue(DropTailQueue::QUEUE_MODE_BYTES));
			set_successful &= dtqueue->SetAttributeFailSafe("MaxBytes",  UintegerValue(4 * 1024 * 1024)); //4 MB typical to accomodate
		}
		else{
			Ptr<SimpleDCTCPQueue> dtqueue = queue->GetObject<SimpleDCTCPQueue>();
			set_successful &= dtqueue->SetAttributeFailSafe("Mode",      EnumValue(DropTailQueue::QUEUE_MODE_BYTES));
			set_successful &= dtqueue->SetAttributeFailSafe("MaxBytes",  UintegerValue(0)); //4 MB typical to accomodate
			set_successful &= dtqueue->SetAttributeFailSafe("Th",      UintegerValue(0));
			set_successful &= dtqueue->SetAttributeFailSafe("IsServer",      UintegerValue(1));
//			Simulator::Schedule(Seconds(0),&GetQueue,dtqueue);




		}
	}

	NS_ASSERT(set_successful);
}

void Setup_Servers()
{
	for (uint32_t i=0;i<NumberofEndPoints;i++)
	{
		SetupServer(nEnd[i]);
	}
}

void MeasureLink (Ptr<OutputStreamWrapper> stream, Ptr<Node> rnode)
{
	double link_utilization;

	Ptr<Ipv4L3Protocol> rnodeip = rnode->GetObject<Ipv4L3Protocol>();
	NS_ASSERT(rnodeip);

	for(uint32_t j = 0; j < rnodeip->GetNInterfaces(); j++)
	{
		Ptr<NetDevice> netdev = rnodeip->GetNetDevice(j);
		Ptr<PointToPointNetDevice> ptpnetdev = netdev->GetObject<PointToPointNetDevice>();
		if(!ptpnetdev) {
			continue;
		}
		NS_ASSERT(ptpnetdev);
		link_utilization = (double) (ptpnetdev->m_bytes_transmitted * 8)/ interval;
		ptpnetdev->m_bytes_transmitted = 0;
		*stream->GetStream ()<<"Node: "<< rnode->GetId()<<"\tInterface:"<< j <<"\tlink utilization:"<<link_utilization<<endl;
	}
}

void MeasureLinkUtilization(Ptr<OutputStreamWrapper> stream)
{
	for (uint32_t i=0;i<NumberofSpineSwitches;i++)
	{
		MeasureLink(stream, nSpine[i]);
	}
	Simulator::Schedule(Seconds(interval),&MeasureLinkUtilization,stream);
}

void ProbeApps(uint32_t junk)
{
	bool stopnow = true;
	int total = 0;
	uint32_t count = 0;
	for (uint32_t i=0;i<NumberofEndPoints;i++)
	{
		for (uint32_t j=0;j<nEnd[i]->GetNApplications();j++)
		{
			Ptr <Application> Traceapp = nEnd[i]->GetApplication(j);
			if (Traceapp)
			{
				Ptr <BulkSendApplication> Tracebulk = Traceapp->GetObject<BulkSendApplication>();
				if (Tracebulk)
				{
					total++;
					if (Tracebulk->m_application_ended == false)
					{
						stopnow = false;
					}
					else
					{
						count++;
					}
				}
			}
		}
	}
	if (stopnow)
	{
		NS_ASSERT(count == (NumShortFlows + NumLongFlows + NumIncastFlows));
		Simulator::Stop();
	}
	else
	{
		Simulator::Schedule(Seconds(interval),&ProbeApps,0);
	}
}

int
main (int argc, char *argv[])
{
	CommandLine cmd;
	cmd.AddValue ("load",         	  "Controls the n/w load",              load);
	cmd.AddValue ("threshold",              "DCTCP threshold",                    threshold);
	cmd.AddValue ("dctcp",        	  "Enable DCTCP",            		dctcp);
	cmd.AddValue ("runtime",                "Test Runtime in Seconds",  		testruntime);
	cmd.AddValue ("random",		  "Load Balancing : Random ECMP", 	pktspray);
	cmd.AddValue ("incastdegree", 	  "Degree of Incast",			incastDegree);
	cmd.AddValue ("filename",               "FileName to Dump Stats",	        StatsFileName);
	cmd.AddValue ("linkutilization",        "Measure Link Utilization",      	LinkUtilization);
	cmd.Parse (argc, argv);
	//    cout<<"Load "<<load<<endl;
	//    cout<<"threshold "<<threshold<<endl;
	//  cout<<"incast "<< incastDegree<<endl;
	Time::SetResolution (Time::NS);
	LogComponentEnable ("DataCenterSimulator", LOG_ALL);
	NS_LOG_INFO ("DataCenter Simulation");

	outfile.open(StatsFileName.c_str());
	Configure_Simulator();

	Build_Topology();

	Setup_Servers();

	NS_LOG_INFO ("Setting Up Workload");

	Setup_Workload();

	NS_LOG_INFO ("Starting Simulation");


	//Simulator::Schedule (Seconds(0.1), &QueueStat);
	if(LinkUtilization)
	{
		oss << "Util." <<StatsFileName ;
		AsciiTraceHelper asciiTraceHelper;
		Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (oss.str ());
		//		Simulator::Schedule(Seconds(interval),&MeasureLinkUtilization,stream);
		Simulator::Schedule(Seconds(interval),&ProbeApps,0);
	}

	Simulator::Run();
	Simulator::Destroy ();

	outfile.close();
	NS_LOG_INFO ("Simulation Finished");

	return 0;
}


