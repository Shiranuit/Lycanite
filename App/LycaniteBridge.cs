using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;

namespace Lycanite
{
    public enum ELycaniteEventType : byte
    {
        UNKOWN = 0,
        PROCESS_CREATE = 1,
        PROCESS_DESTROY = 2,
        PROCESS_REQPERM = 3,
    }

    public class LycaniteEvent
    {
        internal LycaniteEvent(byte[] buffer)
        {
            if (buffer[0] == (byte)ELycaniteEventType.PROCESS_CREATE)
            {
                this.eventType = ELycaniteEventType.PROCESS_CREATE;
                this.ProcessID = BitConverter.ToUInt64(buffer, 1);
                this.UUID = BitConverter.ToUInt64(buffer, 9);
            }
            else if (buffer[0] == (byte)ELycaniteEventType.PROCESS_DESTROY)
            {
                this.eventType = ELycaniteEventType.PROCESS_DESTROY;
                this.UUID = BitConverter.ToUInt64(buffer, 1); ;
            }
            else if (buffer[0] == (byte)ELycaniteEventType.PROCESS_DESTROY)
            {
                this.eventType = ELycaniteEventType.PROCESS_REQPERM;
            }
        }

        public ELycaniteEventType eventType { get; } = ELycaniteEventType.UNKOWN;
        public UInt64 UUID { get; } = 0;
        public UInt64 ProcessID { get; } = 0;

    }

    [Flags]
    public enum ELycanitePerm : UInt64
    {
        LYCANITE_NONE = 0b_0000_0000,
        LYCANITE_WRITE = 0b_0000_0001,
        LYCANITE_READ = 0b_0000_0010,
        LYCANITE_DELETE = 0b_0000_0100,
    }

    public unsafe class LycaniteBridge
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
        public struct SFILTER_MESSAGE_HEADER
        {
            public uint replyLength;
            public ulong messageId;
        }

        public struct SDATA_RECEIVE
        {
            public SFILTER_MESSAGE_HEADER messageHeader;
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

        private enum ELycaniteAction
        {
            SET_LYCANITE_PID = 0,
            SET_AUTHORIZATION_PID = 1,
            SET_AUTHORIZATION_GLOBAL = 2,
            GET_PROCESS_STATS = 3,
            DELETE_AUTHORIATION_PID = 4,
            DELETE_AUTHORIZATION_GLOBAL = 5,
        }

        private IntPtr port;
        private bool connected;
        private Thread thread;

        public delegate void LycaniteEventHandler(LycaniteEvent e);

        public event LycaniteEventHandler OnLycaniteEvent;

        private void MessageReader()
        {
            IntPtr dataReceive = Marshal.AllocHGlobal(sizeof(SDATA_RECEIVE));
            while (this.connected)
            {

                int status = FilterGetMessage(*(IntPtr*)this.port.ToPointer(), dataReceive, sizeof(byte) * BUFFER_SIZE, IntPtr.Zero);

                if (status == 0)
                {
                    SDATA_RECEIVE data = *(SDATA_RECEIVE*)dataReceive.ToPointer();
                    byte[] buff = new byte[BUFFER_SIZE];
                    Marshal.Copy((IntPtr)data.messageContent, buff, 0, sizeof(byte) * BUFFER_SIZE);
                    OnLycaniteEvent.Invoke(new LycaniteEvent(buff));
                }

            }
            Marshal.FreeHGlobal(dataReceive);
        }

        public LycaniteBridge()
        {
            this.port = Marshal.AllocHGlobal(sizeof(IntPtr));
            this.thread = new Thread(new ThreadStart(this.MessageReader));
        }

        ~LycaniteBridge()
        {
            this.Disconnect();
            Marshal.FreeHGlobal(this.port);
        }

        public bool Connect()
        {
            if (!this.connected)
            {
                this.connected = FilterConnectCommunicationPort("\\LycaniteFF", 0, IntPtr.Zero, 0, IntPtr.Zero, this.port) == 0;
                if (this.connected)
                {
                    this.thread.Start();
                }
            }
            return this.connected;
        }

        public bool IsConnected()
        {
            return this.connected;
        }

        public void Disconnect()
        {
            if (this.connected)
            {
                CloseHandle(*(IntPtr*)this.port.ToPointer());
                this.connected = false;
                this.thread.Abort();
            }
        }

        private bool SendStream(MemoryStream stream)
        {
            byte[] bytes = stream.ToArray();

            int size = bytes.Length * sizeof(byte);
            IntPtr sendPtr = Marshal.AllocHGlobal(size);
            IntPtr byterec = Marshal.AllocHGlobal(sizeof(uint));
            Marshal.Copy(bytes, 0, sendPtr, size);

            bool status = FilterSendMessage(*(IntPtr*)this.port.ToPointer(), sendPtr, (uint)size, IntPtr.Zero, 0, byterec) == 0;
            Marshal.FreeHGlobal(sendPtr);
            Marshal.FreeHGlobal(byterec);

            return status;
        }

        public bool SetLycanitePID(UInt64 pid)
        {
            if (this.IsConnected())
            {

                MemoryStream stream = new MemoryStream();
                using (BinaryWriter writer = new BinaryWriter(stream))
                {
                    writer.Write((byte)ELycaniteAction.SET_LYCANITE_PID);
                    writer.Write(pid);
                }

                return this.SendStream(stream);
            }
            return false;
        }

        public bool SetPIDFilePermissions(UInt64 pid, string file, ELycanitePerm permissions)
        {
            if (this.IsConnected())
            {
                MemoryStream stream = new MemoryStream();
                using (BinaryWriter writer = new BinaryWriter(stream))
                {
                    writer.Write((byte)ELycaniteAction.SET_AUTHORIZATION_PID);
                    writer.Write(pid);
                    writer.Write((UInt64)permissions);
                    writer.Write((UInt16)file.Length);
                    writer.Write(file.ToCharArray());
                }

                return this.SendStream(stream);
            }
            return false;
        }

        public bool SetGlobalFilePermissions(string file, ELycanitePerm permissions)
        {
            if (this.IsConnected())
            {
                MemoryStream stream = new MemoryStream();
                using (BinaryWriter writer = new BinaryWriter(stream))
                {
                    writer.Write((byte)ELycaniteAction.SET_AUTHORIZATION_GLOBAL);
                    writer.Write((UInt64)permissions);
                    writer.Write((UInt16)file.Length);
                    writer.Write(file.ToCharArray());
                }

                return this.SendStream(stream);
            }
            return false;
        }

        public bool DeletePIDFilePermissions(UInt64 pid, string file)
        {
            if (this.IsConnected())
            {
                MemoryStream stream = new MemoryStream();
                using (BinaryWriter writer = new BinaryWriter(stream))
                {
                    writer.Write((byte)ELycaniteAction.DELETE_AUTHORIATION_PID);
                    writer.Write(pid);
                    writer.Write((UInt16)file.Length);
                    writer.Write(file.ToCharArray());
                }

                return this.SendStream(stream);
            }
            return false;
        }

        public bool DeleteGlobalFilePermissions(string file)
        {
            if (this.IsConnected())
            {
                MemoryStream stream = new MemoryStream();
                using (BinaryWriter writer = new BinaryWriter(stream))
                {
                    writer.Write((byte)ELycaniteAction.DELETE_AUTHORIZATION_GLOBAL);
                    writer.Write((UInt16)file.Length);
                    writer.Write(file.ToCharArray());
                }

                return this.SendStream(stream);
            }
            return false;
        }

    }

}