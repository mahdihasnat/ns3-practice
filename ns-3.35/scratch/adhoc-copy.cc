#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"
#include "ns3/core-module.h"
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

NS_LOG_COMPONENT_DEFINE("Adhoc");

using namespace ns3;

int main(int argc, char *argv[])
{

	LogComponentEnable("Adhoc", LOG_LEVEL_ALL);
	LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
	LogComponentEnable("Ipv4", LOG_LEVEL_ALL);
	// LogComponentEnableAll(LOG_LEVEL_INFO);

	int n = 2;

	CommandLine cmd(__FILE__);
	cmd.AddValue("n", "total nodes in network", n);
	cmd.Parse(argc, argv);

	WifiHelper wifi;
	wifi.SetStandard(WIFI_STANDARD_80211a);
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
								 "DataMode", StringValue("OfdmRate54Mbps"));

	WifiMacHelper wifiMac;
	wifiMac.SetType("ns3::AdhocWifiMac");
	
	YansWifiPhyHelper wifiPhy;
	YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();

	
	

	NodeContainer c;
	c.Create(n);

	PacketSocketHelper packetSocket;
	packetSocket.Install(c);

	wifiPhy.SetChannel(wifiChannel.Create());

	NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, c);

	MobilityHelper mobility;
	mobility.SetPositionAllocator("ns3::GridPositionAllocator",
								  "MinX", DoubleValue(0.0),
								  "MinY", DoubleValue(0.0),
								  "DeltaX", DoubleValue(5.0),
								  "DeltaY", DoubleValue(10.0),
								  "GridWidth", UintegerValue(3),
								  "LayoutType", StringValue("RowFirst"));

	mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
							  "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)));

	mobility.Install(c);

	PacketSocketAddress socket;
	socket.SetSingleDevice(devices.Get(0)->GetIfIndex());
	socket.SetPhysicalAddress(devices.Get(n - 1)->GetAddress());
	socket.SetProtocol(1);

	OnOffHelper onoff("ns3::PacketSocketFactory", Address(socket));
	onoff.SetConstantRate(DataRate(6000));
	onoff.SetAttribute("PacketSize", UintegerValue(2000));

	ApplicationContainer apps = onoff.Install(c.Get(0));
	apps.Start(Seconds(0.5));
	apps.Stop(Seconds(10.0));

	TypeId tid = TypeId::LookupByName("ns3::PacketSocketFactory");
	Ptr<Socket> sink = Socket::CreateSocket(c.Get(n - 1), tid);
	NS_ASSERT(sink->Bind() == 0);

	// enable trace file generation
	AsciiTraceHelper ascii;
	wifiPhy.EnableAsciiAll(ascii.CreateFileStream("adhoc-copy.tr"));

	AnimationInterface anim("adhoc-copy.xml");
	anim.EnablePacketMetadata(true);

	NS_LOG_DEBUG("Before running simulation");

	Simulator::Stop(Seconds(11.0));
	Simulator::Run();

	Simulator::Destroy();
	NS_LOG_DEBUG("after running simulation");
}