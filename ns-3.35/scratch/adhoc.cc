#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"



NS_LOG_COMPONENT_DEFINE("Adhoc");

using namespace ns3;

int main(int argc,char *argv[])
{

	LogComponentEnable("Adhoc", LOG_LEVEL_ALL);
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
	// LogComponentEnableAll(LOG_LEVEL_INFO);
	// LogComponentEnable("Ipv4",LOG_LEVEL_ALL);

	int n=2;

	CommandLine cmd (__FILE__);
	cmd.AddValue("n","total nodes in network",n);
	cmd.Parse (argc, argv);

	// create all nodes
	NodeContainer nodes;
	nodes.Create(n);

	// create ad hoc wifi network
	WifiHelper wifi;
	wifi.SetStandard(WIFI_STANDARD_80211a);

	// wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
    //                             "DataMode", StringValue ("OfdmRate54Mbps"));


	WifiMacHelper wifiMac;
	wifiMac.SetType("ns3::AdhocWifiMac");

	YansWifiChannelHelper wifiChannel;
	// wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(1000.0));
	// wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  	wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel");

	

	YansWifiPhyHelper wifiPhy;	
	wifiPhy.SetChannel(wifiChannel.Create());	

	// install wifi on all nodes
	NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);


	// create mobility helper
	MobilityHelper mobility;
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
								"MinX", DoubleValue (0.0),
								"MinY", DoubleValue (0.0),
								"DeltaX", DoubleValue (5.0),
								"DeltaY", DoubleValue (10.0),
								"GridWidth", UintegerValue (3),
								"LayoutType", StringValue ("RowFirst"));

	mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
							"Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
	
	mobility.Install(nodes);


	// install stack on nodes
	InternetStackHelper internet;
	AodvHelper aodv;
	internet.SetRoutingHelper(aodv);
	internet.Install(nodes);

	// assign ip addresses to all nodes
	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.1.1.0","255.255.255.0");
	Ipv4InterfaceContainer interfaces;
	for(int i=0;i<n;i++)
	{
		interfaces.Add(ipv4.Assign(devices.Get(i)));
		// ipv4.NewNetwork();
	}

	

	// print ip addresses
	for(int i=0;i<n;i++)
	{
		NS_LOG_INFO("Node "<<i<<" has ip address "<<interfaces.GetAddress(i));
	}


	// create udp server on node 0
	uint16_t port = 9;
	UdpEchoServerHelper server(port);
	ApplicationContainer serverapps = server.Install(nodes.Get(0));
	NS_LOG_INFO("Server is installed on node "<<0);
	serverapps.Start(Seconds(1.0));
	serverapps.Stop(Seconds(10.0));

	// create udp client on node n-1

	UdpEchoClientHelper client(interfaces.GetAddress(0),port);
	client.SetAttribute("MaxPackets",UintegerValue(1));
	client.SetAttribute("Interval",TimeValue(Seconds(1.0)));
	client.SetAttribute("PacketSize",UintegerValue(1024));
	
	ApplicationContainer clientapps = client.Install(nodes.Get(n-1));
	NS_LOG_INFO("Client is installed on node "<<n-1);
	clientapps.Start(Seconds(2.0));
	clientapps.Stop(Seconds(9.0));


	// Populate routing table
	// Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	// enable trace file generation
	AsciiTraceHelper ascii;
	wifiPhy.EnableAsciiAll(ascii.CreateFileStream("adhoc.tr"));


	// enable netanim animation
	AnimationInterface anim("adhoc.xml");
	anim.EnablePacketMetadata(true);

	// run simulation
	Simulator::Stop(Seconds(11.0));
	Simulator::Run();	
	Simulator::Destroy();

	return 0;
	



}