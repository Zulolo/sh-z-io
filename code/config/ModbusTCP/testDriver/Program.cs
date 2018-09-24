/*
 * Created by SharpDevelop.
 * User: dodol
 * Date: 2018/9/24
 * Time: 15:33
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using ModbusTCP;
using System.Windows.Forms;

namespace testDriver
{
	class Program
	{
			
		public static void Main(string[] args)
		{
			Console.WriteLine("Hello World!");
			
			// TODO: Implement Functionality Here
			byte[] device_info = null;
			ushort mb_id = 1;
			
			var modebus_client = new Master();
//			modebus_client.timeout = 300;
			modebus_client.connect("192.168.0.118", 502, false);
			if (modebus_client.connected) {
				modebus_client.ReadInputRegister(mb_id, 1, 40100, 4, ref device_info);
//				modebus_client.ReportSlaveID(mb_id, 1, ref device_info);
//				mb_id++;
//				if (device_info != null) {
//					// check slave ID and maybe also sub slave ID	
//					MessageBox.Show(device_info.ToString(), "report slave ID", MessageBoxButtons.OK, MessageBoxIcon.Information);
//					modebus_client.disconnect();
//				} 
				modebus_client.disconnect();
			}
			
			Console.Write("Press any key to continue . . . ");
			Console.ReadKey(true);
		}
	}
}