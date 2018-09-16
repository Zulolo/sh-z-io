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
using System.Text;

namespace TFTPClient
{    
    internal class PacketBuilder
    {
        public byte[] Request (TFTPSession.OpCodes OpCode, string RemoteFileName,
            TFTPSession.Modes Mode, int BlockSize, long TransferSize, int Timeout)
        {
            // Request packet structure
            // -----------------------------------------------------------------------------
            // |OpCode|FileName|0|Mode|0|BlkSize|0|BSVal|0|TSize|0|TSVal|0|Timeout|0|TVal|0|
            // -----------------------------------------------------------------------------
            int len;
            
            string packetStr = "";
            string mode = Mode.ToString().ToLower();
            string blockSize = BlockSize.ToString();
            string nullChar = "\0";
            
            byte[] packet;                

            // Create packet as a string
            switch (OpCode)
            {
                case TFTPSession.OpCodes.RRQ:
                    packetStr = nullChar + (char)1;
                    break;
                case TFTPSession.OpCodes.WRQ:
                    packetStr = nullChar + (char)2;
                    break;
            }

            packetStr += RemoteFileName + nullChar + mode + nullChar + "blksize" +
                nullChar + BlockSize.ToString() + nullChar + "tsize" + nullChar +
                TransferSize.ToString() + nullChar + "timeout" + nullChar +
                Timeout.ToString() + nullChar ;
            
            len = packetStr.Length;
            packet = new byte[len];

            // Encode packet as ASCII bytes
            packet = System.Text.Encoding.ASCII.GetBytes(packetStr);                                
            return packet;
        }

        public byte[] Ack(int Block1, int Block2)
        {
            // ACK packet structure
            // ----------
            // |04|Block|
            // ----------
            byte[] packet = new byte[4];
            packet[0] = 0;
            packet[1] = (byte)TFTPSession.OpCodes.ACK;
            packet[2] = (byte)Block1;
            packet[3] = (byte)Block2;
            return packet;
        }

        public byte[] Data(byte[] SendData, int Block1, int Block2)
        {
            // DATA packet structure
            // ----------
            // |03|Block|
            // ----------
            byte[] packet = new byte[SendData.Length + 4];
            //packet[0] = 0;
            packet[1] = (byte)TFTPSession.OpCodes.DATA;
            packet[2] = (byte)Block1;
            packet[3] = (byte)Block2;
            for(int h = 4; h < SendData.Length + 4; h++)
            {
                packet[h] = SendData[h - 4];
            }
            return packet;
        }

        public int[] IncrementBock(byte[] ReceivedData, int[] Block)
        {
            if (ReceivedData[3] == 255)
            {
                if (ReceivedData[2] < 255)
                {
                    Block[0] = (int)ReceivedData[2] + 1; Block[1] = 0;
                }
                else
                {
                    Block[0] = 0; Block[1] = 0;
                }
            }
            else
            {
                Block[1] = (int)ReceivedData[3] + 1;
            }
            return Block;
        }
    }
}