#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"


NS_LOG_COMPONENT_DEFINE("manet");

#include "routing-experiment.h"

using namespace ns3;
using namespace std;

int main(int argc, char *argv[])
{

	int n = 2;
	int nFlows = 2;
	// int packetRate=10; // number of packet per sec
	double nodeSpeed = 20;		   // in m/s
	double simulationTime = 100.0; // in s

	LogComponentEnable("manet", LOG_LEVEL_ALL);
	LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

	CommandLine cmd(__FILE__);
	cmd.AddValue("n", "total nodes in network", n);
	cmd.AddValue("nFlows", "number of flows", nFlows);
	cmd.AddValue("nodeSpeed", "node speed in m/s", nodeSpeed);
	cmd.Parse(argc, argv);

	RoutingExperiment experiment(n, nFlows, nodeSpeed);

	// disable hellopacket in all node of routing
	Config::SetDefault("ns3::aodv::RoutingProtocol::EnableHello", BooleanValue(false));

	experiment.Run(simulationTime, new AodvHelper());
}


