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
using EasyModbus;

namespace config
{
	/// <summary>
	/// Description of sh_z_002.
	/// </summary>
	public class sh_z_002
	{
		private const int SH_Z_002_DEV_INFO_REG_ADDR = 30000;
		private const int SH_Z_002_DEV_INFO_REG_LENTH = 8;
//		private IPAddress device_ip;
//		private int device_port;
//		private ModbusClient modebus_client;
//		private int device_in_data_grid_index;
//		public IPAddress ip
//		{
//			set {
//				device_ip = value;
//			}
//			get {
//				return device_ip;
//			}
//		}
//				
//		public int port
//		{
//			set {
//				if (value > 1 && value < 65536) {
//					device_port = value;
//				} else {
//					device_port = 502;
//				}				
//			}
//			get {
//				return device_port;
//			}
//		}
		
		static public bool is_sh_z_002(string ipAddress, int port)
		{
			var modebus_client = new ModbusClient(ipAddress, port);
//			if (modebus_client.Connected == false) {
//				modebus_client.Connect();	
//			}
			if (modebus_client.Available(2000) == false) {
				return false;
			}
			int[] buf = modebus_client.ReadInputRegisters(SH_Z_002_DEV_INFO_REG_ADDR, SH_Z_002_DEV_INFO_REG_LENTH);
			if (buf != null) {
				
				return true;
			} else {
				return false;
			}
		}
		
		public sh_z_002(string ipAddress, int port)
		{
//			this.ip = IPAddress.Parse(ipAddress);
//			this.device_port = port;
//			modebus_client = new ModbusClient(ipAddress, port);
		}
	}
}
