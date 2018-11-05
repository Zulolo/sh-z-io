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
//using System.Windows.Forms;

namespace config
{
	
	/// <summary>
	/// Description of udp_scan.
	/// </summary>
	public class sh_z_002_scan
	{
		List<sh_z_002> sh_z_002_list = new List<sh_z_002>();
		readonly UdpClient udpClient;
		const string UDP_BROADCAST_DEV_NAME = "sh-z-002";
		
		public struct UdpState
		{
			public UdpClient u;
			public IPEndPoint e;
		}

		void ReceiveCallback(IAsyncResult ar)
		{
			try {
				IPEndPoint remoteIP = new IPEndPoint(IPAddress.Any, 52018);			
				if (ar != null ) {
					Byte[] receiveBytes = udpClient.EndReceive(ar, ref remoteIP);
					string receiveString = Encoding.ASCII.GetString(receiveBytes);
					if (receiveString.StartsWith(UDP_BROADCAST_DEV_NAME)) {
						sh_z_002_list.Add(new sh_z_002(remoteIP.Address));
//						MessageBox.Show("From IP: " + remoteIP.Address.ToString(), "UDP", MessageBoxButtons.OK, MessageBoxIcon.Information);
					}
					udpClient.BeginReceive(ReceiveCallback, new object());	
										
				}
			} catch {
				/*do nothing*/
			}
		}
		
		public List<sh_z_002> udp_discovery() {
			var broadcastAddress = new IPEndPoint(IPAddress.Any, 52018);
			
			sh_z_002_list.Clear();
			udpClient.Client.Bind(broadcastAddress);
			udpClient.BeginReceive(ReceiveCallback, new object());
			Thread.Sleep(6000);
			
			try {
				udpClient.Close();
			} catch (Exception) {
				/*do nothing*/
			}
	
			return sh_z_002_list;
		}
		
		public sh_z_002_scan()
		{
			udpClient = new UdpClient();
			udpClient.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
			udpClient.ExclusiveAddressUse = false; 
		}
	}
}
