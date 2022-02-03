#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/animation-interface.h"

#include "ns3/aodv-module.h"

/*


   st    st    st
   1 ... 2 ... 3

	   n
	   ap
 */

NS_LOG_COMPONENT_DEFINE("wifi_networks");

using namespace ns3;
using namespace std;




NodeContainer nodes;
NodeContainer ap_node;
NodeContainer st_nodes;


void init_nodes(int total_nodes)
{
	// create all nodes
	nodes.Create(total_nodes);

	// ap node is last node
	
	ap_node.Add(nodes.Get(total_nodes-1));

	// stationary nodes are first total_nodes-1 nodes
	
	for(int i=0; i<total_nodes-1; i++)
	{
		st_nodes.Add(nodes.Get(i));
	}
}

int main(int agrc, char * argv[])
{
	// active log 
	LogComponentEnable("wifi_networks", LOG_LEVEL_INFO);
	LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);


	int total_nodes = 4;
	
	CommandLine cmd(__FILE__);
	cmd.AddValue("n", "total nodes in network", total_nodes);

	//parse command line
	cmd.Parse(agrc, argv);

	
	NS_LOG_INFO("Total Nodes: " << total_nodes);

	init_nodes(total_nodes);


	// create channel helper

	YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
	YansWifiPhyHelper phy;
	phy.SetChannel(channel.Create());

	// create wifi helper
	WifiHelper wifi;
	wifi.SetRemoteStationManager("ns3::AarfWifiManager");

	// create wifi mac helper
	WifiMacHelper mac;
	Ssid ssid = Ssid("wifi-network");
	mac.SetType("ns3::StaWifiMac",
				"Ssid", SsidValue(ssid),
				"ActiveProbing", BooleanValue(false));
	
	// change mac type to ap node
	mac.SetType("ns3::ApWifiMac",
				"Ssid", SsidValue(ssid));
	
	
	
	// install wifi on st nodes
	NetDeviceContainer st_devices;
	st_devices = wifi.Install(phy, mac, st_nodes);



	YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default();
	YansWifiPhyHelper phy2;
	phy2.SetChannel(channel2.Create());

	
	// install wifi on ap node
	NetDeviceContainer ap_devices;
	ap_devices = wifi.Install(phy2, mac, ap_node);
	

	
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
	
	mobility.Install(st_nodes);

	// mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install(ap_node);


	// Install internet stack on all nodes
	InternetStackHelper internet;

	// NS_LOG_DEBUG("Using aodv routing");
	// AodvHelper aodv;
	// internet.SetRoutingHelper(aodv);
	internet.Install(nodes);


	NS_LOG_DEBUG("Starting IP address assignment");
	Ipv4AddressHelper ipv4_address;
	ipv4_address.SetBase("10.1.1.0", "255.255.255.0");

	// assign ip address to all nodes
	Ipv4InterfaceContainer interfaces;
	for(int i=0;i<total_nodes-1;i++)
	{
		// set up ip ipv4 addresses
		interfaces.Add(ipv4_address.Assign(st_devices.Get(i)));
		ipv4_address.NewNetwork();
	}

	// assign ip address to ap node
	interfaces.Add(ipv4_address.Assign(ap_devices.Get(0)));

	for(int i=0;i<total_nodes;i++)
	{
		NS_LOG_INFO("Node: " << i << " IP address: " << interfaces.GetAddress(i));
	}

	NS_LOG_DEBUG("IP address assignment complete");




	UdpEchoServerHelper echoServer(9);

	ApplicationContainer serverApps = echoServer.Install(ap_node);
	serverApps.Start(Seconds(1.0));
	serverApps.Stop(Seconds(10.0));


	for(int i=0;i<total_nodes -1; i++){

		UdpEchoClientHelper echoClient(interfaces.GetAddress(total_nodes-1), 9);
		echoClient.SetAttribute("MaxPackets", UintegerValue(1));
		echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
		echoClient.SetAttribute("PacketSize", UintegerValue(1024));

		ApplicationContainer clientApps = echoClient.Install(st_nodes.Get(i));

		clientApps.Start(Seconds(i+1));
		clientApps.Stop(Seconds(10.0));

	}

	// create outputstreamwrapper from output string stream
	using namespace std;
	ostringstream oss;
	Ptr<OutputStreamWrapper> stream = Create<OutputStreamWrapper>( &oss);


	AnimationInterface anim("wifi_networks.xml");
	// enable packet metadata in anim
	anim.EnablePacketMetadata(true);

	// enable global routing 
	// Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	
	Simulator::Stop(Seconds(12.0));

	// Run simulator
	Simulator::Run();
	Simulator::Destroy();

}