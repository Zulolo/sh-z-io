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
namespace TFTPClient
{    
    public partial class TFTPSession
    {
        // Delegates
        public delegate void ConnectedHandler();
        public delegate void TransferringHandler(long BytesTransferred, long BytesTotal);
        public delegate void TransferFailedHandler(short ErrorCode, string ErrorMessage);
        public delegate void TransferFinishedHandler();
        public delegate void DisconnectedHandler();

        // Events
        public event ConnectedHandler Connected;
        public event TransferringHandler Transferring;
        public event TransferFailedHandler TransferFailed;
        public event TransferFinishedHandler TransferFinished;
        public event DisconnectedHandler Disconnected;

        // Enumerations
        public enum Modes
        {
            NETASCII = 0,
            OCTET = 1
        }

        public enum OpCodes
        {
            RRQ = 1,    // Read Request
            WRQ = 2,    // Write Request
            DATA = 3,   // Data
            ACK = 4,    // Acknowledge
            ERROR = 5,  // Error
            OACK = 6    // Option Acknowledge
        }

        // Properties
        int _blockSize;

        public int BlockSize
        {
            get
            {
                return _blockSize;
            }
            set
            {
                _blockSize = value;
            }
        }
            
        int _timeout;

        public int Timeout
        {
            get
            {
                return _timeout;
            }
            set
            {
                _timeout = value;
            }
        }

        string _host;

        public string Host
        {
            get
            {
                return _host;
            }
            set
            {
                _host = value;
            }
        }

        Modes _mode;

        public Modes Mode
        {
            get
            {
                return _mode;
            }
            set
            {
                _mode = value;
            }
        }

        PacketReader _packetReader = new PacketReader();

        PacketBuilder _packetBuilder = new PacketBuilder();

        // Constructor
        public TFTPSession()
        {
            // Default property values
            _mode = Modes.OCTET;
            _blockSize = 512;
            _timeout = 10;
        }
    }
}