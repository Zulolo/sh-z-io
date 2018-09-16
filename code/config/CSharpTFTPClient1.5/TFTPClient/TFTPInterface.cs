//--------------------------------------------------------------------------------//
//                                                                                //
// Copyright © 2007 John Leitch                                                   //
//                                                                                //
// Distributed under the terms of the GNU General Public License                  //
//                                                                                //
// This file is part of Open Source TFTP Client.                                  //
//                                                                                //
// Open Source TFTP Client is free software: you can redistribute it and/or       //
// modify it under the terms of the GNU General Public License version 3 as       //
// published by the Free Software Foundation.                                     //
//                                                                                //
// Open Source TFTP Client is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                 //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General      //
// Public License for more details.                                               //
//                                                                                //
// You should have received a copy of the GNU General Public License              //
// along with Open Source TFTP Client.  If not, see http://www.gnu.org/licenses/. //
//                                                                                //
//--------------------------------------------------------------------------------//

namespace TFTPC
{
    interface ITFTP
    {   
        string Host
        {
            get;
            set;
        }
        TFTP.Modes Mode
        {
            get;
            set;
        }
        int BlockSize
        {
            get;
            set;
        }
        int Timeout
        {
            get;
            set;
        }

        bool Get(object TransferOptions);
        bool Get(string File);
        bool Get(string File, string Host);
        bool Get(string LocalFile, string RemoteFile, string Host);
        bool Put(object TransferOptions);
        bool Put(string File);
        bool Put(string File, string Host);
        bool Put(string LocalFile, string RemoteFile, string Host);        
    }
}
