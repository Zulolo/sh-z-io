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
		private int update_progress;
		private object _sync = new object();
		private static AutoResetEvent TransferFinishedEvent = new AutoResetEvent(false);

		[System.ComponentModel.DisplayName("MAC Address")]
		public string device_mac { get; set; }
		[System.ComponentModel.DisplayName("升级")]
		public bool isSelected { get; set; }	
		[System.ComponentModel.DisplayName("IP Address")]
		public IPAddress device_ip { get; set; }
		[System.ComponentModel.DisplayName("Port")]		
		public int device_port { get; set; }
		
		public int progress { 
			get {lock (_sync) { return update_progress; }}
			set {lock (_sync) { update_progress = value; }} 
		}
		
        public bool ok_to_update()
        {
        	return true;
        }

        // Transfer delegate methods
        private void _session_Connected()
        {
            Console.WriteLine("Connected");
        }

        private void _session_Transferring(long BytesTransferred, long BytesTotal)
        {
            if (BytesTotal != 0) {
        		this.progress = (int)((BytesTransferred * 100) / BytesTotal);
//                Console.Write("{0}/{1} Bytes Transferred\r", BytesTransferred, BytesTotal);
        	} else {
        		this.progress = 100;
//                Console.Write(".");
        	}
        }

        private void _session_TransferFailed(short ErrorCode, string ErrorMessage)
        {
//            Console.WriteLine("Error {0}: {1}", ErrorCode, ErrorMessage);
            TransferFinishedEvent.Set();
        }

        private void _session_TransferFinished()
        {
//			Console.WriteLine("\nTransfer Finished");
//			MessageBox.Show("Transfer Complete", "TFTP Client", MessageBoxButtons.OK, MessageBoxIcon.Information);
			TransferFinishedEvent.Set();
        }

        private void _session_Disconnected()
        {
//            Console.WriteLine("Disconnected\n");
            TransferFinishedEvent.Set();
        }    
        
        public void update(string update_file_name)	//, Action<string, int> updatProgress)
        {
        	var tftp_client = new TFTPSession();
        	tftp_client.Connected += new TFTPSession.ConnectedHandler(_session_Connected);
            tftp_client.Transferring += new TFTPSession.TransferringHandler(_session_Transferring);
            tftp_client.TransferFailed += new TFTPSession.TransferFailedHandler(_session_TransferFailed);
            tftp_client.TransferFinished += new TFTPSession.TransferFinishedHandler(_session_TransferFinished);
            tftp_client.Disconnected += new TFTPSession.DisconnectedHandler(_session_Disconnected);
            tftp_client.Mode = TFTPSession.Modes.OCTET;
            tftp_client.BlockSize = 256;
            
            TransferOptions tOptions = new TransferOptions();            
            tOptions.LocalFilename = update_file_name;
            tOptions.RemoteFilename = update_file_name;
            tOptions.Host = this.device_ip.ToString();
            tOptions.Action = TransferType.Put;  

			tftp_client.Put(tOptions);   
			TransferFinishedEvent.WaitOne();			
        }
		
		static public bool is_sh_z_002(IPAddress ipAddress, int port)
		{
			Result mb_result;
			byte byte_count;
			byte[] device_info = new byte[256];
			
			return true;
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
		
		public bool is_sh_z_002()
		{
			Result mb_result;
			byte byte_count;
			byte[] device_info = new byte[256];
			
			return true;
			var modebus_client = new WSMBTControl();
			modebus_client.ResponseTimeout = 500;
			modebus_client.ConnectTimeout = 500;
			mb_result = modebus_client.Connect(this.device_ip.ToString(), this.device_port);
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
		
		public sh_z_002(IPAddress ipAddress, int port, PhysicalAddress physicalAddress)
		{
			this.device_ip = ipAddress;
			this.device_port = port;
			this.device_mac = BitConverter.ToString(physicalAddress.GetAddressBytes()).Replace('-', ':');
//			modebus_client = new ModbusClient(ipAddress, port);
		}
	}
}
