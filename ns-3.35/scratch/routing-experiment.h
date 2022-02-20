#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-l3-protocol.h"


using namespace ns3;
using namespace std;

// #define ENABLE_PACKET_PRINTING

class RoutingExperiment
{
public:
	int m_totalNodes;
	int m_totalFlows;
	double m_nodeSpeed;
	double m_xRange = 300.0;
	double m_yRange = 900.0;
	int m_packetRate = 10;
	uint32_t m_maxPacketCount = 1;

	YansWifiPhyHelper m_wifiPhy;
	Ptr<OutputStreamWrapper> m_tpPerFlowStream;
	Ptr<OutputStreamWrapper> m_ipv4Stream;
	uint128_t m_TxPacketL3 = 0;
	uint128_t m_RxPacketL3 = 0;
	uint128_t m_DropPacketL3 = 0;
	uint128_t m_TxPacketSizeL3 = 0;
	uint128_t m_RxPacketSizeL3 = 0;
	uint128_t m_DropPacketSizeL3 = 0;
	map<uint64_t,Time> m_TxTimeOfPacket;
	Time m_TotalEnd2EndDelay = Seconds(0);
	uint128_t m_TotalLocalDeliveredL3Packet = 0;
	
	long double m_throughput = 0;
	long double m_averageEnd2EndDelay = 0;
	long double m_packetDropRatio = 0;
	long double m_packetDelivaryRatio = 0;

	RoutingExperiment(int n, int nFlows, double nodeSpeed)
	{
		this->m_totalNodes = n;
		this->m_totalFlows = nFlows;
		this->m_nodeSpeed = nodeSpeed;
		NS_LOG_INFO("Total nodes: " << this->m_totalNodes);
		NS_LOG_INFO("Total flows: " << this->m_totalFlows);
		NS_LOG_INFO("Node speed: " << this->m_nodeSpeed);
		m_tpPerFlowStream = Create<OutputStreamWrapper>("throughput-per-flow.dat", std::ios::out);
		m_ipv4Stream = Create<OutputStreamWrapper>("manet-ipv4.tr", std::ios::out);
	}

	virtual void AddMobility(NodeContainer &adhocNodes);
	NetDeviceContainer AddDevie(NodeContainer &adhocNodes);
	Ipv4InterfaceContainer AssignAddress(NetDeviceContainer &adhocDevices);
	void InstallInternetStack(NodeContainer &adhocNodes, Ipv4RoutingHelper *routingHelper);
	virtual void SetUpServer(Ptr<Node> node, uint16_t port, double startTime, double endTime);
	virtual void SetUpClient(Ptr<Node> node, Ipv4Address serverIp, uint16_t serverPort, double startTime, double endTime);
	virtual void AddFlows(NodeContainer &adhocNodes);
	void CalculateThroughput(FlowMonitorHelper &flowmonHelper, double simulationTime);
	long double PacketDeliveryRatioL3()
	{
		return (long double)m_RxPacketL3 * 100.0L / (long double)m_TxPacketL3;
	}
	long double PacketDropRatioL3()
	{
		return (long double)m_DropPacketL3 * 100.0L / (long double)m_TxPacketL3;
	}
	void setXRange(double xRange)
	{
		this->m_xRange = xRange;
	}
	void setYRange(double yRange)
	{
		this->m_yRange = yRange;
	}
	void setPacketRate(int packetRate)
	{
		this->m_packetRate = packetRate;
	}
	void setMaxPacketCount(uint32_t maxPacketCount)
	{
		this->m_maxPacketCount = maxPacketCount;
	}

	void TxTraceL3(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface)
	{
		m_TxPacketL3++;
		m_TxPacketSizeL3 += packet->GetSize();

#ifdef ENABLE_PACKET_PRINTING
		NS_LOG_DEBUG("Node " << ipv4->GetObject<Node>()->GetId());
		NS_LOG_DEBUG("TxTraceL3 at " << Simulator::Now() << " size " << packet->GetSize() << " destination " << packet->ToString());
		NS_LOG_DEBUG("");
#endif 
	}
	void RxTraceL3(Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface)
	{
		m_RxPacketL3++;
		m_RxPacketSizeL3 += packet->GetSize();

#ifdef ENABLE_PACKET_PRINTING
		NS_LOG_DEBUG("Node " << ipv4->GetObject<Node>()->GetId());
		NS_LOG_DEBUG("RxTraceL3 at " << Simulator::Now() << " size " << packet->GetSize() << " destination " << packet->ToString());
		NS_LOG_DEBUG("");
#endif
	}
	void DropTraceL3(const Ipv4Header &header, Ptr<const Packet> packet, Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4, uint32_t interface)
	{
		m_DropPacketL3++;
		m_DropPacketSizeL3 += packet->GetSize();

#ifdef ENABLE_PACKET_PRINTING
		NS_LOG_DEBUG("Node " << ipv4->GetObject<Node>()->GetId());
		NS_LOG_DEBUG("DropTraceL3 at " << Simulator::Now() << " size " << packet->GetSize() << " destination " << packet->ToString());

		// enum DropReason
		// {
		// 	DROP_TTL_EXPIRED = 1,   /**< Packet TTL has expired */
		// 	DROP_NO_ROUTE,   /**< No route to host */
		// 	DROP_BAD_CHECKSUM,   /**< Bad checksum */
		// 	DROP_INTERFACE_DOWN,   /**< Interface is down so can not send packet */
		// 	DROP_ROUTE_ERROR,   /**< Route error */
		// 	DROP_FRAGMENT_TIMEOUT, /**< Fragment timeout exceeded */
		// 	DROP_DUPLICATE  /**< Duplicate packet received */
		// };

		switch (reason)
		{
		case Ipv4L3Protocol::DropReason::DROP_TTL_EXPIRED:
			NS_LOG_DEBUG("DROP_TTL_EXPIRED");
			break;
		case Ipv4L3Protocol::DropReason::DROP_NO_ROUTE:
			NS_LOG_DEBUG("DROP_NO_ROUTE");
			break;
		case Ipv4L3Protocol::DropReason::DROP_BAD_CHECKSUM:
			NS_LOG_DEBUG("DROP_BAD_CHECKSUM");
			break;
		case Ipv4L3Protocol::DropReason::DROP_INTERFACE_DOWN:
			NS_LOG_DEBUG("DROP_INTERFACE_DOWN");
			break;
		case Ipv4L3Protocol::DropReason::DROP_ROUTE_ERROR:
			NS_LOG_DEBUG("DROP_ROUTE_ERROR");
			break;
		case Ipv4L3Protocol::DropReason::DROP_FRAGMENT_TIMEOUT:
			NS_LOG_DEBUG("DROP_FRAGMENT_TIMEOUT");
			break;
		case Ipv4L3Protocol::DropReason::DROP_DUPLICATE:
			NS_LOG_DEBUG("DROP_DUPLICATE");
			break;
		default:
			NS_LOG_DEBUG("Unknown");
			break;
		}

		NS_LOG_DEBUG("");
#endif
	}

	void SendOutgoingTraceL3(const Ipv4Header &header, Ptr<const Packet> packet, uint32_t interface)
	{
		
		#ifdef ENABLE_PACKET_PRINTING
		NS_LOG_DEBUG("SendOutgoingTraceL3 at " << Simulator::Now().GetSeconds()
		 << " size " << packet->GetSize() << "  " << packet->ToString()<<"\n"<<packet->GetUid());
		NS_LOG_DEBUG("");
		#endif
		NS_ASSERT(m_TxTimeOfPacket.find(packet->GetUid()) == m_TxTimeOfPacket.end());
		m_TxTimeOfPacket[packet->GetUid()] = Simulator::Now();
	}

	void LocalDeliverTraceL3(const Ipv4Header &header, Ptr<const Packet> packet, uint32_t interface)
	{
		
		#ifdef ENABLE_PACKET_PRINTING
		NS_LOG_DEBUG("LocalDeliverTraceL3 at " << Simulator::Now().GetSeconds()
		 << " size " << packet->GetSize() << "  " << packet->ToString()<<
		 "\n"<<packet->GetUid()
		 );
		NS_LOG_DEBUG("");
		#endif
		// NS_ASSERT(m_TxTimeOfPacket.find(packet->GetUid()) != m_TxTimeOfPacket.end());
		m_TotalEnd2EndDelay += Simulator::Now() - m_TxTimeOfPacket[packet->GetUid()];
		m_TxTimeOfPacket.erase(packet->GetUid());
		m_TotalLocalDeliveredL3Packet++;
	}

	void Run(double simulationTime, Ipv4RoutingHelper *routingHelper)
	{
		NodeContainer adhocNodes;
		adhocNodes.Create(m_totalNodes);

		NS_LOG_INFO("Created " << m_totalNodes << " nodes.");

		NetDeviceContainer adhocDevices = AddDevie(adhocNodes);
		AddMobility(adhocNodes);
		InstallInternetStack(adhocNodes, routingHelper);
		Ipv4InterfaceContainer adhocInterfaces = AssignAddress(adhocDevices);
		AddFlows(adhocNodes);

		AnimationInterface anim("manet.xml");
		anim.EnablePacketMetadata(true);
		// resize nodes
		for (int i = 0; i < m_totalNodes; i++)
		{
			anim.UpdateNodeSize(i, 10, 10);
		}

		// enable trace generation
		AsciiTraceHelper ascii;
		m_wifiPhy.EnableAsciiAll(ascii.CreateFileStream("manet.tr"));

		// enable flow monitor

		Ptr<FlowMonitor> flowmon;
		FlowMonitorHelper flowmonHelper;
		flowmon = flowmonHelper.InstallAll();

		// add TraceL3 to all nodes
		for (int i = 0; i < m_totalNodes; i++)
		{
			adhocNodes.Get(i)->GetObject<Ipv4L3Protocol>()->TraceConnectWithoutContext("Tx", MakeCallback(&RoutingExperiment::TxTraceL3, this));
			adhocNodes.Get(i)->GetObject<Ipv4L3Protocol>()->TraceConnectWithoutContext("Rx", MakeCallback(&RoutingExperiment::RxTraceL3, this));
			adhocNodes.Get(i)->GetObject<Ipv4L3Protocol>()->TraceConnectWithoutContext("Drop", MakeCallback(&RoutingExperiment::DropTraceL3, this));
			adhocNodes.Get(i)->GetObject<Ipv4L3Protocol>()->TraceConnectWithoutContext("SendOutgoing", MakeCallback(&RoutingExperiment::SendOutgoingTraceL3, this));
			adhocNodes.Get(i)->GetObject<Ipv4L3Protocol>()->TraceConnectWithoutContext("LocalDeliver", MakeCallback(&RoutingExperiment::LocalDeliverTraceL3, this));
		}

		Simulator::Stop(Seconds(simulationTime));
		Simulator::Run();
		Simulator::Destroy();

		CalculateThroughput(flowmonHelper, simulationTime);
		flowmon->SerializeToXmlFile("manet.flowmon", true, true);

		NS_LOG_INFO("Total packets sent: " << (long long)m_TxPacketL3);
		NS_LOG_INFO("Total packets received: " << (long long)m_RxPacketL3);
		NS_LOG_INFO("Total packets dropped: " << (long long)m_DropPacketL3);
		m_packetDelivaryRatio =PacketDeliveryRatioL3();
		NS_LOG_INFO("Packet Delivery Ratio (L3): " << m_packetDelivaryRatio << "%");
		m_packetDropRatio = PacketDropRatioL3();
		NS_LOG_INFO("Packet Drop Ratio (L3): " << m_packetDropRatio << "%");
		NS_LOG_INFO("Average end-to-end delay: " << m_TotalEnd2EndDelay.GetSeconds() / m_TotalLocalDeliveredL3Packet << "s");
		m_averageEnd2EndDelay = m_TotalEnd2EndDelay.GetSeconds() / m_TotalLocalDeliveredL3Packet;
	}
};

void RoutingExperiment::AddMobility(NodeContainer &adhocNodes)
{
	double nodeSpeed = m_nodeSpeed; //  in m/s , default=20
	double nodePause = 0;			// in s
	int64_t streamIndex = 0;		// used to get consistent mobility across scenarios

	ObjectFactory pos;
	pos.SetTypeId("ns3::RandomRectanglePositionAllocator");

	ostringstream ossX;
	ossX << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_xRange << "]";

	ostringstream ossY;
	ossY << "ns3::UniformRandomVariable[Min=0.0|Max=" << m_yRange << "]";

	pos.Set("X", StringValue(ossX.str()));
	pos.Set("Y", StringValue(ossY.str()));

	Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();
	streamIndex += taPositionAlloc->AssignStreams(streamIndex);

	std::stringstream ssSpeed;
	ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
	std::stringstream ssPause;
	ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";

	MobilityHelper mobilityAdhoc;
	mobilityAdhoc.SetMobilityModel("ns3::RandomWaypointMobilityModel",
								   "Speed", StringValue(ssSpeed.str()),
								   "Pause", StringValue(ssPause.str()),
								   "PositionAllocator", PointerValue(taPositionAlloc));
	mobilityAdhoc.SetPositionAllocator(taPositionAlloc);
	mobilityAdhoc.Install(adhocNodes);

	streamIndex += mobilityAdhoc.AssignStreams(adhocNodes, streamIndex);
	NS_UNUSED(streamIndex); // From this point, streamIndex is unused
}

NetDeviceContainer
RoutingExperiment::AddDevie(NodeContainer &adhocNodes)
{
	string phyMode = "DsssRate11Mbps";
	string rate = "2048bps";

	double txp = 7.5;

	// setting up wifi phy and channel using helpers
	WifiHelper wifi;

	// https://www.geckoandfly.com/10041/wireless-wifi-802-11-abgn-router-range-and-distance-comparison/
	wifi.SetStandard(WIFI_STANDARD_80211b);
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
								 "DataMode", StringValue(phyMode),
								 "ControlMode", StringValue(phyMode));

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
	wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(140)); // max range of 802.11b
	// nakagami propagation loss model
	wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel");

	m_wifiPhy.SetChannel(wifiChannel.Create());
	m_wifiPhy.Set("TxPowerStart", DoubleValue(txp));
	m_wifiPhy.Set("TxPowerEnd", DoubleValue(txp));

	WifiMacHelper wifiMac;
	wifiMac.SetType("ns3::AdhocWifiMac");

	NetDeviceContainer adhocDevices = wifi.Install(m_wifiPhy, wifiMac, adhocNodes);

	return adhocDevices;
}

Ipv4InterfaceContainer
RoutingExperiment::AssignAddress(NetDeviceContainer &adhocDevices)
{
	Ipv4AddressHelper addressAdhoc;
	addressAdhoc.SetBase("10.0.0.0", "255.0.0.0");
	Ipv4InterfaceContainer adhocInterfaces;
	for (int i = 0; i < m_totalNodes; i++)
	{
		adhocInterfaces.Add(addressAdhoc.Assign(adhocDevices.Get(i)));
		// addressAdhoc.NewNetwork();
	}

	// print the IP address
	for (int i = 0; i < m_totalNodes; i++)
	{
		NS_LOG_INFO("IP address of node " << i << " is " << adhocInterfaces.GetAddress(i));
	}

	// return addressAdhoc.Assign(adhocDevices);
	return adhocInterfaces;
}

void RoutingExperiment::InstallInternetStack(NodeContainer &adhocNodes, Ipv4RoutingHelper *routingHelper)
{
	InternetStackHelper internet;
	internet.SetRoutingHelper(*routingHelper);
	internet.Install(adhocNodes);
	internet.EnableAsciiIpv4All(m_ipv4Stream);
}

void RoutingExperiment::SetUpServer(Ptr<Node> node, uint16_t port, double startTime, double endTime)
{
	UdpEchoServerHelper echoServer(port);
	ApplicationContainer serverApps = echoServer.Install(node);
	serverApps.Start(Seconds(startTime)); // start time in sec
	serverApps.Stop(Seconds(endTime));	  // end time in sec
}

void TxTraceL4(Ptr<const Packet> p)
{
#ifdef ENABLE_PACKET_PRINTING
	NS_LOG_DEBUG("Packet sent at " << Simulator::Now().GetSeconds() << " : " << p->ToString());
	ostringstream oss;
	p->Print(oss);
	oss << "\n";
	p->PrintPacketTags(oss);
	oss << "\n\n";
	NS_LOG_DEBUG(oss.str());
#endif
}

void RoutingExperiment::SetUpClient(Ptr<Node> node, Ipv4Address serverIp, uint16_t serverPort, double startTime, double endTime)
{
	UdpEchoClientHelper echoClient(serverIp, serverPort);
	echoClient.SetAttribute("MaxPackets", UintegerValue(m_maxPacketCount));
	echoClient.SetAttribute("Interval", TimeValue(Seconds(m_totalFlows * 1.0L / m_packetRate)));
	echoClient.SetAttribute("PacketSize", UintegerValue(100));

	ApplicationContainer clientApps = echoClient.Install(node);
	clientApps.Get(0)->TraceConnectWithoutContext("Tx", MakeCallback(&TxTraceL4));
	clientApps.Start(Seconds(startTime)); // start time in sec
	clientApps.Stop(Seconds(endTime));	  // end time in sec
}
void RoutingExperiment::AddFlows(NodeContainer &adhocNodes)
{
	int toalNodes = adhocNodes.GetN();
	for (int i = 0; i < (m_totalFlows + 1) / 2; i++)
	{
		int server = rand() % toalNodes;
		int client = rand() % toalNodes;
		if (server == client)
		{
			i--;
			continue;
		}
		uint16_t port = rand() | 0x8000;
		Ipv4Address serverIp = adhocNodes.Get(server)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
		SetUpServer(adhocNodes.Get(server), port, 1, 100);

		SetUpClient(adhocNodes.Get(client), serverIp, port, 2, 99);
		NS_LOG_INFO("Flow " << i << ": " << adhocNodes.Get(client)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> " << serverIp << ":" << port);
	}
}

void RoutingExperiment::CalculateThroughput(FlowMonitorHelper &flowmonHelper, double simulationTime)
{
	Ptr<FlowMonitor> flowmon = flowmonHelper.GetMonitor();
	// get flowclassifier from flowmon object
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
	NS_ASSERT(classifier != 0);

	// iterator over every flow and print the flow monitor statistics
	const FlowMonitor::FlowStatsContainer &stats = flowmon->GetFlowStats();
	long double total_rx = 0;
	Time start_time = Seconds(simulationTime);
	Time end_time = Seconds(0);
	for (FlowMonitor::FlowStatsContainer::const_iterator i = stats.begin(); i != stats.end(); ++i)
	{
		FlowId flowId = i->first;
		FlowMonitor::FlowStats fs = i->second;
		Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(flowId);

		if(t.destinationPort == 654) continue;
		if(t.sourcePort == 654) continue;

		ostringstream oss;
		oss << "(" << t.sourceAddress << "," << t.sourcePort << ")->(" << t.destinationAddress << "," << t.destinationPort << ")";
		string flowDesc = oss.str();

		long double throuputPerFlow = fs.rxBytes ? (fs.rxBytes * 8.0 / (fs.timeLastRxPacket.GetSeconds() - fs.timeFirstTxPacket.GetSeconds())) / 1e6 : 0; // in Mbps

		(*m_tpPerFlowStream->GetStream()) << flowDesc << "\t" << throuputPerFlow << "\n";

		NS_LOG_INFO("fid: " << flowId << " bytes: " << fs.rxBytes << " (" << t.sourceAddress << "," << t.sourcePort << ")->(" << t.destinationAddress << "," << t.destinationPort << ")");
		// NS_LOG_INFO("start time: " << fs.timeFirstTxPacket.GetSeconds() << " end time: " << fs.timeLastRxPacket.GetSeconds() << " duration: " << fs.timeLastRxPacket.GetSeconds() - fs.timeFirstTxPacket.GetSeconds());

		total_rx += fs.rxBytes;

		start_time = min(start_time, fs.timeFirstTxPacket);
		end_time = max(end_time, fs.timeLastRxPacket);
	}
	NS_LOG_INFO("Total Rx: " << total_rx);
	NS_LOG_INFO("Start time: " << start_time);
	NS_LOG_INFO("End time: " << end_time);
	long double throughput = total_rx ? total_rx * 8.0 / (end_time.GetSeconds() - start_time.GetSeconds()) / 1e6 : 0;
	m_throughput=throughput;
	NS_LOG_INFO("Average Throughput: " << throughput << " Mbps");
}
