#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"

NS_LOG_COMPONENT_DEFINE("manet");

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
	void SetUpServer(Ptr<Node> node, uint16_t port, double startTime, double endTime);
	void SetUpClient(Ptr<Node> node, Ipv4Address serverIp, uint16_t serverPort, double startTime, double endTime);
	void AddFlows(NodeContainer &adhocNodes);

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
		for(int i=0;i<m_totalNodes ; i++)
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

		flowmon->SerializeToXmlFile("manet.flowmon", true, true);
	}
};

int main(int argc, char *argv[])
{

	int n = 2;
	int nflows = 2;
	// int packetRate=10; // number of packet per sec
	double nodeSpeed = 20;		   // in m/s
	double simulationTime = 100.0; // in s

	LogComponentEnable("manet", LOG_LEVEL_ALL);
	LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

	CommandLine cmd(__FILE__);
	cmd.AddValue("n", "total nodes in network", n);
	cmd.Parse(argc, argv);

	RoutingExperiment experiment(n, nflows, nodeSpeed);

	// disable hellopacket in all node of routing
	Config::SetDefault("ns3::aodv::RoutingProtocol::EnableHello", BooleanValue(false));

	experiment.Run(simulationTime, new AodvHelper());
}

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
	wifi.SetStandard(WIFI_STANDARD_80211b);

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
	m_wifiPhy.SetChannel(wifiChannel.Create());

	// Add a mac and disable rate control
	WifiMacHelper wifiMac;
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
								 "DataMode", StringValue(phyMode),
								 "ControlMode", StringValue(phyMode));

	m_wifiPhy.Set("TxPowerStart", DoubleValue(txp));
	m_wifiPhy.Set("TxPowerEnd", DoubleValue(txp));

	wifiMac.SetType("ns3::AdhocWifiMac");
	return wifi.Install(m_wifiPhy, wifiMac, adhocNodes);
}

Ipv4InterfaceContainer
RoutingExperiment::AssignAddress(NetDeviceContainer &adhocDevices)
{
	Ipv4AddressHelper addressAdhoc;
	addressAdhoc.SetBase("10.1.1.0", "255.255.255.0");
	return addressAdhoc.Assign(adhocDevices);
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
	echoClient.SetAttribute("PacketSize", UintegerValue(1024));
	ApplicationContainer clientApps = echoClient.Install(node);
	clientApps.Start(Seconds(startTime)); // start time in sec
	clientApps.Stop(Seconds(endTime));	  // end time in sec
}
void RoutingExperiment::AddFlows(NodeContainer &adhocNodes)
{
	int toalNodes = adhocNodes.GetN();
	for (int i = 0; i < (m_totalFlows +1)/ 2; i++)
	{
		int server = rand() % toalNodes;
		int client = rand() % toalNodes;
		if(server == client)
		{
			i--;
			continue;
		}
		uint16_t port = rand();
		Ipv4Address serverIp = adhocNodes.Get(server)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
		SetUpServer(adhocNodes.Get(server), port, 1, 9);
		
		SetUpClient(adhocNodes.Get(client), serverIp, port, 2, 8);
		NS_LOG_INFO("Flow " << i << ": " << adhocNodes.Get(client)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() << " -> " << serverIp << ":" << port);
	}
}
