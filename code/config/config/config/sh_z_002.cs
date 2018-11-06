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
using System.Linq;
using ModbusTCP;

namespace config
{
	/// <summary>
	/// Description of sh_z_002.
	/// </summary>
	/// 
//	public enum DeviceStatus {Unknow, sh_z_002, sh_z_002_boot, sh_z_002_main_app};
	
	public class sh_z_002 : sh_z_device
	{
		const int SH_Z_002_DEV_INFO_REG_ADDR = 30000;
		const int SH_Z_002_DEV_INFO_REG_LENTH = 8;
		const byte SH_Z_002_SLAVE_ID = 2;
		static ushort mb_tcp_id = 0;
			     	
		static public bool is_sh_z_002(IPAddress ip_address, int mb_port)
		{
			byte[] device_info = null;
			
			var modebus_client = new Master();
			modebus_client.timeout = 300;
			modebus_client.connect(ip_address.ToString(), (ushort)mb_port, false);
			if (modebus_client.connected) {
				modebus_client.ReportSlaveID(mb_tcp_id++, 1, ref device_info);
				if (device_info != null) {
					// check slave ID and maybe also sub slave ID	
					if (device_info[0] == SH_Z_002_SLAVE_ID) {
						modebus_client.disconnect();
						return true;
					} else {
						modebus_client.disconnect();
						return false;
					}
				} 
				modebus_client.disconnect();
			}
		
			return false;
		}
					
		public sh_z_002(IPAddress ipAddress)
		{
			this.device_ip = ipAddress;
		}
	}
}
