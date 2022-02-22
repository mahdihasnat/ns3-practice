#include "routing-experiment.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/sixlowpan-module.h"

using namespace ns3;

class RoutingExperimentLrWpan : public RoutingExperiment
{
public:
	RoutingExperimentLrWpan(int n, int nFlows, double nodeSpeed)
		: RoutingExperiment(n, nFlows, nodeSpeed)
		{

		}
	
	virtual void AddFlows6(NodeContainer &nodes,Ipv6InterfaceContainer & interfaces)
	
	{
		int toalNodes = nodes.GetN();
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
			Address serverIp = interfaces.GetAddress(server,1);
			

			UdpEchoServerHelper echoServer(port);
			ApplicationContainer serverApps = echoServer.Install(nodes.Get(server));
			serverApps.Start(Seconds(1)); // start time in sec
			serverApps.Stop(Seconds(100));	  // end time in sec


			UdpEchoClientHelper echoClient(serverIp, port);
			echoClient.SetAttribute("MaxPackets", UintegerValue(m_maxPacketCount));
			echoClient.SetAttribute("Interval", TimeValue(Seconds(m_totalFlows/2 * 1.0L / m_packetRate)));
			// NS_LOG_UNCOND((Seconds(m_totalFlows/2 * 1.0L / m_packetRate)));
			echoClient.SetAttribute("PacketSize", UintegerValue(100));

			ApplicationContainer clientApps = echoClient.Install(nodes.Get(client));
			
			clientApps.Start(Seconds(2)); // start time in sec
			clientApps.Stop(Seconds(99));	  // end time in sec

			NS_LOG_INFO("Flow " << i << ": " << nodes.Get(client)->GetObject<Ipv6>()->GetAddress(1, 0).GetAddress() << " -> " << serverIp << ":" << port);
		}
	}
	
	void Run(double simulationTime, Ipv4RoutingHelper * routing=0)
	{
		NodeContainer nodes;
		nodes.Create(m_totalNodes);
		AddMobility(nodes);

		LrWpanHelper lrWpanHelper;
		NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);

		lrWpanHelper.AssociateToPan (lrwpanDevices, 1);

		InternetStackHelper internetv6;
		// internetv6.SetIpv4StackInstall(false);
		internetv6.Install (nodes);

		SixLowPanHelper sixlowpan;
		NetDeviceContainer devices = sixlowpan.Install (lrwpanDevices); 

		Ipv6AddressHelper ipv6;
		ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
		Ipv6InterfaceContainer deviceInterfaces;
		deviceInterfaces = ipv6.Assign (devices);

		AddFlows6(nodes, deviceInterfaces);

		if(m_netAnim)
		{
			AnimationInterface anim("low-rate-network.xml");
			anim.EnablePacketMetadata(true);
			// resize nodes
			for (int i = 0; i < m_totalNodes; i++)
			{
				anim.UpdateNodeSize(i, 10, 10);
			}
		}

		// enable trace generation
		if(m_tracePhy)
		{
			AsciiTraceHelper ascii;
			m_wifiPhy.EnableAsciiAll(ascii.CreateFileStream("routing-experiment-lrwpan.tr"));
			lrWpanHelper.EnablePcapAll (std::string ("routing-experiment-lrwpan"), true);
		}

		
		// enable flow monitor

		Ptr<FlowMonitor> flowmon;
		FlowMonitorHelper flowmonHelper;
		flowmon = flowmonHelper.InstallAll();


		Simulator::Stop(Seconds(simulationTime));
		Simulator::Run();

		CalculateMetrics(flowmonHelper, simulationTime);
		// flowmon->SerializeToXmlFile("manet.flowmon", true, true);
		Simulator::Destroy();

		NS_LOG_INFO("Total packets sent: " << (long long)m_TxPacketL3);
		NS_LOG_INFO("Total packets received: " << (long long)m_RxPacketL3);
		NS_LOG_INFO("Total packets dropped: " << (long long)m_DropPacketL3);
		// m_packetDelivaryRatio =PacketDeliveryRatioL3();
		NS_LOG_INFO("Packet Delivery Ratio (L3): " << m_packetDelivaryRatio << "%");
		// m_packetDropRatio = PacketDropRatioL3();
		NS_LOG_INFO("Packet Drop Ratio (L3): " << m_packetDropRatio << "%");
		NS_LOG_INFO("Average end-to-end delay(l3): " << m_TotalEnd2EndDelay.GetSeconds() / m_TotalLocalDeliveredL3Packet << "s");
		// NS_LOG_UNCOND("Average end-to-end delay(l3): " << m_TotalEnd2EndDelay.GetSeconds() / m_TotalLocalDeliveredL3Packet << "s");
		// m_averageEnd2EndDelay = m_TotalEnd2EndDelay.GetSeconds() / m_TotalLocalDeliveredL3Packet;

	}

void CalculateMetrics(FlowMonitorHelper &flowmonHelper, double simulationTime);

};

void
RoutingExperimentLrWpan::CalculateMetrics(FlowMonitorHelper &flowmonHelper, double simulationTime)
{
	Ptr<FlowMonitor> flowmon = flowmonHelper.GetMonitor();
	// get flowclassifier from flowmon object
	Ptr<Ipv6FlowClassifier> classifier = DynamicCast<Ipv6FlowClassifier>(flowmonHelper.GetClassifier6());
	NS_ASSERT(classifier != 0);

	long double packetSent = 0;
	long double packetReceived = 0;
	long double packetDropped = 0;
	long double endToEndDelay = 0;
	
	int totalFlows = 0;

	// iterator over every flow and print the flow monitor statistics
	const FlowMonitor::FlowStatsContainer &stats = flowmon->GetFlowStats();
	long double total_rx = 0;
	Time start_time = Seconds(simulationTime);
	Time end_time = Seconds(0);
	
	for (FlowMonitor::FlowStatsContainer::const_iterator i = stats.begin(); i != stats.end(); ++i)
	{
		FlowId flowId = i->first;
		FlowMonitor::FlowStats fs = i->second;
		Ipv6FlowClassifier::FiveTuple t = classifier->FindFlow(flowId);

		if(t.destinationPort == 654) continue;
		if(t.sourcePort == 654) continue;

		if(m_traceRouting)
		{

			ostringstream oss;
			oss << "(" << t.sourceAddress << "," << t.sourcePort << ")->(" << t.destinationAddress << "," << t.destinationPort << ")";
			string flowDesc = oss.str();

			long double throuputPerFlow = fs.rxBytes ? (fs.rxBytes * 8.0 / (fs.timeLastRxPacket.GetSeconds() - fs.timeFirstTxPacket.GetSeconds())) / 1e6 : 0; // in Mbps
			
			(*m_tpPerFlowStream->GetStream()) << flowDesc << "\t" << throuputPerFlow << "\n";

		}

		NS_LOG_INFO("fid: " << flowId << " bytes: " << fs.rxBytes << " (" << t.sourceAddress << "," << t.sourcePort << ")->(" << t.destinationAddress << "," << t.destinationPort << ")");
		// NS_LOG_INFO("start time: " << fs.timeFirstTxPacket.GetSeconds() << " end time: " << fs.timeLastRxPacket.GetSeconds() << " duration: " << fs.timeLastRxPacket.GetSeconds() - fs.timeFirstTxPacket.GetSeconds());

		total_rx += fs.rxBytes;

		start_time = min(start_time, fs.timeFirstTxPacket);
		end_time = max(end_time, fs.timeLastRxPacket);

		totalFlows++;
		packetSent+= fs.txPackets;
		packetReceived+= fs.rxPackets;
		packetDropped+= fs.lostPackets;
		endToEndDelay+= fs.delaySum.GetSeconds();

	}
	NS_LOG_INFO("Total Rx: " << total_rx);
	NS_LOG_INFO("Start time: " << start_time);
	NS_LOG_INFO("End time: " << end_time);
	long double throughput = total_rx ? total_rx * 8.0 / (end_time.GetSeconds() - start_time.GetSeconds()) / 1e3 : 0;
	m_throughput=throughput;
	NS_LOG_INFO("Average Throughput: " << throughput << " Mbps");
	NS_LOG_INFO("Packet Sent (flowmon): " << packetSent);
	NS_LOG_INFO("Packet Received (flowmon): " << packetReceived);
	NS_LOG_INFO("Packet Dropped (flowmon): " << packetDropped);
	NS_LOG_INFO("End to End Delay (flowmon): " << endToEndDelay);
	NS_LOG_INFO("Total Flows (flowmon): " << totalFlows);
	NS_LOG_INFO("Packet Delivery Ratio (flowmon): " << packetReceived / packetSent*100);
	NS_LOG_INFO("Packet Drop Ratio (flowmon): " << packetDropped / packetSent*100);
	NS_LOG_INFO("Average End to End Delay (flowmon): " << endToEndDelay / totalFlows);
	// NS_LOG_UNCOND("Average End to End Delay (flowmon): " << endToEndDelay / packetReceived);
	m_averageEnd2EndDelay = endToEndDelay / packetReceived;
	m_packetDelivaryRatio = packetReceived / packetSent*100;
	m_packetDropRatio = packetDropped / packetSent*100;

}