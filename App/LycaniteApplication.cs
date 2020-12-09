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
    public partial class LycaniteApplication : MetroSetForm
    {
        private Dictionary<String, Process> _processes = new Dictionary<String, Process>();
        public LycaniteApplication()
        {
            InitializeComponent();
            metroSetTabControl1.TabClose += closeProcess;
        }

        public void closeProcess(String path)
        {
            foreach (KeyValuePair<String, Process> kvp in _processes)
            {
                Console.WriteLine("Key = {0}", kvp.Key);
            }
            Console.WriteLine("Path = {0}", path);
            try
            {
                KillAllProcessesSpawnedBy((UInt32)_processes[path].Id);
                _processes[path].CloseMainWindow();
                _processes.Remove(path);
                Console.WriteLine("\n\nDELETEDDD\n\n");
            }
            catch(Win32Exception ex)
            {
                Console.WriteLine(ex.Message);
                Console.WriteLine("\n\nNOT     DELETEDDD\n\n");
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            metroSetTabControl1.Padding = new Point(20, 4);
        }

        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.All;
        }

        public void processIsClosed(object sender, EventArgs e)
        {
            Console.WriteLine("NEED TO DELETE THE TAB");
        }

        private static void KillAllProcessesSpawnedBy(UInt32 parentProcessId)
        {
            Console.WriteLine("Finding processes spawned by process with Id [" + parentProcessId + "]");

            // NOTE: Process Ids are reused!
            System.Management.ManagementObjectSearcher searcher = new System.Management.ManagementObjectSearcher(
                "SELECT * " +
                "FROM Win32_Process " +
                "WHERE ParentProcessId=" + parentProcessId);
            System.Management.ManagementObjectCollection collection = searcher.Get();
            if (collection.Count > 0)
            {
                Console.WriteLine("Killing [" + collection.Count + "] processes spawned by process with Id [" + parentProcessId + "]");
                foreach (var item in collection)
                {
                    UInt32 childProcessId = (UInt32)item["ProcessId"];
                    if ((int)childProcessId != Process.GetCurrentProcess().Id)
                    {
                        KillAllProcessesSpawnedBy(childProcessId);
                        Console.WriteLine("LE PROCESS {0} n'est pas terminer {1}", childProcessId, item.ToString());
                        Process childProcess = Process.GetProcessById((int)childProcessId);
                        Console.WriteLine("Killing child process [" + childProcess.ProcessName + "] with Id [" + childProcessId + "]");
                        childProcess.Kill();
                    }
                }
            }
        }
        private void Form1_DragDrop(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);
            String name = "";
            TabTemplate newTab = new TabTemplate();
            
            foreach (string file in files)
            {
                if (File.Exists(file))
                {
                    MessageBox.Show(file);
                    name = Path.GetFileName(file);

                    newTab.addPath(file);

                    metroSetLabel1.Visible = false;
                    metroSetTabControl1.Visible = true;
                    openExecButton.Location = new Point(700, 40);
                    openExecButton.Size = new Size(80, 30);

                    TabPage tab = new TabPage();
                    tab.Controls.Add(newTab);
                    newTab.Dock = DockStyle.Fill;
                    tab.Text = name;

                    metroSetTabControl1.Controls.Add(tab);

                    ProcessStartInfo startInfo = new ProcessStartInfo(file);
                    try
                    {
                        Process proc_tmp = Process.Start(startInfo);
                        _processes.Add(name, proc_tmp);
                        foreach (KeyValuePair<String, Process> kvp in _processes)
                        {
                            Console.WriteLine("Key = {0} ", kvp.Key);
                        }
                        _processes[name].EnableRaisingEvents = true;
                        _processes[name].Exited += processIsClosed;
                    }
                    catch (Win32Exception ex)
                    {
                        Console.WriteLine(ex.Message);
                        return;
                    }
                }
            }
        }



        private void openExecButton_Click(object sender, EventArgs e)
        {
            System.Windows.Forms.OpenFileDialog fileDialog = new System.Windows.Forms.OpenFileDialog();
            TabTemplate newTab = new TabTemplate();
            String name = "";

            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                MessageBox.Show(fileDialog.FileName);
                name = Path.GetFileName(fileDialog.FileName);

                ProcessStartInfo startInfo = new ProcessStartInfo(fileDialog.FileName);
                try
                {
                    Process proc_tmp = Process.Start(startInfo);
                    _processes.Add(name, proc_tmp);
                    foreach (KeyValuePair<String, Process> kvp in _processes)
                    {
                        Console.WriteLine("Key = {0} ", kvp.Key);
                    }
                    _processes[name].EnableRaisingEvents = true;
                    _processes[name].Exited += processIsClosed;

                }
                catch (Win32Exception ex)
                {
                    Console.WriteLine(ex.Message);
                    return;
                }

                newTab.addPath(fileDialog.FileName);

                metroSetLabel1.Visible = false;
                metroSetTabControl1.Visible = true;
                openExecButton.Location = new Point(700, 40);
                openExecButton.Size = new Size(80, 30);
                

                TabPage tab = new TabPage();
                tab.Controls.Add(newTab);
                newTab.Dock = DockStyle.Fill;
                tab.Text = name;

                metroSetTabControl1.Controls.Add(tab);

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
