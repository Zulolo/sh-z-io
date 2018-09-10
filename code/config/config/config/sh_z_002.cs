/*
 * Created by SharpDevelop.
 * User: Zulolo
 * Date: 7/2/2018
 * Time: 10:18 PM
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Net;
using System.Windows.Forms;
using WSMBT;

namespace config
{
	/// <summary>
	/// Description of sh_z_002.
	/// </summary>
	public class sh_z_002
	{
		private const int SH_Z_002_DEV_INFO_REG_ADDR = 30000;
		private const int SH_Z_002_DEV_INFO_REG_LENTH = 8;
		private IPAddress device_ip;
		private int device_port;
		private int device_in_data_grid_index;
		public IPAddress ip
		{
			set {
				device_ip = value;
			}
			get {
				return device_ip;
			}
		}
				
		public int port
		{
			set {
				if (value > 1 && value < 65536) {
					device_port = value;
				} else {
					device_port = 502;
				}				
			}
			get {
				return device_port;
			}
		}
		
		static public bool is_sh_z_002(IPAddress ipAddress, int port)
		{
			Result mb_result;
			byte byte_count;
			byte[] device_info = new byte[256];
			
			var modebus_client = new WSMBTControl();
			modebus_client.ResponseTimeout = 500;
			modebus_client.ConnectTimeout = 500;
			mb_result = modebus_client.Connect(ipAddress.ToString(), port);
			if (Result.SUCCESS == mb_result) {
				mb_result = modebus_client.ReportSlaveID(1, out byte_count, device_info);
				if (Result.SUCCESS == mb_result) {
					// check slave ID and maybe also sub slave ID	
					MessageBox.Show(device_info.ToString(), "report slave ID", MessageBoxButtons.OK, MessageBoxIcon.Information);
					return true;
				}
			}
		
			return false;
		}
		
		public sh_z_002(IPAddress ipAddress, int port)
		{
//			this.ip = IPAddress.Parse(ipAddress);
//			this.device_port = port;
//			modebus_client = new ModbusClient(ipAddress, port);
		}
	}
}
