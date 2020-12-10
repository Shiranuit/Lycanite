using System;
using System.IO;
using System.Runtime.InteropServices;

namespace LycaniteDebugger
{
    [Flags]
    public enum LycanitePerm : UInt64
    {
        LYCANITE_WRITE = 0b_0000_0001,
        LYCANITE_READ  = 0b_0000_0010,
    }

    unsafe class LycaniteBridge
    {

        #region Fltlib DLL import

        public const int BUFFER_SIZE = 4096;

        [DllImport("fltlib.dll", SetLastError = true)]
        public static extern int FilterConnectCommunicationPort(
            [MarshalAs(UnmanagedType.LPWStr)]
            string portName,
            uint options,
            IntPtr context,
            uint sizeOfContex,
            IntPtr securityAttributes,
            IntPtr portPtr
        );

        [DllImport("fltlib.dll", SetLastError = true)]
        public static extern int FilterSendMessage(
            IntPtr hPort,
            IntPtr lpInBuffer,
            uint dwInBuffer,
            IntPtr lpOutBuffer,
            uint dwOutBufferSize,
            IntPtr lpBytesReturned
        );

        [StructLayout(LayoutKind.Sequential)]
        public struct FILTER_MESSAGE_HEADER
        {
            public uint replyLength;
            public ulong messageId;
        }

        public struct DATA_RECEIVE
        {
            public FILTER_MESSAGE_HEADER messageHeader;
            public fixed byte messageContent[BUFFER_SIZE];
        }

        [DllImport("fltlib.dll", SetLastError = true)]
        public static extern int FilterGetMessage(
            IntPtr hPort,
            IntPtr lpMessageBuffer,
            uint dwMessageBufferSize,
            IntPtr lpOverlapped
        );

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern int CloseHandle(
            IntPtr handle
        );

        #endregion

        private enum LycaniteAction
        {
            SET_LYCANITE_PID             = 0,
            SET_AUTHORIZATION_PID        = 1,
            SET_AUTHORIZATION_GLOBAL     = 2,
            GET_PROCESS_STATS            = 3,
            DELETE_AUTHORIATION_PID      = 4,
            DELETE_AUTHORIZATION_GLOBAL  = 5,
        }

        private IntPtr port;
        private bool connected;



        public LycaniteBridge()
        {
            port = Marshal.AllocHGlobal(sizeof(IntPtr));
        }

        ~LycaniteBridge()
        {
            Disconnect();
            Marshal.FreeHGlobal(port);
        }

        public bool Connect()
        {
            connected = FilterConnectCommunicationPort("\\LycaniteFF", 0, IntPtr.Zero, 0, IntPtr.Zero, port) == 0;
            return connected;
        }

        public bool IsConnected()
        {
            return connected;
        }

        public void Disconnect()
        {
            if (connected)
            {
                CloseHandle(*(IntPtr*)port.ToPointer());
            }
        }

        private bool sendStream(MemoryStream stream)
        {
            byte[] bytes = stream.ToArray();

            int size = bytes.Length * sizeof(byte);
            IntPtr sendPtr = Marshal.AllocHGlobal(size);
            IntPtr byterec = Marshal.AllocHGlobal(sizeof(uint));
            Marshal.Copy(bytes, 0, sendPtr, size);

            bool status = FilterSendMessage(*(IntPtr*)port.ToPointer(), sendPtr, (uint)size, IntPtr.Zero, 0, byterec) == 0;
            Marshal.FreeHGlobal(sendPtr);
            Marshal.FreeHGlobal(byterec);

            return status;
        }

        public bool SetLycanitePID(UInt64 pid)
        {
            if (IsConnected())
            {

                MemoryStream stream = new MemoryStream();
                using (BinaryWriter writer = new BinaryWriter(stream))
                {
                    writer.Write((byte)LycaniteAction.SET_LYCANITE_PID);
                    writer.Write(pid);
                }

                return sendStream(stream);
            }
            return false;
        }

        public bool setPIDFilePermissions(UInt64 pid, string file, LycanitePerm permissions)
        {
            if (IsConnected())
            {
                MemoryStream stream = new MemoryStream();
                using (BinaryWriter writer = new BinaryWriter(stream))
                {
                    writer.Write((byte)LycaniteAction.SET_AUTHORIZATION_PID);
                    writer.Write(pid);
                    writer.Write((UInt64)permissions);
                    writer.Write((UInt16)file.Length);
                    writer.Write(file.ToCharArray());
                }

                return sendStream(stream);
            }
            return false;
        }

        public bool setGlobalFilePermissions(string file, LycanitePerm permissions)
        {
            if (IsConnected())
            {
                MemoryStream stream = new MemoryStream();
                using (BinaryWriter writer = new BinaryWriter(stream))
                {
                    writer.Write((byte)LycaniteAction.SET_AUTHORIZATION_GLOBAL);
                    writer.Write((UInt64)permissions);
                    writer.Write((UInt16)file.Length);
                    writer.Write(file.ToCharArray());
                }

                return sendStream(stream);
            }
            return false;
        }

        public bool deletePIDFilePermissions(UInt64 pid, string file)
        {
            if (IsConnected())
            {
                MemoryStream stream = new MemoryStream();
                using (BinaryWriter writer = new BinaryWriter(stream))
                {
                    writer.Write((byte)LycaniteAction.DELETE_AUTHORIATION_PID);
                    writer.Write(pid);
                    writer.Write((UInt16)file.Length);
                    writer.Write(file.ToCharArray());
                }

                return sendStream(stream);
            }
            return false;
        }

        public bool deleteGlobalFilePermissions(string file)
        {
            if (IsConnected())
            {
                MemoryStream stream = new MemoryStream();
                using (BinaryWriter writer = new BinaryWriter(stream))
                {
                    writer.Write((byte)LycaniteAction.DELETE_AUTHORIZATION_GLOBAL);
                    writer.Write((UInt16)file.Length);
                    writer.Write(file.ToCharArray());
                }

                return sendStream(stream);
            }
            return false;
        }

    }
}
