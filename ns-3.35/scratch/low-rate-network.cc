#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"

NS_LOG_COMPONENT_DEFINE("low-rate-network");

#include "routing-experiment-lrwpan.h"
#include "ns3/tora-module.h"
#include "ns3/rip-helper.h"
#include "ns3/dsr-module.h"

using namespace ns3;
using namespace std;


int n = 2;
int nFlows = 2;
double nodeSpeed = 20;		   // in m/s
double simulationTime = 100.0; // in s
double xRange = 100.0;
double yRange = 200.0;
int packetRate = 10; // packets per sec
uint32_t maxPacketCount = 1;


void SetUpRoutingExperiment(RoutingExperiment *experiment)
{

	experiment->setXRange(xRange);
	experiment->setYRange(yRange);
	experiment->setPacketRate(packetRate);
	experiment->setMaxPacketCount(maxPacketCount);
}

void VaryNodes()
{
	ofstream out("node_variation.dat", ios::out|ios::app);

	nFlows = 20;
	nodeSpeed = 2;
	xRange = 100.0;
	yRange = 200.0;
	packetRate = 100;
	maxPacketCount = 10;
	simulationTime = 150.0;

	
	RoutingExperimentLrWpan experiment(n, nFlows, nodeSpeed);
	SetUpRoutingExperiment(&experiment);
	NS_ASSERT(experiment.m_totalNodes == n);
	NS_ASSERT(experiment.m_totalFlows == nFlows);
	NS_ASSERT(experiment.m_nodeSpeed == nodeSpeed);
	NS_ASSERT(experiment.m_packetRate == packetRate);
	NS_ASSERT(experiment.m_maxPacketCount == maxPacketCount);
	NS_ASSERT(experiment.m_xRange == xRange);
	NS_ASSERT(experiment.m_yRange == yRange);

	experiment.Run(simulationTime );

	out << n << "," << experiment.m_throughput <<","<< experiment.m_averageEnd2EndDelay <<
	","<<experiment.m_packetDelivaryRatio <<","<< experiment.m_packetDropRatio << endl;
	cout << n << "," << experiment.m_throughput <<","<< experiment.m_averageEnd2EndDelay <<
	","<<experiment.m_packetDelivaryRatio <<","<< experiment.m_packetDropRatio << endl;
	
	
	
}

void VaryFlows()
{
	ofstream out("flow_variation.dat", ios::out|ios::app);
	n=40;
	nodeSpeed = 1;
	xRange = 100.0;
	yRange = 200.0;
	packetRate = 50;
	maxPacketCount = 10;
	simulationTime = 200.0;

	RoutingExperimentLrWpan experiment(n, nFlows, nodeSpeed);
	SetUpRoutingExperiment(&experiment);
	NS_ASSERT(experiment.m_totalNodes == n);
	NS_ASSERT(experiment.m_totalFlows == nFlows);
	NS_ASSERT(experiment.m_nodeSpeed == nodeSpeed);
	NS_ASSERT(experiment.m_packetRate == packetRate);
	NS_ASSERT(experiment.m_maxPacketCount == maxPacketCount);
	NS_ASSERT(experiment.m_xRange == xRange);
	NS_ASSERT(experiment.m_yRange == yRange);

	experiment.Run(simulationTime );
	out << nFlows << "," << experiment.m_throughput <<","<< experiment.m_averageEnd2EndDelay <<
	","<<experiment.m_packetDelivaryRatio <<","<< experiment.m_packetDropRatio << endl;
	cout << nFlows << "," << experiment.m_throughput <<","<< experiment.m_averageEnd2EndDelay <<
	","<<experiment.m_packetDelivaryRatio <<","<< experiment.m_packetDropRatio << endl;
	

}

void VaryPacketRate()
{
	ofstream out("packet_variation.dat", ios::out|ios::app);
	n=20;
	nFlows = 20;
	nodeSpeed = 1;
	xRange = 100.0;
	yRange = 200.0;
	maxPacketCount = 100;
	simulationTime = 300.0;

	RoutingExperimentLrWpan experiment(n, nFlows, nodeSpeed);
	SetUpRoutingExperiment(&experiment);
	NS_ASSERT(experiment.m_totalNodes == n);
	NS_ASSERT(experiment.m_totalFlows == nFlows);
	NS_ASSERT(experiment.m_nodeSpeed == nodeSpeed);
	NS_ASSERT(experiment.m_packetRate == packetRate);
	NS_ASSERT(experiment.m_maxPacketCount == maxPacketCount);
	NS_ASSERT(experiment.m_xRange == xRange);
	NS_ASSERT(experiment.m_yRange == yRange);

	experiment.Run(simulationTime );
	out << packetRate << "," << experiment.m_throughput <<","<< experiment.m_averageEnd2EndDelay <<
	","<<experiment.m_packetDelivaryRatio <<","<< experiment.m_packetDropRatio << endl;
	cout << packetRate << "," << experiment.m_throughput <<","<< experiment.m_averageEnd2EndDelay <<
	","<<experiment.m_packetDelivaryRatio <<","<< experiment.m_packetDropRatio << endl;
	
}

void VaryNodeSpeed()
{
	ofstream out("speed_variation.dat", ios::out|ios::app);
	n=40;
	nFlows = 70;
	xRange = 400.0;
	yRange = 500.0;
	packetRate = 200;
	maxPacketCount = 100;
	simulationTime = 300.0;

	RoutingExperimentLrWpan experiment(n, nFlows, nodeSpeed);
	SetUpRoutingExperiment(&experiment);
	NS_ASSERT(experiment.m_totalNodes == n);
	NS_ASSERT(experiment.m_totalFlows == nFlows);
	NS_ASSERT(experiment.m_nodeSpeed == nodeSpeed);
	NS_ASSERT(experiment.m_packetRate == packetRate);
	NS_ASSERT(experiment.m_maxPacketCount == maxPacketCount);
	NS_ASSERT(experiment.m_xRange == xRange);
	NS_ASSERT(experiment.m_yRange == yRange);

	experiment.Run(simulationTime );
	out << nodeSpeed << "," << experiment.m_throughput <<","<< experiment.m_averageEnd2EndDelay <<
	","<<experiment.m_packetDelivaryRatio <<","<< experiment.m_packetDropRatio << endl;
	cout << nodeSpeed << "," << experiment.m_throughput <<","<< experiment.m_averageEnd2EndDelay <<
	","<<experiment.m_packetDelivaryRatio <<","<< experiment.m_packetDropRatio << endl;
	
}


int main(int argc, char *argv[])
{

	

	// LogComponentEnable("low-rate-network", LOG_LEVEL_ALL);
	// LogComponentEnable("ToraRoutingProtocol", LOG_LEVEL_ALL);
	// LogComponentEnable("AodvRoutingProtocol", LOG_PREFIX_FUNC);
	
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


	// RoutingExperimentLrWpan experiment(n, nFlows, nodeSpeed);
	
	// experiment.m_netAnim=true;

	// SetUpRoutingExperiment(&experiment);

	// experiment.Run(simulationTime);

	// VaryNodes();
	// VaryFlows();
	// VaryPacketRate();
	VaryNodeSpeed();

}


