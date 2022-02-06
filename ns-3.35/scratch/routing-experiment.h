#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"

// NS_LOG_COMPONENT_DEFINE("manet");

using namespace ns3;
using namespace std;

class RoutingExperiment
{
public:
	int m_totalNodes;
	int m_totalFlows;
	double m_nodeSpeed;
	YansWifiPhyHelper m_wifiPhy;


	RoutingExperiment(int n, int nFlows, double nodeSpeed)
	{
		this->m_totalNodes = n;
		this->m_totalFlows = nFlows;
		this->m_nodeSpeed = nodeSpeed;
		NS_LOG_INFO("Total nodes: " << this->m_totalNodes);
		NS_LOG_INFO("Total flows: " << this->m_totalFlows);
		NS_LOG_INFO("Node speed: " << this->m_nodeSpeed);
	}

	virtual void AddMobility(NodeContainer &adhocNodes);
	NetDeviceContainer AddDevie(NodeContainer &adhocNodes);
	Ipv4InterfaceContainer AssignAddress(NetDeviceContainer &adhocDevices);
	void InstallInternetStack(NodeContainer &adhocNodes, Ipv4RoutingHelper *routingHelper);
	virtual void SetUpServer(Ptr<Node> node, uint16_t port, double startTime, double endTime);
	virtual void SetUpClient(Ptr<Node> node, Ipv4Address serverIp, uint16_t serverPort, double startTime, double endTime);
	void AddFlows(NodeContainer &adhocNodes);
	void PrintThroughput(FlowMonitorHelper &flowmonHelper, double simulationTime);

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

		Simulator::Stop(Seconds(simulationTime));
		Simulator::Run();
		Simulator::Destroy();

		PrintThroughput(flowmonHelper, simulationTime);
		flowmon->SerializeToXmlFile("manet.flowmon", true, true);
		
	}
};

void RoutingExperiment::AddMobility(NodeContainer &adhocNodes)
{
	double nodeSpeed = m_nodeSpeed; //  in m/s , default=20
	double nodePause = 0;			// in s

	ObjectFactory pos;
	pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
	pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));
	pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=100.0]"));

	Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();

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
	addressAdhoc.SetBase("10.1.1.0", "255.255.255.0");
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
}

void RoutingExperiment::SetUpServer(Ptr<Node> node, uint16_t port, double startTime, double endTime)
{
	UdpEchoServerHelper echoServer(port);
	ApplicationContainer serverApps = echoServer.Install(node);
	serverApps.Start(Seconds(startTime)); // start time in sec
	serverApps.Stop(Seconds(endTime));	  // end time in sec
}

void RoutingExperiment::SetUpClient(Ptr<Node> node, Ipv4Address serverIp, uint16_t serverPort, double startTime, double endTime)
{
	UdpEchoClientHelper echoClient(serverIp, serverPort);
	echoClient.SetAttribute("MaxPackets", UintegerValue(1));
	echoClient.SetAttribute("Interval", TimeValue(Seconds(0.1)));
	echoClient.SetAttribute("PacketSize", UintegerValue(2000));
	ApplicationContainer clientApps = echoClient.Install(node);
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
		uint16_t port =  rand();
		Ipv4Address serverIp = adhocNodes.Get(server)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
		SetUpServer(adhocNodes.Get(server), port, 1, 9);

		SetUpClient(adhocNodes.Get(client), serverIp, port, 2, 8);
		NS_LOG_INFO("Flow " << i << ": " << adhocNodes.Get(client)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> " << serverIp << ":" << port);
	}
}

void RoutingExperiment::PrintThroughput(FlowMonitorHelper &flowmonHelper, double simulationTime)
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
		NS_LOG_INFO("fid: " << flowId << " bytes: " << fs.rxBytes << " (" << t.sourceAddress << "," << t.sourcePort << ")->(" << t.destinationAddress << "," << t.destinationPort << ")");
		
		total_rx += fs.rxBytes;

		start_time = min(start_time, fs.timeFirstTxPacket);
		end_time = max(end_time, fs.timeLastRxPacket);
	}
	NS_LOG_INFO("Total Rx: " << total_rx);
	NS_LOG_INFO("Start time: " << start_time);
	NS_LOG_INFO("End time: " << end_time);
	long double throughput = total_rx ? total_rx * 8.0 / (end_time.GetSeconds() - start_time.GetSeconds()) / 1e6 : 0;
	NS_LOG_INFO("Average Throughput: " << throughput << " Mbps");
}
