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
        private Dictionary<int, Process> _processes = new Dictionary<int, Process>();
        private LycaniteBridge lycaniteBridge = new LycaniteBridge();

        public LycaniteApplication()
        {
            this.InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            this.metroSetTabControl1.Padding = new Point(20, 4);
            /*if (!this.lycaniteBridge.Connect()) {
                this.BeginInvoke(new MethodInvoker(this.Close));
            }*/
        }

        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.All;
        }

        // Do this when you start your application
        
        private void CloseTab(Process process)
        {
            foreach (TabPage page in this.metroSetTabControl1.TabPages)
            {
                if (page.Tag.ToString().CompareTo(this._processes[process.Id].Id.ToString()) == 0)
                {
                    this.metroSetTabControl1.TabPages.Remove(page);
                }
            }
        }

        public void ProcessIsClosed(object sender, EventArgs e)
        {
            if (this.InvokeRequired)
                this.Invoke(new NotifyProcessClose(this.CloseTab), (Process)sender);
            else
                this.CloseTab((Process)sender);
        }

        private void CreateProcessAndTab(String fileName)
        {
            TabTemplate newTab = new TabTemplate();
            String name = Path.GetFileName(fileName);
            int id = 0;

            newTab.SetBridge(this.lycaniteBridge);

            ProcessStartInfo startInfo = new ProcessStartInfo(fileName);
            try
            {
                Process proc_tmp = Process.Start(startInfo);
                if (proc_tmp == null)
                {
                    return;
                }
                this._processes.Add(proc_tmp.Id, proc_tmp);

                this._processes[proc_tmp.Id].EnableRaisingEvents = true;
                this._processes[proc_tmp.Id].Exited += this.ProcessIsClosed;
                id = proc_tmp.Id;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                return;
            }

            newTab.AddPath(fileName);

            this.metroSetLabel1.Visible = false;
            this.metroSetTabControl1.Visible = true;
            this.openExecButton.Location = new Point(700, 40);
            this.openExecButton.Size = new Size(80, 30);

            // Create tab only when receive event with id from driver
            TabPage tab = new TabPage();
            tab.Controls.Add(newTab);
            newTab.Dock = DockStyle.Fill;
            tab.Text = name;
            if (id != 0) {
                tab.Tag = id;
                newTab.SetPid((ulong)id);
            }
            this.metroSetTabControl1.Controls.Add(tab);
        }

        private void Form1_DragDrop(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);

            foreach (string file in files)
            {
                if (File.Exists(file))
                {
                    this.CreateProcessAndTab(file);
                }
            }
        }

        private void OpenExecButton_Click(object sender, EventArgs e)
        {
            OpenFileDialog fileDialog = new OpenFileDialog();

            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                this.CreateProcessAndTab(fileDialog.FileName);
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
