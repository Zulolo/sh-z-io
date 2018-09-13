/*
 * Created by SharpDevelop.
 * User: Zulolo
 * Date: 7/2/2018
 * Time: 10:09 PM
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.IO;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Threading;
using System.Threading.Tasks;
using System.ComponentModel;
using SharpPcap;

namespace config
{
	/// <summary>
	/// Description of MainForm.
	/// </summary>
	public partial class MainForm : Form
	{
//		delegate void BollArgReturningVoidDelegate(bool enable); 
//		private Thread scanThread = null; 
		private BindingList<sh_z_002> sh_z_002_devices = new BindingList<sh_z_002>();
		private List<arp_scan> arp_scan_list = new List<arp_scan>();
		private BindingSource sh_z_002_dev_list = new BindingSource();
		private string update_file_name = "";
		
		public MainForm()
		{
			//
			// The InitializeComponent() call is required for Windows Forms designer support.
			//
			InitializeComponent();
			
			//
			// TODO: Add constructor code after the InitializeComponent() call.
			//
			start_scan.Enabled = true;
			scan_progress.Visible = false;
//				scan_result.AutoGenerateColumn = false;  
//			scan_result.DataSource = sh_z_002_devices;	
			info_label.Text = "Click scan to start";
		}

		void Start_scanClick(object sender, EventArgs e)
		{
			start_scan.Enabled = false;
			scan_progress.Visible = true;
			
			CaptureDeviceList network_devices = CaptureDeviceList.Instance;
			if(network_devices.Count < 1)
			{
			    MessageBox.Show(this, "No devices were found on this machine", "no network", MessageBoxButtons.OK, 
				                MessageBoxIcon.Error, MessageBoxDefaultButton.Button1, MessageBoxOptions.RightAlign);
			} else {				
				arp_scan_list.Clear();
				var tasks = new List<Task>();
				foreach(ICaptureDevice network_dev in network_devices) 
				{
					var my_arp_scan = new arp_scan(network_dev);
					arp_scan_list.Add(my_arp_scan);
					tasks.Add(Task.Factory.StartNew(() => scan_sh_z_002(my_arp_scan)));
				}
				this.scan_progress.Maximum = 100;
				for (int i = 1; i <= 100; i++)
				{
					this.scan_progress.Value = i;
					Thread.Sleep(70);
				}
				Task.WaitAll(tasks.ToArray());
				
				sh_z_002_devices.Clear();
				foreach(arp_scan arp_scan_obj in arp_scan_list) 
				{
					if (arp_scan_obj.result.Count > 0) {
						foreach (sh_z_002 sh_z_002_obj in arp_scan_obj.result) {
							sh_z_002_devices.Add(sh_z_002_obj);
						}						
					}
				}
				scan_result.DataSource = null;
				scan_result.DataSource = sh_z_002_devices;	
				AdjustColumnOrder();
			}
			start_scan.Enabled = true;
			scan_progress.Visible = false;			
		}
		private void AdjustColumnOrder()
		{
		    scan_result.Columns["device_port"].Visible = false;
		    scan_result.Columns["isSelected"].DisplayIndex = 0;
		    scan_result.Columns["isSelected"].Width = 40;
		    scan_result.Columns["device_ip"].DisplayIndex = 1;
		    scan_result.Columns["device_mac"].DisplayIndex = 2;
		}
		private void scan_sh_z_002(arp_scan my_arp_scan) 
		{			
			if(my_arp_scan.ready_to_scan()) {
				my_arp_scan.start_scan(6000);
			}
		}
		private bool is_sh_z_002_fw(string filename) 
		{			
			return true;
		}
		void OpenUpdateClick(object sender, EventArgs e)
		{
		   if(openUpdateFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK) {  
		      update_file_name = openUpdateFileDialog.FileName;  
		   } 			
		}
		void StartUpdateClick(object sender, EventArgs e)
		{
			if ((update_file_name != "") && File.Exists(update_file_name) && is_sh_z_002_fw(update_file_name)) {
				foreach (sh_z_002 sh_z_002_obj in sh_z_002_devices ) {
					if (sh_z_002_obj.isSelected) {
						MessageBox.Show(sh_z_002_obj.device_ip.ToString(), "update", MessageBoxButtons.OK, MessageBoxIcon.Information);
					}
				}
				
			}
		}

		
//		private void enable_scan_button(bool enable)  
//        {  
//            if (this.start_scan.InvokeRequired)  
//            {     
//                BollArgReturningVoidDelegate d = new BollArgReturningVoidDelegate(enable_scan_button);  
//                this.Invoke(d, new object[] { enable });  
//            }  
//            else  
//            {  
//                this.start_scan.Enabled = enable;  
//            }  
//        } 
//		
//		private void visible_progress_bar(bool enable)  
//        {  
//            if (this.start_scan.InvokeRequired)  
//            {     
//                BollArgReturningVoidDelegate d = new BollArgReturningVoidDelegate(visible_progress_bar);  
//                this.Invoke(d, new object[] { enable });  
//            }  
//            else  
//            {  
//                this.scan_progress.Visible = enable;  
//            }  
//        }
	}
}
