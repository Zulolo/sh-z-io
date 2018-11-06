/*
 * Created by SharpDevelop.
 * User: dodol
 * Date: 2018/11/5
 * Time: 19:39
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Threading;
using System.Linq;
//using System.Windows.Forms;

namespace config
{
	
	/// <summary>
	/// Description of udp_scan.
	/// </summary>
	/// 
	
	public class sh_z_scan
	{
		List<sh_z_device> sh_z_list = new List<sh_z_device>();
		readonly UdpClient udpClient;
		const string UDP_BROADCAST_DEV_NAME = "sh-z-";
		
		public struct UdpState
		{
			public UdpClient u;
			public IPEndPoint e;
		}
		Boolean dev_not_in_list(List<sh_z_device> dev_list, IPAddress dev_ip)
		{
			foreach(sh_z_device dev in dev_list) {
				if (dev.device_ip.Equals(dev_ip)) {
					return false;
				}
			}
			return true;
		}
		
		void ReceiveCallback(IAsyncResult ar)
		{
			try {
				IPEndPoint remoteIP = new IPEndPoint(IPAddress.Any, 52018);			
				if (ar != null ) {
					Byte[] receiveBytes = udpClient.EndReceive(ar, ref remoteIP);
					string receiveString = Encoding.ASCII.GetString(receiveBytes);
					if (receiveString.StartsWith(UDP_BROADCAST_DEV_NAME) && dev_not_in_list(sh_z_list, remoteIP.Address)) {
						sh_z_list.Add(new sh_z_device(remoteIP.Address));
//						MessageBox.Show("From IP: " + remoteIP.Address.ToString(), "UDP", MessageBoxButtons.OK, MessageBoxIcon.Information);
					}
					udpClient.BeginReceive(ReceiveCallback, new object());	
										
				}
			} catch {
				/*do nothing*/
			}
		}
		
		
		public List<sh_z_device> udp_discovery() {
			var broadcastAddress = new IPEndPoint(IPAddress.Any, 52018);
			
			sh_z_list.Clear();
			udpClient.Client.Bind(broadcastAddress);
			udpClient.BeginReceive(ReceiveCallback, new object());
			Thread.Sleep(6000);
			
			try {
				udpClient.Close();
			} catch (Exception) {
				/*do nothing*/
			}
	
			return sh_z_list;
		}
		
		public sh_z_scan()
		{
			udpClient = new UdpClient();
			udpClient.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
			udpClient.ExclusiveAddressUse = false; 
		}
	}
}
