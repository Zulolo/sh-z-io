/*
 * Created by SharpDevelop.
 * User: Zulolo
 * Date: 7/2/2018
 * Time: 10:09 PM
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using System.Threading;
using System.Threading.Tasks;
using SharpPcap;
using SharpPcap.LibPcap;

namespace config
{
	/// <summary>
	/// Description of MainForm.
	/// </summary>
	public partial class MainForm : Form
	{
//		delegate void BollArgReturningVoidDelegate(bool enable); 
//		private Thread scanThread = null; 
		List<sh_z_002> sh_z_002_devices = new List<sh_z_002>();
		
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
				var tasks = new List<Task>();
				foreach(ICaptureDevice network_dev in network_devices) 
				{
					tasks.Add(Task.Factory.StartNew(() => scan_sh_z_002(network_dev)));
//					this.scanThread = new Thread(new ThreadStart(this.scan_sh_z_002));  
//    				this.scanThread.Start(network_dev);  
				}
				this.scan_progress.Maximum = 100;
				for (int i = 1; i <= 100; i++)
				{
					this.scan_progress.Value = i;
					Thread.Sleep(70);
				}
				Task.WaitAll(tasks.ToArray());
			}
			start_scan.Enabled = true;
			scan_progress.Visible = false;			
		}
	
		private void scan_sh_z_002(ICaptureDevice network_dev) 
		{
			var my_arp_scan = new arp_scan(network_dev);
			
			if(my_arp_scan.ready_to_scan())
			{
				var arp_devices = my_arp_scan.start_scan(6000);
				if (arp_devices.Count > 0) 
				{
					foreach (var arp_dev in arp_devices) {
						if (sh_z_002.is_sh_z_002(arp_dev.ToString(), 502))
						{
							sh_z_002_devices.Add(new sh_z_002(arp_dev.ToString(), 502));
						}
					}
				}
			}
//			this.enable_scan_button(true);
//			this.visible_progress_bar(false);
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
