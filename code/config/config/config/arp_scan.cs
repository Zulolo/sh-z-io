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
        private List<sh_z_002> sh_z_002_list;

        public List<sh_z_002> result
		{
			get {
				return sh_z_002_list;
			}
		}       

        private static string formatMac(PhysicalAddress resolvedMacAddress)
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
        
        public List<sh_z_002> start_scan(int scan_time)
        {
        	var arp_devices_ip = new List<IPAddress>();
        	var arp_devices = new List<sh_z_002>();
        	
        	network_dev.Open(DeviceMode.Promiscuous, 4000);
			RawCapture raw_packet = null;
			var scanStartTime = DateTime.Now;
			while ((DateTime.Now - scanStartTime).TotalSeconds <= 5) {
				if( (raw_packet = network_dev.GetNextPacket()) != null )
				{
					Packet packet = Packet.ParsePacket(raw_packet.LinkLayerType, raw_packet.Data);
					var arp = (ARPPacket)packet.Extract
						(typeof(ARPPacket));
				    if(arp != null)
				    {
				    	var from_hw_addr = arp.SenderHardwareAddress;
				 		var from_ip_addr = arp.SenderProtocolAddress;
				 		var dst_hw_addr = arp.TargetHardwareAddress;
				 		var dst_ip_addr = arp.TargetProtocolAddress;
				 		if ((0x02 == from_hw_addr.GetAddressBytes()[0]) && (0x01 == from_hw_addr.GetAddressBytes()[1]) && (0x71 == from_hw_addr.GetAddressBytes()[2])) {
				 			if (!arp_devices_ip.Contains(from_ip_addr)) {
				 				arp_devices_ip.Add(from_ip_addr);
				 				arp_devices.Add(new sh_z_002(from_ip_addr, 502, from_hw_addr));
				 			}				 			
//				 			MessageBox.Show("From HW: " + from_hw_addr.ToString() + "\r\n" + "From IP: " + from_ip_addr.ToString(), "ARP", MessageBoxButtons.OK, MessageBoxIcon.Information);
				 		}
				 	}				
				}				
			}

        	network_dev.Close();
        	
    		if (arp_devices.Count > 0) 
			{
				foreach (var arp_dev in arp_devices) {
					if (arp_dev.is_sh_z_002(arp_dev.device_mac)) {
						sh_z_002_list.Add(arp_dev);				
					}
				}

			}
        	return sh_z_002_list;
        }
        
        public bool ready_to_scan()
        {
        	if ((network_dev != null) && (network_dev.Started == false)) {
        		return true;
        	} else {
        		return false;
        	}
        }
        
        public arp_scan(ICaptureDevice network_device)
        {
        	network_dev = network_device;
        	sh_z_002_list = new List<sh_z_002>();
        }
	}
}
