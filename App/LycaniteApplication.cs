using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using MetroSet_UI.Forms;
using System.IO;
using System.Diagnostics;
using Lycanite;

namespace App
{
    public delegate void NotifyProcessClose(Process process);

    public partial class LycaniteApplication : MetroSetForm
    {
        private Dictionary<ulong, String> processes = new Dictionary<ulong, String>();
        private LycaniteBridge lycaniteBridge = new LycaniteBridge();
        private Dictionary<ulong, TabPage> tabPages = new Dictionary<ulong, TabPage>();

        public LycaniteApplication()
        {
            this.InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            this.metroSetTabControl1.Padding = new Point(20, 4);
            if (!this.lycaniteBridge.Connect()) {
                this.BeginInvoke(new MethodInvoker(this.Close));
            } else {
                this.lycaniteBridge.OnLycaniteEvent += this.ProcessLycanite;
            }
        }

        private void ProcessLycanite(LycaniteEvent e) {
            switch (e.eventType) {
                case ELycaniteEventType.PROCESS_CREATE:
                    this.CreateTab(e.UUID, e.ProcessID);
                    break;
                case ELycaniteEventType.PROCESS_DESTROY:
                    this.CloseTab(e.UUID);
                    break;
                case ELycaniteEventType.PROCESS_REQPERM:
                    break;
                default:
                    break;
            }
        }

        private void CloseTab(ulong UUID) {
            TabPage page = this.tabPages[UUID];
            this.tabPages.Remove(UUID);
            if (page == null)
                return;
            this.metroSetTabControl1.TabPages.Remove(this.tabPages[UUID]);
        }

        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.All;
        }

        private void CreateTab(ulong UUID, ulong PID) {
            TabTemplate newTab = new TabTemplate();

            String path = this.processes[PID];
            if (path == null)
                return;
            this.processes.Remove(PID);

            String name = Path.GetFileName(path);
            newTab.SetBridge(this.lycaniteBridge);

            newTab.AddPath(path);

            this.metroSetLabel1.Visible = false;
            this.metroSetTabControl1.Visible = true;
            this.openExecButton.Location = new Point(700, 40);
            this.openExecButton.Size = new Size(80, 30);

            TabPage tab = new TabPage();
            tab.Controls.Add(newTab);
            newTab.Dock = DockStyle.Fill;
            tab.Text = name;
            newTab.SetUUID(UUID);
            this.metroSetTabControl1.Controls.Add(tab);

            this.tabPages.Add(UUID, tab);
        }

        private void CreateProcess(String fileName)
        {

            ProcessStartInfo startInfo = new ProcessStartInfo(fileName);
            try {
                Process proc_tmp = Process.Start(startInfo);
                if (proc_tmp == null) {
                    return;
                }

                this.processes.Add((ulong)proc_tmp.Id, fileName);
            } catch (Exception ex) {
                Console.WriteLine(ex.Message);
                return;
            }
        }

        private void Form1_DragDrop(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);

            foreach (string file in files)
            {
                if (File.Exists(file))
                {
                    this.CreateProcess(file);
                }
            }
        }

        private void OpenExecButton_Click(object sender, EventArgs e)
        {
            OpenFileDialog fileDialog = new OpenFileDialog();

            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                this.CreateProcess(fileDialog.FileName);
            }
        }

        private void MetroSetTabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (this.metroSetTabControl1.TabCount == 0)
            {
                this.metroSetLabel1.Visible = true;
                this.metroSetTabControl1.Visible = false;
                this.openExecButton.Location = new Point(276, 355);
                this.openExecButton.Size = new Size(249, 46);
            }
        }
    }
}
