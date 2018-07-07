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
using SharpPcap;
using SharpPcap.LibPcap;

namespace config
{
	/// <summary>
	/// Description of MainForm.
	/// </summary>
	public partial class MainForm : Form
	{
		delegate void BollArgReturningVoidDelegate(bool enable); 
		private Thread scanThread = null; 
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
			this.scanThread = new Thread(new ThreadStart(this.scan_sh_z_002));  
    		this.scanThread.Start();    
		}
	
		private void scan_sh_z_002() 
		{
			for (int i = 0; i < 500; i++) {
				Console.Write ("y");
				Thread.Sleep(5);
			}
			this.enable_scan_button(true);
			this.visible_progress_bar(false);
		}
		
		private void enable_scan_button(bool enable)  
        {  
            if (this.start_scan.InvokeRequired)  
            {     
                BollArgReturningVoidDelegate d = new BollArgReturningVoidDelegate(enable_scan_button);  
                this.Invoke(d, new object[] { enable });  
            }  
            else  
            {  
                this.start_scan.Enabled = enable;  
            }  
        } 
		
		private void visible_progress_bar(bool enable)  
        {  
            if (this.start_scan.InvokeRequired)  
            {     
                BollArgReturningVoidDelegate d = new BollArgReturningVoidDelegate(visible_progress_bar);  
                this.Invoke(d, new object[] { enable });  
            }  
            else  
            {  
                this.scan_progress.Visible = enable;  
            }  
        }
	}
}
