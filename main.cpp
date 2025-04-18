
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Wifi6ReassociationExample");

void WifiReassociation()
{
    NS_LOG_UNCOND ("[Time " << Simulator::Now ().GetSeconds () << "s] Triggering disassociation and reassociation...");

    // 模擬 STA 的移動導致重新關聯
    Ptr<MobilityModel> mob = NodeList::GetNode(1)->GetObject<MobilityModel>();
    mob->SetPosition(Vector(150.0, 0.0, 0.0)); // STA 移動到遠離第一個 AP 的位置，接近第二個 AP
}

int main (int argc, char *argv[])
{
    std::cout << "Simulation start" << std::endl;
    LogComponentEnable("Wifi6ReassociationExample", LOG_LEVEL_INFO);

    NodeContainer wifiStaNodes;
    wifiStaNodes.Create (1);
    NodeContainer wifiApNodes;
    wifiApNodes.Create (2);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy;
    phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    phy.SetChannel (channel.Create ());

    WifiHelper wifi;
    wifi.SetStandard (WIFI_STANDARD_80211ax);

    WifiMacHelper mac;
    Ssid ssid = Ssid ("wifi6-ssid");

    NetDeviceContainer staDevice;
    NetDeviceContainer apDevices;

    mac.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid),
                "ActiveProbing", BooleanValue (true));
    staDevice = wifi.Install (phy, mac, wifiStaNodes);

    mac.SetType ("ns3::ApWifiMac",
                "Ssid", SsidValue (ssid));
    apDevices = wifi.Install (phy, mac, wifiApNodes);

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));    // STA
    positionAlloc->Add (Vector (0.0, 10.0, 0.0));   // AP1
    positionAlloc->Add (Vector (200.0, 0.0, 0.0));  // AP2
    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiStaNodes);
    mobility.Install (wifiApNodes);

    InternetStackHelper stack;
    stack.Install (wifiApNodes);
    stack.Install (wifiStaNodes);

    Ipv4AddressHelper address;
    address.SetBase ("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer staInterfaces = address.Assign (staDevice);
    address.Assign (apDevices);

    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (wifiApNodes.Get (1));
    serverApps.Start (Seconds (0.0));
    serverApps.Stop (Seconds (20.0));

    UdpEchoClientHelper echoClient (staInterfaces.GetAddress (0), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (100));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps = echoClient.Install (wifiStaNodes.Get (0));
    clientApps.Start (Seconds (1.0));
    clientApps.Stop (Seconds (20.0));

    Simulator::Schedule (Seconds (5.0), &WifiReassociation); // 在第 5 秒觸發移動

    Simulator::Stop (Seconds (20.0));
    Simulator::Run ();
    Simulator::Destroy ();
    std::cout << "Simulation finish" << std::endl;

    return 0;
}
