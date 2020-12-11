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

namespace App
{
    public delegate void NotifyProcessClose(Process process);
    public partial class LycaniteApplication : MetroSetForm
    {
        private int mainThreadId = System.Threading.Thread.CurrentThread.ManagedThreadId;
        private Dictionary<int, Process> _processes = new Dictionary<int, Process>();
        public LycaniteApplication()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            metroSetTabControl1.Padding = new Point(20, 4);
        }

        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.All;
        }

        // Do this when you start your application
        
        private void closeTab(Process process)
        {
            foreach (TabPage page in metroSetTabControl1.TabPages)
            {
                if (page.Tag.ToString().CompareTo(_processes[process.Id].Id.ToString()) == 0)
                {
                    metroSetTabControl1.TabPages.Remove(page);
                }
            }
        }

        public void processIsClosed(object sender, EventArgs e)
        {
            if (this.InvokeRequired)
                this.Invoke(new NotifyProcessClose(closeTab), (Process)sender);
            else
                closeTab((Process)sender);
        }

        void createProcessAndTab(String fileName)
        {
            TabTemplate newTab = new TabTemplate();
            MessageBox.Show(fileName);
            String name = Path.GetFileName(fileName);
            int id = 0;

            ProcessStartInfo startInfo = new ProcessStartInfo(fileName);
            try
            {
                Process proc_tmp = Process.Start(startInfo);
                if (proc_tmp == null)
                {
                    return;
                }
                _processes.Add(proc_tmp.Id, proc_tmp);

                _processes[proc_tmp.Id].EnableRaisingEvents = true;
                _processes[proc_tmp.Id].Exited += processIsClosed;
                id = proc_tmp.Id;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                return;
            }

            newTab.addPath(fileName);

            metroSetLabel1.Visible = false;
            metroSetTabControl1.Visible = true;
            openExecButton.Location = new Point(700, 40);
            openExecButton.Size = new Size(80, 30);

            // Create tab only when receive event with id from driver
            TabPage tab = new TabPage();
            tab.Controls.Add(newTab);
            newTab.Dock = DockStyle.Fill;
            tab.Text = name;
            if (id != 0)
                tab.Tag = id;
            metroSetTabControl1.Controls.Add(tab);
        }

        private void Form1_DragDrop(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);
            String name = "";
            TabTemplate newTab = new TabTemplate();
            int id = 0;

            foreach (string file in files)
            {
                if (File.Exists(file))
                {
                    createProcessAndTab(file);
                }
            }
        }

        private void openExecButton_Click(object sender, EventArgs e)
        {
            System.Windows.Forms.OpenFileDialog fileDialog = new System.Windows.Forms.OpenFileDialog();

            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                createProcessAndTab(fileDialog.FileName);
            }
        }

        private void metroSetTabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (metroSetTabControl1.TabCount == 0)
            {
                metroSetLabel1.Visible = true;
                metroSetTabControl1.Visible = false;
                openExecButton.Location = new Point(276, 355);
                openExecButton.Size = new Size(249, 46);
            }
        }
    }
}
