using System;
using System.Collections.Generic;
using System.Text;

namespace TFTPClient
{
    public class PacketReader
    {
        public TFTPSession.OpCodes ReadOpCode(byte[] ReceivedData)
        {
            return (TFTPSession.OpCodes)ReceivedData[1];
        }

        public int ReadTransferSize(byte[] ReceivedData)
        {
            int h, tSize = 0;
            string searchStr, decPacket = Encoding.ASCII.GetString(ReceivedData);
            char[] splitChar = { '\0' };
            string[] splitPacket = decPacket.Split(splitChar);

            for (h = 0; h < splitPacket.Length - 1; h++)
            {
                searchStr = splitPacket[h].ToLower();
                if (searchStr == "tsize")
                {
                    tSize = int.Parse(splitPacket[h + 1]);
                }
            }
            return tSize;
        }

        public ErrorPacket ReadError(byte[] ReceivedData)
        {
            string codeStr = ReceivedData[2].ToString() + ReceivedData[3].ToString();

            short code = short.Parse(codeStr);
            string message = "";

            for (int h = 4; h < ReceivedData.Length; h++)
            {
                if (ReceivedData[h] == 0)
                    break;

                message += (char)ReceivedData[h];
            }

            return new ErrorPacket(code, message);
        }

        public bool CompareBlocks(byte[] SentData, byte[] ReceivedData)
        {
            if (ReceivedData[2] == SentData[2] &&
                ReceivedData[3] == SentData[3])
                return true;

            return false;
        }
    }
}
