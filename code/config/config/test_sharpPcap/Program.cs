/*
 * Created by SharpDevelop.
 * User: dodol
 * Date: 7/8/2018
 * Time: 3:14 PM
 * 
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using SharpPcap;
using SharpPcap.LibPcap;

namespace test_sharpPcap
{
	class Program
	{
		public static void Main(string[] args)
		{
			// Print SharpPcap version 
			string ver = SharpPcap.Version.VersionString;
			Console.WriteLine("SharpPcap {0}, Example1.IfList.cs", ver);
			
			// Retrieve the device list
			CaptureDeviceList devices = CaptureDeviceList.Instance;
			
			// If no devices were found print an error
			if(devices.Count < 1)
			{
			    Console.WriteLine("No devices were found on this machine");
			    return;
			}
			
			Console.WriteLine("\nThe following devices are available on this machine:");
			Console.WriteLine("----------------------------------------------------\n");
			
			// Print out the available network devices
			foreach(ICaptureDevice dev in devices)
			    Console.WriteLine("{0}\n", dev.ToString());
			
			Console.Write("Press any key to continue . . . ");
			Console.ReadKey(true);
		}
	}
}