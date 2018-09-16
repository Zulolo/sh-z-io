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
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Text;

namespace TFTPClient
{
    public partial class TFTPSession
    {
        public bool Get(string LocalFile, string RemoteFile, string Host,
            Modes Mode, int BlockSize, int Timeout)
        {
            int recvLen, remoteFileSize = 0, buffer = BlockSize + 4;
            long bytesReceived = 0;

            BinaryWriter BWriter = new BinaryWriter(File.Open(LocalFile, FileMode.Create));

            TFTPSession.OpCodes opCode = new TFTPSession.OpCodes();

            IPHostEntry hInfo = Dns.GetHostEntry(Host);
            IPAddress address = hInfo.AddressList[0];
            IPEndPoint remoteEP = new IPEndPoint(address, 69);
            EndPoint localEP = (remoteEP);
            Socket UDPSock = new Socket
                (remoteEP.AddressFamily, SocketType.Dgram, ProtocolType.Udp);

            // Create initial request and buffer for response
            byte[] sendData = _packetBuilder.Request(TFTPSession.OpCodes.RRQ,
                RemoteFile, Mode, BlockSize, 0, Timeout);
            byte[] recvData = new byte[BlockSize + 4];

            UDPSock.ReceiveTimeout = Timeout * 1000;

            // Send request and wait for response
            UDPSock.SendTo(sendData, remoteEP);
            recvLen = UDPSock.ReceiveFrom(recvData, ref localEP);

            // Get TID
            remoteEP.Port = ((IPEndPoint)localEP).Port;

            // Invoke connected event
            Connected.Invoke();

            while (true)
            {
                // Read opcode
                opCode = _packetReader.ReadOpCode(recvData);

                // DATA packet
                if (opCode == TFTPSession.OpCodes.DATA)
                {
                    bytesReceived += recvLen - 4;



                    // Invoke Transferring Event
                    Transferring.Invoke(bytesReceived, remoteFileSize);

                    for (int h = 4; h < recvLen; h++)
                    {
                        BWriter.Write(recvData[h]);
                    }

                    sendData = _packetBuilder.Ack(recvData[2], recvData[3]);

                    // Check if this packet is the last
                    if (recvLen < buffer)
                    {
                        // Send final ACK
                        UDPSock.SendTo(sendData, remoteEP);

                        // Invoked TransferFinished Event
                        TransferFinished.Invoke();

                        break;
                    }
                }

                // OACK packet
                else if (opCode == TFTPSession.OpCodes.OACK)
                {
                    remoteFileSize = _packetReader.ReadTransferSize(recvData);
                    sendData = _packetBuilder.Ack(0, 0);
                }

                // ERROR packet
                else if (opCode == TFTPSession.OpCodes.ERROR)
                {
                    ErrorPacket transferError = _packetReader.ReadError(recvData);
                    TransferFailed.Invoke(transferError.Code, transferError.Message);

                    break;
                }

                // Send next packet
                UDPSock.SendTo(sendData, remoteEP);
                recvLen = UDPSock.ReceiveFrom(recvData, ref localEP);
                remoteEP.Port = ((IPEndPoint)localEP).Port;
            }

            BWriter.Close();
            UDPSock.Close();

            // Invoke Disconnected Event
            Disconnected.Invoke();
            return true;
        }

        #region Get Overloads

        public bool Get(object TransferOptions)
        {
            TransferOptions tOptions = (TransferOptions)TransferOptions;
            return Get(tOptions.LocalFilename, tOptions.RemoteFilename,
                tOptions.Host, Mode, BlockSize, Timeout);
        }

        public bool Get(string File)
        {
            return Get(File, File, Host, Mode, BlockSize, Timeout);
        }

        public bool Get(string File, string Host)
        {
            return Get(File, File, Host, Mode, BlockSize, Timeout);
        }

        public bool Get(string LocalFile, string RemoteFile, string Host)
        {
            return Get(LocalFile, RemoteFile, Host, Mode, BlockSize, Timeout);
        }

        #endregion        
    }
}
