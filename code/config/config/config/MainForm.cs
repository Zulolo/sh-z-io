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

namespace config
{
	/// <summary>
	/// Description of MainForm.
	/// </summary>
	public partial class MainForm : Form
	{
//		delegate void BollArgReturningVoidDelegate(bool enable); 
//		private Thread scanThread = null; 
		BindingList<sh_z_device> sh_z_devices = new BindingList<sh_z_device>();
		BindingSource sh_z_device_list = new BindingSource();
		Sample.DataGridViewProgressColumn Progress = new Sample.DataGridViewProgressColumn();
		string update_file_name = "";
		List<sh_z_device> sh_z_dev_list = null; 
		
		private void AdjustColumnOrder()
		{
		    scan_result.Columns["device_port"].Visible = false;
		    scan_result.Columns["progress"].Visible = false;
//		    scan_result.Columns["device_status"].Visible = false;
		    scan_result.Columns["isSelected"].DisplayIndex = 0;
		    scan_result.Columns["isSelected"].Width = 40;
		    scan_result.Columns["device_ip"].DisplayIndex = 1;
		    scan_result.Columns["device_ip"].Width = 80;
//		    scan_result.Columns["device_ip"].ReadOnly = true;
		    scan_result.Columns["device_mac"].DisplayIndex = 2;
//		    scan_result.Columns["device_mac"].ReadOnly = true;
		    scan_result.Columns["isStaticIP"].DisplayIndex = 4;
		    scan_result.Columns["isStaticIP"].Width = 50;
		    scan_result.Columns["static_ip"].DisplayIndex = 5;
		    scan_result.Columns["static_ip"].Width = 80;
		    scan_result.Columns["device_gateway"].DisplayIndex = 6;
		    scan_result.Columns["device_gateway"].Width = 80;
		    scan_result.Columns["device_netmask"].DisplayIndex = 7;
		    scan_result.Columns["device_netmask"].Width = 80;
//		    if (!scan_result.Columns.Contains(Progress)) {
//		    	scan_result.Columns.Add(Progress);
//		    	var dataGridViewCellStyle1 = new DataGridViewCellStyle(); 
//		    	var resources = new ComponentResourceManager(typeof(MainForm));
//				dataGridViewCellStyle1.Alignment = DataGridViewContentAlignment.MiddleCenter;
//	            dataGridViewCellStyle1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
//	            dataGridViewCellStyle1.ForeColor = System.Drawing.Color.Red;
//	            dataGridViewCellStyle1.NullValue = ((object)(resources.GetObject("dataGridViewCellStyle1.NullValue")));
//	            Progress.DefaultCellStyle = dataGridViewCellStyle1;
//	            Progress.HeaderText = "进度 [%]";
//	            Progress.Name = "dev_up_progress";
//	            Progress.ProgressBarColor = System.Drawing.Color.Lime;
//	            Progress.DisplayIndex = 3;
//	            Progress.ReadOnly = true;
//				scan_result.Columns["dev_up_progress"].Width = 80;	            
//		    }
		}
				
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
		
		void enableButtons(bool enable) 
		{
			start_scan.Enabled = enable;
			openUpdate.Enabled = enable;
			startUpdate.Enabled = enable;
			scan_result.Enabled = enable;
			set_device.Enabled = enable;					
		}
		
		void Start_scanClick(object sender, EventArgs e)
		{
			enableButtons(false);
			scan_progress.Visible = true;
			var tasks = new List<Task>();
			tasks.Add(Task.Factory.StartNew(() => scan_sh_z_devices()));
			this.scan_progress.Maximum = 100;
			for (int i = 1; i <= 100; i++)
			{
				scan_progress.Value = i;
				Thread.Sleep(70);
			}
			Task.WaitAll(tasks.ToArray());
			foreach(sh_z_device sh_z_dev in sh_z_dev_list){
				sh_z_dev.device_port = 502;
				if (sh_z_dev.get_eth_info() != true) {
					sh_z_dev_list.Remove(sh_z_dev);
				}
			}
			scan_result.DataSource = null;
			scan_result.DataSource = sh_z_dev_list;
			AdjustColumnOrder();	
			
			enableButtons(true);
			scan_progress.Visible = false;			
		}
		
		void scan_sh_z_devices() 
		{			
			var my_sh_z_scan = new sh_z_scan();
			sh_z_dev_list = my_sh_z_scan.udp_discovery();
		}
		
		 bool is_sh_z_fw(string filename) 
		{			
			return true;
		}
		
		void OpenUpdateClick(object sender, EventArgs e)
		{
		   if(openUpdateFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK) {  
		      update_file_name = openUpdateFileDialog.FileName;  
		   } 			
		}

		void update_sh_z_device(sh_z_device sh_z_device) 
		{			
			if(sh_z_device.ok_to_update()) {
				sh_z_device.update(update_file_name);	//, update_dev_up_progress);
			}
		}
		
		void set_sh_z_device(sh_z_device sh_z_device) 
		{			
			if(sh_z_device.ok_to_config()) {
				sh_z_device.config_device_eth();	//, update_dev_up_progress);
			}
		}
		
		void StartUpdateClick(object sender, EventArgs e)
		{
			enableButtons(false);
			
			if ((update_file_name != "") && File.Exists(update_file_name)) {	// && is_sh_z_002_fw(update_file_name)) {
				var tasks = new List<Task>();
				foreach (sh_z_device sh_z_obj in sh_z_devices ) {
					if (sh_z_obj.isSelected) {
						sh_z_obj.progress = 0;
						tasks.Add(Task.Factory.StartNew(() => update_sh_z_device(sh_z_obj)));
					}					
				}
				bool all_dev_up_done = false;
				while (false == all_dev_up_done) {
					all_dev_up_done = true;
					foreach (sh_z_device sh_z_dev in sh_z_devices ) {
						if (sh_z_dev.isSelected) {
							if (sh_z_dev.progress < 100) {
								all_dev_up_done = false;
								foreach (DataGridViewRow data_row in scan_result.Rows) {
									if (sh_z_dev.device_mac == data_row.Cells["device_mac"].Value) {
										data_row.Cells["dev_up_progress"].Value = sh_z_dev.progress;
										scan_result.Refresh();
									}
								}															
							}
						}					
					}
					Thread.Sleep(100);
				}	
				foreach (sh_z_device sh_z_obj in sh_z_devices ) {
					if (sh_z_obj.isSelected) {
						foreach (DataGridViewRow data_row in scan_result.Rows) {
							data_row.Cells["dev_up_progress"].Value = sh_z_obj.progress;
							scan_result.Refresh();
						}															
					}					
				}				
				Task.WaitAll(tasks.ToArray());
			}
			enableButtons(true);
		}
		
		void Set_deviceClick(object sender, EventArgs e)
		{
			enableButtons(false);
			var tasks = new List<Task>();
			foreach (sh_z_device sh_z_dev in sh_z_dev_list ) {
				if (sh_z_dev.isSelected) {
					tasks.Add(Task.Factory.StartNew(() => set_sh_z_device(sh_z_dev)));
				}
			}		
			Task.WaitAll(tasks.ToArray());

			enableButtons(true);			
		}
	}
}
