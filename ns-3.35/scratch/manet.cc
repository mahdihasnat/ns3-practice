#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"

NS_LOG_COMPONENT_DEFINE("manet");

#include "routing-experiment.h"
#include "ns3/tora-module.h"
#include "ns3/rip-helper.h"
#include "ns3/dsr-module.h"

using namespace ns3;
using namespace std;


class Test1 : public RoutingExperiment
{
public:
	Test1(int n, int nFlows, double nodeSpeed): RoutingExperiment(3, nFlows, nodeSpeed)
	{
	}
	void AddMobility(NodeContainer &nodes)
	{
		NS_ASSERT(nodes.GetN() == 3);
		// set nodes[0] to (0,0) constant position
		Ptr<ConstantPositionMobilityModel> mob = CreateObject<ConstantPositionMobilityModel>();
		mob->SetPosition(Vector(0,0,0));
		nodes.Get(0)->AggregateObject(mob);
		// set nodes[1] to (100,0) constant position
		mob = CreateObject<ConstantPositionMobilityModel>();
		mob->SetPosition(Vector(100,0,0));
		nodes.Get(1)->AggregateObject(mob);
		// set nodes[2] to (200,20) constant position
		mob = CreateObject<ConstantPositionMobilityModel>();
		mob->SetPosition(Vector(200,20,0));
		nodes.Get(2)->AggregateObject(mob);

	}
	void AddFlows(NodeContainer &adhocNodes)
	{
		NS_ASSERT(adhocNodes.GetN() == 3);
		// Add flows from nodes[0] to nodes[2]
		SetUpServer(adhocNodes.Get(0), 9,1,99);
		Ipv4Address serverIp = adhocNodes.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal();
		SetUpClient(adhocNodes.Get(2), serverIp, 9,2,98);

	}
};

int n = 2;
int nFlows = 2;
// int packetRate=10; // number of packet per sec
double nodeSpeed = 20;		   // in m/s
double simulationTime = 100.0; // in s
double xRange = 300.0;
double yRange = 900.0;
int packetRate = 10; // packets per sec
uint32_t maxPacketCount = 1;


void VaryNodes(RoutingExperiment & re,Ipv4RoutingHelper * routing  )
{
	for(int n=10; n<=100; n+=10)
	{
		re.Run(simulationTime,routing);
	}
}


int main(int argc, char *argv[])
{

	

	LogComponentEnable("manet", LOG_LEVEL_ALL);
	LogComponentEnable("ToraRoutingProtocol", LOG_LEVEL_ALL);
	LogComponentEnable("AodvRoutingProtocol", LOG_PREFIX_FUNC);
	
	// LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
	// LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

	CommandLine cmd(__FILE__);
	cmd.AddValue("n", "total nodes in network", n);
	cmd.AddValue("nFlows", "number of flows", nFlows);
	cmd.AddValue("nodeSpeed", "node speed in m/s", nodeSpeed);
	cmd.AddValue("simulationTime", "simulation time in s", simulationTime);
	cmd.AddValue("xRange", "x axis range", xRange);
	cmd.AddValue("yRange", "y axis range", yRange);
	cmd.AddValue("packetRate", "packet rate", packetRate);
	cmd.AddValue("maxPacketCount", "max packet count", maxPacketCount);
	cmd.Parse(argc, argv);

	NS_LOG_DEBUG("n = " << n);
	NS_LOG_DEBUG("nFlows = " << nFlows);
	NS_LOG_DEBUG("nodeSpeed = " << nodeSpeed);
	NS_LOG_DEBUG("simulationTime " << simulationTime);
	NS_LOG_DEBUG("xRange " << xRange);
	NS_LOG_DEBUG("yRange " << yRange);
	NS_LOG_DEBUG("packetRate " << packetRate);
	NS_LOG_DEBUG("maxPacketCount " << maxPacketCount);

	Packet::EnablePrinting();
	Packet::EnableChecking();


	Test1 experiment(n, nFlows, nodeSpeed);
	experiment.setXRange(xRange);
	experiment.setYRange(yRange);
	experiment.setPacketRate(packetRate);
	experiment.setMaxPacketCount(maxPacketCount);

	// disable hellopacket in all node of routing
	Config::SetDefault("ns3::aodv::RoutingProtocol::EnableHello", BooleanValue(false));
	Ipv4RoutingHelper * routing = new ToraHelper();
	// Ipv4RoutingHelper * routing = new AodvHelper();
	experiment.Run(simulationTime, routing);
	VaryNodes(experiment , routing);
	delete routing;

}


