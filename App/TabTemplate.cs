using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using MetroSet_UI.Components;
using MetroSet_UI.Enums;
using MetroSet_UI.Interfaces;
using System.Diagnostics;
using System.Diagnostics;
using System.Threading;

namespace App
{
    public partial class TabTemplate : UserControl, IMetroSetControl
    {
        private Dictionary<String, String> _fileListAuthorize = new Dictionary<String, String>();
        private String _appPath = "";
        public TabTemplate()
        {
            InitializeComponent();
        }

        public Style Style { get; set; }
        public StyleManager StyleManager { get; set; }
        public string ThemeAuthor { get; set; }
        public string ThemeName { get; set; }
        public bool IsDerivedStyle { get; set; }

        public TabTemplate(IContainer container)
        {
            container.Add(this);

            InitializeComponent();
        }

        public void addDirList(String filename)
        {
            listView1.ForeColor = Color.White;

            listView1.View = View.Details;
            listView1.HeaderStyle = ColumnHeaderStyle.None;
            ColumnHeader colHeader = new ColumnHeader();
            colHeader.Width = listView1.ClientSize.Width - SystemInformation.VerticalScrollBarWidth;
            listView1.Columns.Add(colHeader);

            ListViewItem tmp_item = new ListViewItem(Path.GetFileName(filename));
            if (Directory.Exists(filename))
                tmp_item.ImageIndex = 0;
            else
                tmp_item.ImageIndex = 1;
            listView1.Items.Add(tmp_item);
        }

        public void addPath(String path)
        {
            _appPath = path;
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {
            if (_fileListAuthorize.Count() == 0 || listView1.Items.Count == 0 || listView1.SelectedItems.Count == 0)
                return;

            foreach(KeyValuePair<String, String> kvp in _fileListAuthorize)
            {
                Console.WriteLine("Key = {0}, Value = {1}", kvp.Key, kvp.Value);
            }

            _fileListAuthorize.Remove(listView1.SelectedItems[0].Text);

            foreach (KeyValuePair<String, String> kvp in _fileListAuthorize)
            {
                Console.WriteLine("Key = {0}, Value = {1}", kvp.Key, kvp.Value);
            }

            listView1.Items.Remove(listView1.SelectedItems[0]);
        }

        private void pictureBox1_MouseEnter(object sender, EventArgs e)
        {
            PictureBox boxImg = (PictureBox)sender;

            boxImg.BackColor = Color.FromArgb(100, 100, 100);
        }

        private void pictureBox1_MouseLeave(object sender, EventArgs e)
        {
            PictureBox boxImg = (PictureBox)sender;

            boxImg.BackColor = Color.FromArgb(50, 50, 50);
        }

        private void pictureBox1_MouseDown(object sender, MouseEventArgs e)
        {
            PictureBox boxImg = (PictureBox)sender;

            boxImg.BackColor = Color.Cyan;
        }

        private void pictureBox1_MouseHover(object sender, EventArgs e)
        {
            PictureBox boxImg = (PictureBox)sender;

            boxImg.BackColor = Color.FromArgb(100, 100, 100);
        }

        private void pictureBox1_MouseUp(object sender, MouseEventArgs e)
        {
            PictureBox boxImg = (PictureBox)sender;

            boxImg.BackColor = Color.FromArgb(100, 100, 100);
        }

        private void metroSetSwitch1_SwitchedChanged(object sender)
        {
            MetroSet_UI.Controls.MetroSetSwitch switch_obj = (MetroSet_UI.Controls.MetroSetSwitch)sender;

            if (switch_obj.CheckState == MetroSet_UI.Enums.CheckState.Checked)
            {
                metroSetLabel2.ForeColor = Color.FromArgb(210, 210, 210);
            } else
            {
                metroSetLabel2.ForeColor = Color.FromArgb(170, 170, 170);
            }
        }

        private String[] getAllFileAndDirFromPath(String path)
        {
            String[] allFiles = Directory.GetFiles(path);
            String[] allDir = Directory.GetDirectories(path);
            List<String> allFileAndDir = new List<String>();

            foreach (string filepath in allDir)
            {
                String file_tmp = Path.GetFileName(filepath);
                allFileAndDir.Add(file_tmp);
            }
            foreach (string filepath in allFiles)
            {
                String file_tmp = Path.GetFileName(filepath);
                allFileAndDir.Add(file_tmp);
            }

            return allFileAndDir.ToArray();
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void buttonDirectory_Click(object sender, EventArgs e)
        {
            using (var folderDialog = new FolderBrowserDialog())
            {
                folderDialog.SelectedPath = _appPath;
                DialogResult result = folderDialog.ShowDialog();

                if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(folderDialog.SelectedPath))
                {
                    string[] files = Directory.GetFiles(folderDialog.SelectedPath);

                    _fileListAuthorize.Add(Path.GetFileName(folderDialog.SelectedPath), folderDialog.SelectedPath);
                    addDirList(folderDialog.SelectedPath);
                }
            }
        }

        private void buttonFile_Click(object sender, EventArgs e)
        {
            System.Windows.Forms.OpenFileDialog fileDialog = new System.Windows.Forms.OpenFileDialog();
            fileDialog.InitialDirectory = _appPath;

            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                addDirList(fileDialog.FileName);
                _fileListAuthorize.Add(Path.GetFileName(fileDialog.FileName), fileDialog.FileName);
            }
        }

        private Thread graphThread;
        private double[] networkUsageArray = new double[60];

        private void getPerformanceCounters()
        {
            var data = new PerformanceCounter("Processor Information", "% Processor Time", "_Total");

            while (true)
            {
                networkUsageArray[networkUsageArray.Length - 1] = Math.Round(data.NextValue(), 0);

                Array.Copy(networkUsageArray, 1, networkUsageArray, 0, networkUsageArray.Length - 1);

                if (performanceChart.IsHandleCreated)
                {
                    Invoke((MethodInvoker)delegate { UpdateGraph(); });
                }

                Thread.Sleep(1000);
            }
        }

        private void UpdateGraph()
        {
            performanceChart.Series["Network"].Points.Clear();

            for (int i = 0; i < networkUsageArray.Length - 1; ++i)
            {
                performanceChart.Series["Network"].Points.AddY(networkUsageArray[i]);
            }
        }

        private void metroSetTabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {
            TabPage current = (sender as TabControl).SelectedTab;

            if (current.Text == "Network")
            {
                graphThread = new Thread(new ThreadStart(getPerformanceCounters));
                graphThread.IsBackground = true;
                graphThread.Start();
            }
        }
    }
}
