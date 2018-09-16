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
        public bool Put(string LocalFile, string RemoteFile, string Host,
            Modes Mode, int BlockSize, int Timeout)
        {
            int[] block = new int[2];
            int bufferSize = BlockSize;
            long fileSize, bytesSent = 0;

            BinaryReader BReader = new BinaryReader(File.Open(LocalFile, FileMode.Open));
            FileInfo sendFile = new FileInfo(LocalFile);

            TFTPSession.OpCodes opCode = new TFTPSession.OpCodes();

            IPHostEntry hostInfo = Dns.GetHostEntry(Host);
            IPAddress address = hostInfo.AddressList[0];
            IPEndPoint remoteEP = new IPEndPoint(address, 69);
            EndPoint localEP = (remoteEP);
            Socket UDPSock = new Socket
                (remoteEP.AddressFamily, SocketType.Dgram, ProtocolType.Udp);

            // Retrieve filesize for tsize option
            fileSize = sendFile.Length;

            // Create initial request and buffer for response
            byte[] sendData = _packetBuilder.Request (TFTPSession.OpCodes.WRQ,
                RemoteFile, Mode, BlockSize, fileSize, Timeout);
            byte[] recvData = new byte[bufferSize];

            UDPSock.ReceiveTimeout = Timeout * 1000;

            // Send request and wait for response
            UDPSock.SendTo(sendData, remoteEP);
            UDPSock.ReceiveFrom(recvData, ref localEP);

            //Get TID
            remoteEP.Port = ((IPEndPoint)localEP).Port;

            // Invoke Connected Event
            Connected.Invoke();

            while (true)
            {
                // Read opcode
                opCode = _packetReader.ReadOpCode(recvData);

                // ACK packet
                if (opCode == TFTPSession.OpCodes.ACK)
                {
                    block = _packetBuilder.IncrementBock(recvData, block);

                    sendData = BReader.ReadBytes(bufferSize);
                    bytesSent += sendData.Length;

                    // Invoke Transferring Event
                    Transferring.Invoke(bytesSent, fileSize);

                    sendData = _packetBuilder.Data(sendData, block[0], block[1]);

                    // Check if this packet is the last
                    if (sendData.Length < bufferSize + 4)
                    {
                        // Send final data packet and wait for ack
                        while (true)
                        {
                            UDPSock.SendTo(sendData, remoteEP);
                            UDPSock.ReceiveFrom(recvData, ref localEP);
                            remoteEP.Port = ((IPEndPoint)localEP).Port;

                            // Check the blocks and break free if equal
                            if (_packetReader.CompareBlocks(sendData, recvData))
                                break;
                        }

                        // Invoke TransferFinished Event
                        TransferFinished.Invoke();
                        break;
                    }
                }

                // OACK packet
                else if (opCode == TFTPSession.OpCodes.OACK)
                {
                    sendData = BReader.ReadBytes(bufferSize);
                    sendData = _packetBuilder.Data(sendData, 0, 1);
                    bytesSent += sendData.Length - 4;

                    // Invoke Transferring Event
                    Transferring.Invoke(bytesSent, fileSize);

                    if (fileSize == 0)
                    {
                        // Invoke TransferFinished Event
                        TransferFinished.Invoke();
                        break;
                    }
                    else
                    {

                        // Check if this packet is the last
                        if (sendData.Length < bufferSize + 4)
                        {
                            // Send final data packet and wait for ack
                            while (true)
                            {
                                UDPSock.SendTo(sendData, remoteEP);
                                UDPSock.ReceiveFrom(recvData, ref localEP);
                                remoteEP.Port = ((IPEndPoint)localEP).Port;

                                // Check the blocks and break free if equal
                                if (_packetReader.CompareBlocks(sendData, recvData))
                                    break;
                            }
                            // Invoke TransferFinished Event
                            TransferFinished.Invoke();
                            break;
                        }
                    }
                }
                else if (opCode == TFTPSession.OpCodes.ERROR)
                {
                    ErrorPacket transferError = _packetReader.ReadError(recvData);
                    TransferFailed.Invoke(transferError.Code, transferError.Message);
                    break;
                }

                // Send next packet
                UDPSock.SendTo(sendData, remoteEP);
                UDPSock.ReceiveFrom(recvData, ref localEP);
                remoteEP.Port = ((IPEndPoint)localEP).Port;
            }
            BReader.Close();
            UDPSock.Close();

            // Invoke Disconnected Event
            Disconnected.Invoke();

            return true;
        }

        #region Put Overloads

        public bool Put(object TransferOptions)
        {
            TransferOptions tOptions = (TransferOptions)TransferOptions;
            return Put(tOptions.LocalFilename, tOptions.RemoteFilename,
                tOptions.Host, Mode, BlockSize, Timeout);
        }

        public bool Put(string File)
        {
            return Put(File, File, Host, Mode, BlockSize, Timeout);
        }

        public bool Put(string File, string Host)
        {
            return Put(File, File, Host, Mode, BlockSize, Timeout);
        }

        public bool Put(string LocalFile, string RemoteFile, string Host)
        {
            return Put(LocalFile, RemoteFile, Host, Mode, BlockSize, Timeout);
        }

        #endregion
    }
}
