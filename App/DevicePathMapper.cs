using System;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;


namespace Lycanite {
    public static class DevicePathMapper {
        [DllImport("Kernel32.dll", CharSet = CharSet.Unicode)]
        private static extern uint QueryDosDevice([In] string lpDeviceName, [Out] StringBuilder lpTargetPath, [In] int ucchMax);

        public static string FromDevicePath(string devicePath) {
            DriveInfo drive = Array.Find(DriveInfo.GetDrives(), d => {
                return devicePath.StartsWith(d.GetDevicePath(), StringComparison.InvariantCultureIgnoreCase);
            });
            if (drive != null) {
                return devicePath.ReplaceFirst(drive.GetDevicePath(), drive.GetDriveLetter());
            }
            return null;
        }

        public static string ToDevicePath(string path) {
            DriveInfo drive = Array.Find(DriveInfo.GetDrives(), d => {
                return path.StartsWith(d.GetDriveLetter(), StringComparison.InvariantCultureIgnoreCase);
            });

            if (drive != null) {
                return path.ReplaceFirst(drive.GetDriveLetter(), drive.GetDevicePath());
            }
            return null;
        }

        private static string GetDevicePath(this DriveInfo driveInfo) {
            StringBuilder devicePathBuilder = new StringBuilder(128);
            if (QueryDosDevice(driveInfo.GetDriveLetter(), devicePathBuilder, devicePathBuilder.Capacity + 1) != 0) {
                return devicePathBuilder.ToString();
            }
            return null;
        }

        private static string GetDriveLetter(this DriveInfo driveInfo) {
            return driveInfo.Name.Substring(0, 2);
        }

        private static string ReplaceFirst(this string text, string search, string replace) {
            int pos = text.IndexOf(search);
            if (pos < 0) {
                return text;
            }
            return text.Substring(0, pos) + replace + text.Substring(pos + search.Length);
        }
    }
}