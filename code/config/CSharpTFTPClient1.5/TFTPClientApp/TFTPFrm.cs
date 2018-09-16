//Copyright (c) 2007, 2008 John Leitch
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
//
//john.leitch5@gmail.com
using System;
using System.Threading;
using System.Windows.Forms;
using System.Resources;
using TFTPClient;

namespace TFTPClientApp
{
    public partial class TFTPFrm : Form
    {
        private static TFTPFrm _instance;

        public static TFTPFrm Instance
        {
            get
            {
                if (_instance == null)
                    _instance = new TFTPFrm();
                lock (_instance)
                    return TFTPFrm._instance;
            }
            set
            {
                if (_instance == null)
                    _instance = new TFTPFrm();
                lock (_instance)
                TFTPFrm._instance = value;
            }
        }

        public TFTPFrm()
        {
            InitializeComponent();
        }

        // TFTP session object
        private TFTPSession _session = new TFTPSession();

        // Delegates
        private delegate void ProgressBarDelegate(int Maximum, int Value);
        private delegate void TransferButtonDelegate(bool Enabled);        

        // Transfer delegate methods
        private void _session_Connected()
        {
            
            TransferButtonDelegate tBtnDel = new TransferButtonDelegate(TransferBtnDelegateFunction);
            TransferButton.Invoke(tBtnDel, false);

            Console.WriteLine("Connected");
        }

        private void _session_Transferring(long BytesTransferred, long BytesTotal)
        {
            if (BytesTotal != 0)
            {
                ProgressBarDelegate progressBarDel = new ProgressBarDelegate(ProgressBarDelegateFunction);
                progressBar.Invoke(progressBarDel,
                    new object[2] { (int)(BytesTotal / 10), (int)(BytesTransferred / 10) });

                Console.Write("{0}/{1} Bytes Transferred\r", BytesTransferred, BytesTotal);
            }
            else
                Console.Write(".");
        }

        private void _session_TransferFailed(short ErrorCode, string ErrorMessage)
        {
            Console.WriteLine("Error {0}: {1}", ErrorCode, ErrorMessage);
        }

        private void _session_TransferFinished()
        {
            ProgressBarDelegate progressBarDel = new ProgressBarDelegate(ProgressBarDelegateFunction);
            progressBar.Invoke(progressBarDel, new object[2] { 0, 0 });

            Console.WriteLine("\nTransfer Finished");

            MessageBox.Show("Transfer Complete", "TFTP Client",
                MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void _session_Disconnected()
        {
            TransferButtonDelegate tBtnDel = new TransferButtonDelegate(TransferBtnDelegateFunction);
            TransferButton.Invoke(tBtnDel, true);

            Console.WriteLine("Disconnected\n");
        }        

        // UI delegate methods
        private void MainFrm_Load(object sender, EventArgs e)
        {
            BlockSizeCombo.SelectedIndex = 0;
            ModeCombo.SelectedIndex = 0;

            _session.Connected += new TFTPSession.ConnectedHandler(_session_Connected);
            _session.Transferring += new TFTPSession.TransferringHandler(_session_Transferring);
            _session.TransferFailed += new TFTPSession.TransferFailedHandler(_session_TransferFailed);
            _session.TransferFinished += new TFTPSession.TransferFinishedHandler(_session_TransferFinished);
            _session.Disconnected += new TFTPSession.DisconnectedHandler(_session_Disconnected);
        }

        private void TransferBtn_Click(object sender, EventArgs e)
        {
            progressBar.Value = 0;

            TransferOptions tOptions = new TransferOptions();            
            tOptions.LocalFilename = LocalFileNameTxt.Text;
            tOptions.RemoteFilename = RemoteFileNameTxt.Text;
            tOptions.Host = HostTxt.Text;
            tOptions.Action = getRadio.Checked == true ? TransferType.Get : TransferType.Put;

            Thread tThread = new Thread((ParameterizedThreadStart)delegate(object ScanOptions)
                {
                    if (((TransferOptions)ScanOptions).Action == TransferType.Get)
                        _session.Get(ScanOptions);
                    else
                        _session.Put(ScanOptions);  
                });
            tThread.IsBackground = true;            
            tThread.Start(tOptions);
        }

        private void HostTxt_Leave(object sender, EventArgs e)
        {
            if (HostTxt.Text == "")
                HostTxt.Text = "Host";
        }

        private void HostTxt_Click(object sender, EventArgs e)
        {
            if (HostTxt.Text == "Host")
                HostTxt.Clear();
        }

        private void HostTxt_TextChanged(object sender, EventArgs e)
        {
            _session.Host = HostTxt.Text;
        }

        private void LocalFileNameTxt_Leave(object sender, EventArgs e)
        {
            if (LocalFileNameTxt.Text == "")
                LocalFileNameTxt.Text = "Local File";
        }

        private void LocalFileNameTxt_Click(object sender, EventArgs e)
        {
            if (LocalFileNameTxt.Text == "Local File")
                LocalFileNameTxt.Clear();
        }

        private void RemoteFileNameTxt_Leave(object sender, EventArgs e)
        {
            if (RemoteFileNameTxt.Text == "")
                RemoteFileNameTxt.Text = "Remote File";
        }

        private void RemoteFileNameTxt_Click(object sender, EventArgs e)
        {
            if (RemoteFileNameTxt.Text == "Remote File")
                RemoteFileNameTxt.Clear();
        }

        private void getRadio_Click(object sender, EventArgs e)
        {
            putRadio.Checked = false;
        }

        private void putRadio_Click(object sender, EventArgs e)
        {
            getRadio.Checked = false;
        }

        private void ModeCombo_SelectedIndexChanged(object sender, EventArgs e)
        {
            switch (ModeCombo.SelectedIndex)
            {
                case 0:
                    _session.Mode = TFTPSession.Modes.OCTET;
                    break;
                case 1:
                    _session.Mode = TFTPSession.Modes.NETASCII;
                    break;
            }
        }

        private void BlockSizeCombo_SelectedValueChanged(object sender, EventArgs e)
        {
            _session.BlockSize = int.Parse(BlockSizeCombo.SelectedItem.ToString());
        }

        private void ProgressBarDelegateFunction(int Maximum, int Value)
        {
            lock (progressBar)
            {
                try
                {

                    progressBar.Maximum = Maximum;
                    progressBar.Value = Value;
                }
                catch (Exception e) { Console.WriteLine(e.ToString()); }
            }
        }

        private void TransferBtnDelegateFunction(bool Enabled)
        {
            lock (TransferButton)
            {
                TransferButton.Enabled = Enabled;
            }
        }
    }
}