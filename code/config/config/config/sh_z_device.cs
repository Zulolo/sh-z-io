/*
 * Created by SharpDevelop.
 * User: dodol
 * Date: 2018/10/12
 * Time: 20:40
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Threading;
using System.Net;
using TFTPClient;
using System.Net.NetworkInformation;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Globalization;
using System.Linq;
using ModbusTCP;
using System.Runtime.Serialization.Formatters.Binary;
using System.IO;

namespace config
{
	/// <summary>
	/// Description of sh_z_device.
	/// </summary>
	public class sh_z_device
	{
		private object _sync = new object();
		private int update_progress;
		private static AutoResetEvent TransferFinishedEvent = new AutoResetEvent(false);
		private ushort mb_tcp_id = 0;
		
		[System.ComponentModel.DisplayName("MAC Address")]
		public string device_mac { get; set; }
		[System.ComponentModel.DisplayName("选择")]
		public bool isSelected { get; set; }
		[System.ComponentModel.DisplayName("静态IP")]
		public bool isStaticIP { get; set; }		
		[System.ComponentModel.DisplayName("IP地址")]
		public IPAddress device_ip { get; set; }
		[System.ComponentModel.DisplayName("静态地址")]
		public IPAddress static_ip { get; set; }
		[System.ComponentModel.DisplayName("网关")]
		public IPAddress device_gateway { get; set; }
		[System.ComponentModel.DisplayName("子网掩码")]
		public IPAddress device_netmask { get; set; }	
		public int device_port { get; set; }				
		public int progress { 
			get {lock (_sync) { return update_progress; }}
			set {lock (_sync) { update_progress = value; }} 
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
  
        public bool ok_to_update()
        {
        	return true;
        }

        public bool ok_to_config()
        {
        	return true;
        }
        
        public void update(string update_file_name)	//, Action<string, int> updatProgress)
        {
        	TFTPSession tftp_client = new TFTPSession();
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
        
        const ushort ETH_INFO_CONF_STATIC_IP = 0x01;
        [StructLayout(LayoutKind.Sequential)]
        public struct EthConfig_t
		{
			public ushort conf;
			[MarshalAs(UnmanagedType.ByValArray, SizeConst=4)]
			public byte[] uIP_Addr;
			[MarshalAs(UnmanagedType.ByValArray, SizeConst=4)]
			public byte[] uNetmask;
			[MarshalAs(UnmanagedType.ByValArray, SizeConst=4)]
			public byte[] uGateway;
			[MarshalAs(UnmanagedType.ByValArray, SizeConst=6)]
			public byte[] uMAC_Addr;
		}
		public byte[] Serialize<T>(T s) where T : struct
		{
		    var size = Marshal.SizeOf(typeof(T));
		    var array = new byte[size];
		    var ptr = Marshal.AllocHGlobal(size);
		    Marshal.StructureToPtr(s, ptr, true);
		    Marshal.Copy(ptr, array, 0, size);
		    Marshal.FreeHGlobal(ptr);
		    return array;
		}
		
		public T Deserialize<T>(byte[] array) where T : struct
		{
			var size = Marshal.SizeOf(typeof(T));
		    var ptr = Marshal.AllocHGlobal(size);
		    Marshal.Copy(array, 0, ptr, size);
		    var s = (T)Marshal.PtrToStructure(ptr, typeof(T));
		    Marshal.FreeHGlobal(ptr);
		    return s;
		}
        
        public void config_device_eth()	//, Action<string, int> updatProgress)
        {
        	bool bParseError = false;
        	byte[] EthConfByteArray;
        	EthConfig_t EthConf = new EthConfig_t();
        	try {
        		EthConf.uIP_Addr = static_ip.GetAddressBytes();
        	} catch (Exception) {
        		return;
        	}
        	
        	try {
        		EthConf.uGateway = device_gateway.GetAddressBytes();
        	} catch (Exception) {
        		EthConf.uGateway = new byte[4];
        		bParseError = true;
        	}
        	
        	try {
        		EthConf.uNetmask = device_netmask.GetAddressBytes();
        	} catch (Exception) {
        		EthConf.uNetmask = new byte[4];
        		bParseError = true;
        	}
	
        	if (bParseError) {
        		EthConf.conf = 0x00;
        	} else {
	         	if (isStaticIP) {
	        		EthConf.conf = 0x01;
	        	} else {
	        		EthConf.conf = 0x00;
	        	}       		
        	}
     	
        	try {
	        	string[] strings = device_mac.Split(':');
	        	EthConf.uMAC_Addr = strings.Select(s => byte.Parse(s, NumberStyles.AllowHexSpecifier)).ToArray();       	
	        	EthConfByteArray = Serialize<EthConfig_t>(EthConf);
        	} catch (Exception) {
        		return;
        	}
        	
        	var modebus_client = new Master();
        	byte[] mb_write_resp = null;
			modebus_client.timeout = 300;
			modebus_client.connect(device_ip.ToString(), (ushort)device_port, false);
			if (modebus_client.connected) {
				modebus_client.WriteMultipleRegister(mb_tcp_id++, 1, 1000, EthConfByteArray, ref mb_write_resp);
				modebus_client.disconnect();
			}
        }
        
        public Boolean get_eth_info() {
          	var modebus_client = new Master();
        	byte[] mb_read_resp = null;
			modebus_client.timeout = 300;
			modebus_client.connect(device_ip.ToString(), (ushort)device_port, false);
			if (modebus_client.connected) {
				modebus_client.ReadInputRegister(mb_tcp_id++, 1, 1000, 10, ref mb_read_resp);
				modebus_client.disconnect();
				var myEthConfig = Deserialize<EthConfig_t>(mb_read_resp);
				if ((BitConverter.ToString(myEthConfig.uMAC_Addr).Replace('-', ':')).StartsWith("02:80:E1:")) {
					device_mac = BitConverter.ToString(myEthConfig.uMAC_Addr).Replace('-', ':');
					device_gateway = new IPAddress(myEthConfig.uGateway);
					static_ip = new IPAddress(myEthConfig.uIP_Addr);
					device_netmask = new IPAddress(myEthConfig.uNetmask);
					isStaticIP = ((myEthConfig.conf & ETH_INFO_CONF_STATIC_IP) == ETH_INFO_CONF_STATIC_IP);
					return true;
				} else {
					return false;					
				}
			} else {
				return false;
			}	
        }
 		public sh_z_device()
		{
 			
		}
 		
		public sh_z_device(IPAddress ipAddress)
		{
			this.device_ip = ipAddress;
		}
		
		public sh_z_device(IPAddress ipAddress, int port, PhysicalAddress physicalAddress)
		{
			this.device_ip = ipAddress;
			this.device_port = port;
			this.device_mac = BitConverter.ToString(physicalAddress.GetAddressBytes()).Replace('-', ':');
		}
	}
}
