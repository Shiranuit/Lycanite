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
using System.Threading;

namespace App
{
    public delegate void NotifyProcessClose(Process process);

    public partial class LycaniteApplication : MetroSetForm
    {
        private Dictionary<ulong, ulong> processes = new Dictionary<ulong, ulong>();
        private LycaniteBridge lycaniteBridge = new LycaniteBridge();
        private Dictionary<ulong, TabPage> tabPages = new Dictionary<ulong, TabPage>();
        private static Semaphore semaphore = new Semaphore(0, 1);
        private GlobalPathForm globalForm;

        protected override void OnLoad(EventArgs e) {
            base.UseSlideAnimation = false;
            base.OnLoad(e);
        }

        public LycaniteApplication()
        {
            this.InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            this.metroSetTabControl1.Padding = new Point(20, 4);
  /*          if (!this.lycaniteBridge.Connect()) {
                this.BeginInvoke(new MethodInvoker(this.Close));
            } else {
                int processId = Process.GetCurrentProcess().Id;
                this.lycaniteBridge.SetLycanitePID((ulong)processId);
                this.lycaniteBridge.OnLycaniteEvent += this.ProcessLycanite;
            }*/
            this.globalForm = new GlobalPathForm(this);
            this.globalForm.SetBridge(this.lycaniteBridge);
            this.globalForm.Location = new Point(this.Location.X + this.Size.Width, this.Location.Y);
            this.BeginInvoke(new MethodInvoker(this.globalForm.Show));
            this.AllowResize = false;
        }

        private void ProcessLycanite(LycaniteEvent e) {
            switch (e.eventType) {
                case ELycaniteEventType.PROCESS_CREATE:
                    this.BeginInvoke((MethodInvoker)delegate () {
                        this.CloseTab(e.UUID);
                        this.processes[e.ProcessID] = e.UUID;
                    });
                    break;
                case ELycaniteEventType.PROCESS_DESTROY:
                    this.BeginInvoke((MethodInvoker)delegate () {
                        this.CloseTab(e.UUID);
                    });
                    break;
                case ELycaniteEventType.PROCESS_REQPERM:
                    break;
                default:
                    break;
            }
        }

        private void CloseTab(ulong UUID) {

            TabPage page = null;
            if (this.tabPages.TryGetValue(UUID, out page)) {
                this.tabPages.Remove(UUID);
                this.metroSetTabControl1.TabPages.Remove(page);
            }
        }

        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.All;
        }

        private void CreateTab(ulong UUID, String path) {
                TabTemplate newTab = new TabTemplate();

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
                this.metroSetTabControl1.TabPages.Add(tab);

                this.tabPages[UUID] = tab;
        }

        private void CreateProcess(String fileName)
        {
           ProcessStartInfo startInfo = new ProcessStartInfo(fileName);
           try {
                Process proc_tmp = Process.Start(startInfo);
                if (proc_tmp == null) {
                    return;
                }
                ulong UUID;
                if (this.processes.TryGetValue((ulong)proc_tmp.Id, out UUID)) {
                    this.CreateTab(UUID, fileName);
                    this.processes.Remove((ulong)proc_tmp.Id);
                }
            } catch (Exception ex) {
                Console.WriteLine(ex.Message);
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

            this.openExecButton.Location = new Point(680, 40);
            this.openExecButton.Size = new Size(100, 30);
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

        private void LycaniteApplication_FormClosing(Object sender, FormClosingEventArgs e) {
            this.lycaniteBridge.Disconnect();
        }

        private void LycaniteApplication_LocationChanged(Object sender, EventArgs e) {
            MetroSetForm Lycanite = (MetroSetForm)sender;
            if (Lycanite.Location != null && this.globalForm != null)
                this.globalForm.Location = new Point(Lycanite.Location.X + Lycanite.Size.Width, Lycanite.Location.Y);
        }

        private void LycaniteApplication_Enter(Object sender, EventArgs e) {
            
        }
    }
}
