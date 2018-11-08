/*
 * Created by SharpDevelop.
 * User: Zulolo
 * Date: 7/2/2018
 * Time: 10:09 PM
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
namespace config
{
	partial class MainForm
	{
		/// <summary>
		/// Designer variable used to keep track of non-visual components.
		/// </summary>
		private System.ComponentModel.IContainer components = null;
		private System.Windows.Forms.Button start_scan;
		private System.Windows.Forms.ProgressBar scan_progress;
		private System.Windows.Forms.Label info_label;
		private System.Windows.Forms.DataGridView scan_result;
		private System.Windows.Forms.OpenFileDialog openUpdateFileDialog;
		private System.Windows.Forms.Button openUpdate;
		private System.Windows.Forms.Button startUpdate;
		private System.Windows.Forms.Button set_device;
		
		/// <summary>
		/// Disposes resources used by the form.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing) {
				if (components != null) {
					components.Dispose();
				}
			}
			base.Dispose(disposing);
		}
		
		/// <summary>
		/// This method is required for Windows Forms designer support.
		/// Do not change the method contents inside the source code editor. The Forms designer might
		/// not be able to load this method if it was changed manually.
		/// </summary>
		private void InitializeComponent()
		{
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
			this.start_scan = new System.Windows.Forms.Button();
			this.scan_progress = new System.Windows.Forms.ProgressBar();
			this.info_label = new System.Windows.Forms.Label();
			this.scan_result = new System.Windows.Forms.DataGridView();
			this.openUpdateFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.openUpdate = new System.Windows.Forms.Button();
			this.startUpdate = new System.Windows.Forms.Button();
			this.set_device = new System.Windows.Forms.Button();
			((System.ComponentModel.ISupportInitialize)(this.scan_result)).BeginInit();
			this.SuspendLayout();
			// 
			// start_scan
			// 
			this.start_scan.Location = new System.Drawing.Point(12, 12);
			this.start_scan.Name = "start_scan";
			this.start_scan.Size = new System.Drawing.Size(75, 23);
			this.start_scan.TabIndex = 0;
			this.start_scan.Text = "Scan";
			this.start_scan.UseVisualStyleBackColor = true;
			this.start_scan.Click += new System.EventHandler(this.Start_scanClick);
			// 
			// scan_progress
			// 
			this.scan_progress.Location = new System.Drawing.Point(11, 313);
			this.scan_progress.Name = "scan_progress";
			this.scan_progress.Size = new System.Drawing.Size(292, 13);
			this.scan_progress.TabIndex = 1;
			this.scan_progress.Visible = false;
			// 
			// info_label
			// 
			this.info_label.Location = new System.Drawing.Point(310, 308);
			this.info_label.Name = "info_label";
			this.info_label.Size = new System.Drawing.Size(225, 23);
			this.info_label.TabIndex = 2;
			// 
			// scan_result
			// 
			this.scan_result.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
			this.scan_result.Location = new System.Drawing.Point(12, 41);
			this.scan_result.Name = "scan_result";
			this.scan_result.Size = new System.Drawing.Size(564, 264);
			this.scan_result.TabIndex = 3;
			// 
			// openUpdateFileDialog
			// 
			this.openUpdateFileDialog.FileName = "openFileDialog1";
			// 
			// openUpdate
			// 
			this.openUpdate.Location = new System.Drawing.Point(420, 12);
			this.openUpdate.Name = "openUpdate";
			this.openUpdate.Size = new System.Drawing.Size(75, 23);
			this.openUpdate.TabIndex = 4;
			this.openUpdate.Text = "Select file";
			this.openUpdate.UseVisualStyleBackColor = true;
			this.openUpdate.Visible = false;
			this.openUpdate.Click += new System.EventHandler(this.OpenUpdateClick);
			// 
			// startUpdate
			// 
			this.startUpdate.Location = new System.Drawing.Point(501, 12);
			this.startUpdate.Name = "startUpdate";
			this.startUpdate.Size = new System.Drawing.Size(75, 23);
			this.startUpdate.TabIndex = 5;
			this.startUpdate.Text = "Update";
			this.startUpdate.UseVisualStyleBackColor = true;
			this.startUpdate.Visible = false;
			this.startUpdate.Click += new System.EventHandler(this.StartUpdateClick);
			// 
			// set_device
			// 
			this.set_device.Location = new System.Drawing.Point(310, 12);
			this.set_device.Name = "set_device";
			this.set_device.Size = new System.Drawing.Size(75, 23);
			this.set_device.TabIndex = 6;
			this.set_device.Text = "Set";
			this.set_device.UseVisualStyleBackColor = true;
			this.set_device.Click += new System.EventHandler(this.Set_deviceClick);
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(588, 331);
			this.Controls.Add(this.set_device);
			this.Controls.Add(this.startUpdate);
			this.Controls.Add(this.openUpdate);
			this.Controls.Add(this.scan_result);
			this.Controls.Add(this.info_label);
			this.Controls.Add(this.scan_progress);
			this.Controls.Add(this.start_scan);
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "MainForm";
			this.Text = "sh-z-002 config";
			((System.ComponentModel.ISupportInitialize)(this.scan_result)).EndInit();
			this.ResumeLayout(false);

		}
	}
}
