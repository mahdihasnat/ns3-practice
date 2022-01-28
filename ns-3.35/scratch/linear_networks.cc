#include "ns3/core-module.h"
#include "ns3/aodv-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LinearNetworks");

/*
	0 -------- 1 --- 2 --- 3 -- ,, ,, -------- n-1
UdpEchoClient                           UdpEchoServer
*/

int
main(int argc, char * argv[])
{

	Time::SetResolution (Time::NS);
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
	// LogComponentEnable ("GlobalRouteManagerImpl", LOG_LEVEL_INFO);
	// LogComponentEnable ("GlobalRouteManager", LOG_LEVEL_INFO);
	
  	
	int totalNodes = 2;

	CommandLine cmd (__FILE__);
	cmd.AddValue("n", "Total number of nodes", totalNodes);
	cmd.Parse (argc, argv);

	NS_LOG_UNCOND("totalNodes: " << totalNodes);

	NS_ASSERT(totalNodes >= 2);

	NodeContainer nodes;
	nodes.Create(totalNodes);


	// Install the internet stack on the nodes


	InternetStackHelper internet;
	AodvHelper aodv;
	internet.SetRoutingHelper(aodv);
	// internet.SetIpv4StackInstall(false);
	internet.Install(nodes);


	// Create the point-to-point link between the nodes
	// Assign IPv4 addresses to the devices
	PointToPointHelper pointToPoint;

	pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
	pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
	

	for(int i=0;i<totalNodes-1; i++)
	{
		Ipv4AddressHelper address;
		std:: ostringstream oss;
		oss<<"192.168."<<i+1<<".0";
		address.SetBase(oss.str().c_str(), "255.255.255.0");

		NetDeviceContainer devices;
		devices = pointToPoint.Install(nodes.Get(i), nodes.Get(i+1));	

		Ipv4InterfaceContainer interfaces = address.Assign(devices);

		NS_LOG_UNCOND("IP address of node " << i << " is " << interfaces.GetAddress(0) << " and node " << i+1 << " is " << interfaces.GetAddress(1));

	}

	// setup UDP Echo server in node n-1
	UdpEchoServerHelper echoServer(9);
	ApplicationContainer serverApps = echoServer.Install(nodes.Get(totalNodes-1));
	serverApps.Start(Seconds(1.0));
	serverApps.Stop(Seconds(10.0));


	// setup UDP Echo client in node 0
	// get interface address of node n-1
	Ipv4Address serverAddress = nodes.Get(totalNodes-1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
	UdpEchoClientHelper echoClient(serverAddress, 9);
	echoClient.SetAttribute("MaxPackets", UintegerValue(1));
	echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
	echoClient.SetAttribute("PacketSize", UintegerValue(1024));

	ApplicationContainer cleintApps =  echoClient.Install(nodes.Get(0));

	cleintApps.Start(Seconds(2.0));
	cleintApps.Stop(Seconds(9.0));

	

	AsciiTraceHelper asciiTraceHelper;
  	pointToPoint.EnableAsciiAll (asciiTraceHelper.CreateFileStream ("linear_networks.tr"));
  	pointToPoint.EnablePcapAll ("linear_networks");

	AnimationInterface anim ("linear_networks.xml");
	anim.EnablePacketMetadata(true);
	for(int i=0;i<totalNodes;i++)
	{
		anim.SetConstantPosition(nodes.Get(i),i*100,i*100,0);
		anim.UpdateNodeSize(i, 10, 10);
	}

	Simulator::Stop(Seconds(20.0));
	Simulator::Run();
	Simulator::Destroy();


}