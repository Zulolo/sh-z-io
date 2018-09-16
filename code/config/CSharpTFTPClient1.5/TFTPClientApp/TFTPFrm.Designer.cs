namespace TFTPClientApp
{
    partial class TFTPFrm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.LocalFileNameTxt = new System.Windows.Forms.TextBox();
            this.RemoteFileNameTxt = new System.Windows.Forms.TextBox();
            this.HostTxt = new System.Windows.Forms.TextBox();
            this.TransferButton = new System.Windows.Forms.Button();
            this.getRadio = new System.Windows.Forms.RadioButton();
            this.putRadio = new System.Windows.Forms.RadioButton();
            this.ModeCombo = new System.Windows.Forms.ComboBox();
            this.BlockSizeCombo = new System.Windows.Forms.ComboBox();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.SuspendLayout();
            // 
            // LocalFileNameTxt
            // 
            this.LocalFileNameTxt.Location = new System.Drawing.Point(5, 32);
            this.LocalFileNameTxt.Name = "LocalFileNameTxt";
            this.LocalFileNameTxt.Size = new System.Drawing.Size(106, 20);
            this.LocalFileNameTxt.TabIndex = 2;
            this.LocalFileNameTxt.Text = "Local File";
            this.LocalFileNameTxt.Click += new System.EventHandler(this.LocalFileNameTxt_Click);
            this.LocalFileNameTxt.Leave += new System.EventHandler(this.LocalFileNameTxt_Leave);
            // 
            // RemoteFileNameTxt
            // 
            this.RemoteFileNameTxt.Location = new System.Drawing.Point(5, 58);
            this.RemoteFileNameTxt.Name = "RemoteFileNameTxt";
            this.RemoteFileNameTxt.Size = new System.Drawing.Size(106, 20);
            this.RemoteFileNameTxt.TabIndex = 3;
            this.RemoteFileNameTxt.Text = "Remote File";
            this.RemoteFileNameTxt.Click += new System.EventHandler(this.RemoteFileNameTxt_Click);
            this.RemoteFileNameTxt.Leave += new System.EventHandler(this.RemoteFileNameTxt_Leave);
            // 
            // HostTxt
            // 
            this.HostTxt.Location = new System.Drawing.Point(5, 6);
            this.HostTxt.Name = "HostTxt";
            this.HostTxt.Size = new System.Drawing.Size(106, 20);
            this.HostTxt.TabIndex = 1;
            this.HostTxt.Text = "Host";
            this.HostTxt.Click += new System.EventHandler(this.HostTxt_Click);
            this.HostTxt.Leave += new System.EventHandler(this.HostTxt_Leave);
            this.HostTxt.TextChanged += new System.EventHandler(this.HostTxt_TextChanged);
            // 
            // TransferBtn
            // 
            this.TransferButton.Location = new System.Drawing.Point(32, 85);
            this.TransferButton.Name = "TransferBtn";
            this.TransferButton.Size = new System.Drawing.Size(58, 22);
            this.TransferButton.TabIndex = 8;
            this.TransferButton.Text = "Transfer";
            this.TransferButton.UseVisualStyleBackColor = true;
            this.TransferButton.Click += new System.EventHandler(this.TransferBtn_Click);
            // 
            // getRadio
            // 
            this.getRadio.AutoSize = true;
            this.getRadio.Checked = true;
            this.getRadio.Location = new System.Drawing.Point(130, 62);
            this.getRadio.Name = "getRadio";
            this.getRadio.Size = new System.Drawing.Size(42, 17);
            this.getRadio.TabIndex = 6;
            this.getRadio.TabStop = true;
            this.getRadio.Text = "Get";
            this.getRadio.UseVisualStyleBackColor = true;
            this.getRadio.Click += new System.EventHandler(this.getRadio_Click);
            // 
            // putRadio
            // 
            this.putRadio.AutoSize = true;
            this.putRadio.Location = new System.Drawing.Point(130, 83);
            this.putRadio.Name = "putRadio";
            this.putRadio.Size = new System.Drawing.Size(41, 17);
            this.putRadio.TabIndex = 7;
            this.putRadio.Text = "Put";
            this.putRadio.UseVisualStyleBackColor = true;
            this.putRadio.Click += new System.EventHandler(this.putRadio_Click);
            // 
            // ModeCombo
            // 
            this.ModeCombo.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.ModeCombo.FormattingEnabled = true;
            this.ModeCombo.Items.AddRange(new object[] {
            "Binary",
            "NetASCII"});
            this.ModeCombo.Location = new System.Drawing.Point(119, 6);
            this.ModeCombo.Name = "ModeCombo";
            this.ModeCombo.Size = new System.Drawing.Size(70, 21);
            this.ModeCombo.TabIndex = 4;
            this.ModeCombo.SelectedIndexChanged += new System.EventHandler(this.ModeCombo_SelectedIndexChanged);
            // 
            // BlockSizeCombo
            // 
            this.BlockSizeCombo.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.BlockSizeCombo.FormattingEnabled = true;
            this.BlockSizeCombo.Items.AddRange(new object[] {
            "512",
            "1024",
            "2048",
            "4096",
            "8192"});
            this.BlockSizeCombo.Location = new System.Drawing.Point(119, 31);
            this.BlockSizeCombo.Name = "BlockSizeCombo";
            this.BlockSizeCombo.Size = new System.Drawing.Size(70, 21);
            this.BlockSizeCombo.TabIndex = 5;
            this.BlockSizeCombo.SelectedValueChanged += new System.EventHandler(this.BlockSizeCombo_SelectedValueChanged);
            // 
            // progressBar
            // 
            this.progressBar.Location = new System.Drawing.Point(3, 114);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(188, 17);
            this.progressBar.Style = System.Windows.Forms.ProgressBarStyle.Continuous;
            this.progressBar.TabIndex = 0;
            // 
            // TFTPFrm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(194, 134);
            this.Controls.Add(this.progressBar);
            this.Controls.Add(this.BlockSizeCombo);
            this.Controls.Add(this.ModeCombo);
            this.Controls.Add(this.putRadio);
            this.Controls.Add(this.getRadio);
            this.Controls.Add(this.TransferButton);
            this.Controls.Add(this.HostTxt);
            this.Controls.Add(this.RemoteFileNameTxt);
            this.Controls.Add(this.LocalFileNameTxt);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "TFTPFrm";
            this.Text = "TFTP Client";
            this.Load += new System.EventHandler(this.MainFrm_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox LocalFileNameTxt;
        private System.Windows.Forms.TextBox RemoteFileNameTxt;
        private System.Windows.Forms.TextBox HostTxt;
        private System.Windows.Forms.Button TransferButton;
        private System.Windows.Forms.RadioButton getRadio;
        private System.Windows.Forms.RadioButton putRadio;
        private System.Windows.Forms.ComboBox ModeCombo;
        private System.Windows.Forms.ComboBox BlockSizeCombo;
        public System.Windows.Forms.ProgressBar progressBar;
    }
}