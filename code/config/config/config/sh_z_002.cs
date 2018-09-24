/*
 * Created by SharpDevelop.
 * User: Zulolo
 * Date: 7/2/2018
 * Time: 10:18 PM
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
 
 
using System;
using System.Threading;
using System.Net;
using TFTPClient;
using System.Net.NetworkInformation;
using System.Windows.Forms;
using ModbusTCP;

namespace config
{
	/// <summary>
	/// Description of sh_z_002.
	/// </summary>
	/// 
	public enum DeviceStatus {Unknow, sh_z_002, sh_z_002_boot, sh_z_002_main_app};
	
	public class sh_z_002
	{
		private const int SH_Z_002_DEV_INFO_REG_ADDR = 30000;
		private const int SH_Z_002_DEV_INFO_REG_LENTH = 8;
		private int update_progress;
		private object _sync = new object();
		private static AutoResetEvent TransferFinishedEvent = new AutoResetEvent(false);
		private ushort mb_id = 0;
		private const byte SH_Z_002_SLAVE_ID = 2;
		
		[System.ComponentModel.DisplayName("MAC Address")]
		public string device_mac { get; set; }
		[System.ComponentModel.DisplayName("升级")]
		public bool isSelected { get; set; }
		[System.ComponentModel.DisplayName("静态IP")]
		public bool isStaticIP { get; set; }		
		[System.ComponentModel.DisplayName("IP Address")]
		public IPAddress device_ip { get; set; }
	
		public int device_port { get; set; }		
		public DeviceStatus device_status { get; set; }
		
		private TFTPSession tftp_client = new TFTPSession();
		
		public int progress { 
			get {lock (_sync) { return update_progress; }}
			set {lock (_sync) { update_progress = value; }} 
		}
		
        public bool ok_to_update()
        {
        	return true;
        }

        public bool ok_to_config()
        {
        	return true;
        }
        
        // Transfer delegate methods
        private void _session_Connected()
        {
//            Console.WriteLine("Connected");
        }

        private void _session_Transferring(long BytesTransferred, long BytesTotal)
        {
            if (BytesTotal != 0) {
        		this.progress = (int)((BytesTransferred * 100) / BytesTotal);
        	} else {
        		this.progress = 100;
        	}
        }

        private void _session_TransferFailed(short ErrorCode, string ErrorMessage)
        {
//            Console.WriteLine("Error {0}: {1}", ErrorCode, ErrorMessage);
			MessageBox.Show("Transfer faile", "TFTP Client", MessageBoxButtons.OK, MessageBoxIcon.Information);
            TransferFinishedEvent.Set();
        }

        private void _session_TransferFinished()
        {
//			MessageBox.Show("Transfer Complete", "TFTP Client", MessageBoxButtons.OK, MessageBoxIcon.Information);
			TransferFinishedEvent.Set();
        }

        private void _session_Disconnected()
        {
//			MessageBox.Show("Disconnected", "TFTP Client", MessageBoxButtons.OK, MessageBoxIcon.Information);
            TransferFinishedEvent.Set();
        }    
        
        public void update(string update_file_name)	//, Action<string, int> updatProgress)
        {
        	tftp_client.Connected += new TFTPSession.ConnectedHandler(_session_Connected);
            tftp_client.Transferring += new TFTPSession.TransferringHandler(_session_Transferring);
            tftp_client.TransferFailed += new TFTPSession.TransferFailedHandler(_session_TransferFailed);
            tftp_client.TransferFinished += new TFTPSession.TransferFinishedHandler(_session_TransferFinished);
            tftp_client.Disconnected += new TFTPSession.DisconnectedHandler(_session_Disconnected);
            tftp_client.Mode = TFTPSession.Modes.OCTET;
            tftp_client.BlockSize = 512;
            
            TransferOptions tOptions = new TransferOptions();            
            tOptions.LocalFilename = update_file_name;
            tOptions.RemoteFilename = "update.bin";
            tOptions.Host = this.device_ip.ToString();
            tOptions.Action = TransferType.Put;  

			tftp_client.Put(tOptions);   
//			TransferFinishedEvent.WaitOne();			
        }
        
        public void config_device()	//, Action<string, int> updatProgress)
        {
        			
        }
		
		public bool is_sh_z_002()
		{
//			Result mb_result;
//			byte byte_count;
			byte[] device_info = null;
			
			var modebus_client = new ModbusTCP.Master();
			modebus_client.timeout = 300;
			modebus_client.connect(this.device_ip.ToString(), (ushort)this.device_port, false);
			if (modebus_client.connected) {
				modebus_client.ReportSlaveID(mb_id++, 1, ref device_info);
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
		
		public bool is_sh_z_002(string mac_address)
		{
			if (mac_address.StartsWith("02:80:E1:")) {
				device_status = DeviceStatus.sh_z_002;
				if (is_sh_z_002()) {
					device_status = DeviceStatus.sh_z_002_main_app;
				} else {
					device_status = DeviceStatus.sh_z_002_boot;
				}
				return true;		
			} else {
				device_status = DeviceStatus.Unknow;
				return false;
			}			
		}
		
		public sh_z_002(IPAddress ipAddress, int port, PhysicalAddress physicalAddress)
		{
			this.device_ip = ipAddress;
			this.device_port = port;
			this.device_mac = BitConverter.ToString(physicalAddress.GetAddressBytes()).Replace('-', ':');
			device_status = DeviceStatus.Unknow;
//			modebus_client = new ModbusClient(ipAddress, port);
		}
	}
}
