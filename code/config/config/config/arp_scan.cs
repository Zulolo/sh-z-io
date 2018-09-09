/*
 * Created by SharpDevelop.
 * User: dodol
 * Date: 7/7/2018
 * Time: 10:41 PM
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using SharpPcap;
using SharpPcap.LibPcap;
using PacketDotNet;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Net;
using System.Net.NetworkInformation;
using System.Collections;
using System.Collections.Generic;

namespace config
{
	/// <summary>
	/// Description of arp_scan.
	/// </summary>
	public class arp_scan
	{
        static public string deviceAddr = "";
        static public int deviceIndex   = 0;
        static public bool scanRunning  = false;
        static public Hashtable threads = new Hashtable();
        static public int scanDelay = 1200000; // 1.2 sec
        static public int maxRange = 64; // Default to 0-64 range
        private ICaptureDevice network_dev;

        public static LibPcapLiveDeviceList getDeviceList()
        {
            // Retrieve the device list
            var devices = LibPcapLiveDeviceList.Instance;

            // If no devices were found print an error
            if (devices.Count < 1)
            {
                throw new System.ArgumentException("No devices found");
            }
            return devices;
        }

        static public void spoof(string host)
        {
            LibPcapLiveDevice device = getDevice();
            IPAddress ip = IPAddress.Parse(host);

            // Create a new ARP resolver
            ARP arp = new ARP(device);
            arp.Timeout = new System.TimeSpan(scanDelay * 2); // 100ms

            // Preparar ip y mac fake, solo para spoofing
            IPAddress local_ip;
            IPAddress.TryParse("192.168.1.1", out local_ip);

            PhysicalAddress mac;
            mac = PhysicalAddress.Parse("11-22-33-44-55-66");

            // Enviar ARP

            for (int i = 0; i < 10000; i++)
            {
                try
                {
                    arp.Resolve(ip, local_ip, mac);
                }
                catch (Exception e)
                {
                    MessageBox.Show(ip + " stopped responding: " + e.Message);
                    return;
                }

                System.Threading.Thread.Sleep(5000); // 5 sec
            }

            return;
        }

        public string scanHost(string host)
        {
            // Parse target IP addr

            LibPcapLiveDevice device = getDevice();
            IPAddress targetIP = null;

            bool ok = IPAddress.TryParse(host, out targetIP);

            if (!ok)
            {
                Console.WriteLine("Invalid IP.");
                return "fail";
            }

            // Create a new ARP resolver
            ARP arp = new ARP(device);
            arp.Timeout = new System.TimeSpan(scanDelay); // 100ms

            // Enviar ARP
            var resolvedMacAddress = arp.Resolve(targetIP);

            if (resolvedMacAddress == null)
            {
                return "fail";
            }
            else
            {
                string fmac = formatMac(resolvedMacAddress);
                Console.WriteLine(targetIP + " is at: " + fmac);

                return fmac;
            }

        }

        public void scanNetwork(MainForm form)
        {
            scanRunning = true;

            for (int i = 1; i < maxRange; i++)
            {
                // Build up IP and start scanning
                
                string ip;
                if (deviceAddr.Length == 0)
                    ip = "192.168.1.";
                else
                    ip = Regex.Replace(deviceAddr, @"\d+$", "");

                ip += i;
                string results = scanHost(ip);

//                if (results != "fail")
//                    form.updateList(ip, results);

//                if (i % (maxRange / 10) == 0)
//                    form.updateProgress();
            }

            MessageBox.Show("Scan finished!", "info", MessageBoxButtons.OK, MessageBoxIcon.Information);

            scanRunning = false;

        }

        private static string formatMac(System.Net.NetworkInformation.PhysicalAddress resolvedMacAddress)
        {
            string m = resolvedMacAddress.ToString();
            MatchCollection split = Regex.Matches(m, @"\w{2}");
            string mac = "";

            foreach (Match item in split)
            {
                mac += item.Value + ":";
            }

            return mac.Remove(mac.Length - 1);
        }

        private static LibPcapLiveDevice getDevice()
        {
            LibPcapLiveDevice device;
            if (deviceIndex == 0)
                device = getFirstDevice();
            else
                device = getDeviceList()[deviceIndex];
            return device;
        }

        private static LibPcapLiveDevice getFirstDevice()
        {
            var devices = getDeviceList();

            LibPcapLiveDevice device = devices[0];
            return device;
        }
        
//        private static void handleArrivePackage(Object sender, CaptureEventArgs e)
//        {
//        	MessageBox.Show(e.Device.Name, e.Packet.ToString(), MessageBoxButtons.OK, MessageBoxIcon.Information);
//
//        }
//        private static void handleArrivePackage(object sender, CaptureEventArgs e)
//        {
//            var packet = PacketDotNet.Packet.ParsePacket(e.Packet.LinkLayerType, e.Packet.Data);
//            if(packet is PacketDotNet.EthernetPacket)
//            {
//                var eth = ((PacketDotNet.EthernetPacket)packet);
//                Console.WriteLine("Original Eth packet: " + eth.ToString());
//
//                //Manipulate ethernet parameters
//                eth.SourceHwAddress = PhysicalAddress.Parse("00-11-22-33-44-55");
//                eth.DestinationHwAddress = PhysicalAddress.Parse("00-99-88-77-66-55");
//
//                var ip = (PacketDotNet.IPPacket)packet.Extract(typeof(PacketDotNet.IPPacket));
//                if(ip != null)
//                {
//                    Console.WriteLine("Original IP packet: " + ip.ToString());
//
//                    //manipulate IP parameters
//                    ip.SourceAddress = System.Net.IPAddress.Parse("1.2.3.4");
//                    ip.DestinationAddress = System.Net.IPAddress.Parse("44.33.22.11");
//                    ip.TimeToLive = 11;
//
//                    var tcp = (PacketDotNet.TcpPacket)packet.Extract(typeof(PacketDotNet.TcpPacket));
//                    if (tcp != null)
//                    {
//                        Console.WriteLine("Original TCP packet: " + tcp.ToString());
//
//                        //manipulate TCP parameters
//                        tcp.SourcePort = 9999;
//                        tcp.DestinationPort = 8888;
//                        tcp.Syn = !tcp.Syn;
//                        tcp.Fin = !tcp.Fin;
//                        tcp.Ack = !tcp.Ack;
//                        tcp.WindowSize = 500;
//                        tcp.AcknowledgmentNumber = 800;
//                        tcp.SequenceNumber = 800;
//                    }
//
//                    var udp = (PacketDotNet.UdpPacket)packet.Extract(typeof(PacketDotNet.UdpPacket));
//                    if (udp != null)
//                    {
//                        Console.WriteLine("Original UDP packet: " + udp.ToString());
//
//                        //manipulate UDP parameters
//                        udp.SourcePort = 9999;
//                        udp.DestinationPort = 8888;
//                    }
//                }
//
//                Console.WriteLine("Manipulated Eth packet: " + eth.ToString());
//            }
//        }
        
        public List<IPAddress> start_scan(int scan_time)
        {
        	var arp_devices = new List<IPAddress>();
        	
        	network_dev.Open(DeviceMode.Promiscuous, 4000);
			RawCapture raw_packet = null;
			//Keep capture packets using PcapGetNextPacket()
			while( (raw_packet = network_dev.GetNextPacket()) != null )
			{
				Packet packet = Packet.ParsePacket(raw_packet.LinkLayerType, raw_packet.Data);
				var arp = (ARPPacket)packet.Extract(typeof(ARPPacket));
			    if(arp != null)
			    {
			    	var from_hw_addr = arp.SenderHardwareAddress;
			 		var from_ip_addr = arp.SenderProtocolAddress;
			 		var dst_hw_addr = arp.TargetHardwareAddress;
			 		var dst_ip_addr = arp.TargetProtocolAddress;
			 					 		
			 		MessageBox.Show("From HW: " + from_hw_addr.ToString() + "\r\n" + "From IP: " + from_ip_addr.ToString(), "ARP", MessageBoxButtons.OK, MessageBoxIcon.Information);
			    }
    
				
//				if (packet.DataLink.Kind == DataLinkKind.Ethernet) {
//					if (packet.Ethernet.EtherType == EthernetType.Arp) {
//						MessageBox.Show(packet.ToString(), raw_packet.LinkLayerType.ToString(), MessageBoxButtons.OK, MessageBoxIcon.Information);
//					}				
//				}				
			}
        	network_dev.Close();
        	
        	return arp_devices;
        }
        
        public bool ready_to_scan()
        {
        	if ((network_dev != null) && (network_dev.Started == false))
        	{
        		return true;
        	} else {
        		return false;
        	}
        }
        
        public arp_scan(ICaptureDevice network_device)
        {
        	network_dev = network_device;
        }
	}
}
