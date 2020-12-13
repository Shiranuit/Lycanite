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
using Lycanite;

namespace App
{
    public partial class TabTemplate : UserControl, IMetroSetControl {
        private Dictionary<String, Dictionary<String, ELycanitePerm>> fileListAuthorize = new Dictionary<String, Dictionary<String, ELycanitePerm>>();
        private String appPath = "";
        private LycaniteBridge lycaniteBridge;
        private ulong UUID = new ulong();
        public TabTemplate()
        {
            this.InitializeComponent();
        }

        public Style Style { get; set; }
        public StyleManager StyleManager { get; set; }
        public string ThemeAuthor { get; set; }
        public string ThemeName { get; set; }
        public bool IsDerivedStyle { get; set; }

        public TabTemplate(IContainer container)
        {
            container.Add(this);

            this.InitializeComponent();
        }

        public void AddPathToList(String filename)
        {
            this.listView1.ForeColor = Color.White;

            this.listView1.View = View.Details;
            this.listView1.HeaderStyle = ColumnHeaderStyle.None;
            ColumnHeader colHeader = new ColumnHeader();
            colHeader.Width = this.listView1.ClientSize.Width - SystemInformation.VerticalScrollBarWidth;
            this.listView1.Columns.Add(colHeader);

            ListViewItem tmp_item = new ListViewItem(Path.GetFileName(filename));
            tmp_item.ImageIndex = Directory.Exists(filename) ? 0 : 1;
            tmp_item.Tag = filename;
            this.listView1.Items.Add(tmp_item);
        }

        public ulong GetUUID() {
            return this.UUID;
        }

        public void AddPath(String path)
        {
            this.appPath = path;
        }

        public void SetUUID(ulong UUID) {
            this.UUID = UUID;
        }

        private void PictureBox1_Click(object sender, EventArgs e)
        {
            if (this.fileListAuthorize.Count() == 0 || this.listView1.Items.Count == 0 || this.listView1.SelectedItems.Count == 0)
                return;

            String filePath = this.listView1.SelectedItems[0].Tag.ToString();

            String convertedPath = DevicePathMapper.ToDevicePath(filePath);

            if (convertedPath == null)
                return;
            if (!this.lycaniteBridge.DeletePIDFilePermissions(this.UUID, convertedPath))
                return;

            this.fileListAuthorize.Remove(filePath);
            this.listView1.Items.Remove(this.listView1.SelectedItems[0]);
        }

        private void PictureBox1_MouseEnter(object sender, EventArgs e)
        {
            PictureBox boxImg = (PictureBox)sender;

            boxImg.BackColor = Color.FromArgb(100, 100, 100);
        }

        private void PictureBox1_MouseLeave(object sender, EventArgs e)
        {
            PictureBox boxImg = (PictureBox)sender;

            boxImg.BackColor = Color.FromArgb(50, 50, 50);
        }

        private void PictureBox1_MouseDown(object sender, MouseEventArgs e)
        {
            PictureBox boxImg = (PictureBox)sender;

            boxImg.BackColor = Color.Cyan;
        }

        private void PictureBox1_MouseHover(object sender, EventArgs e)
        {
            PictureBox boxImg = (PictureBox)sender;

            boxImg.BackColor = Color.FromArgb(100, 100, 100);
        }

        private void PictureBox1_MouseUp(object sender, MouseEventArgs e)
        {
            PictureBox boxImg = (PictureBox)sender;

            boxImg.BackColor = Color.FromArgb(100, 100, 100);
        }

        private void MetroSetSwitch1_SwitchedChanged(object sender)
        {
            MetroSet_UI.Controls.MetroSetSwitch switch_obj = (MetroSet_UI.Controls.MetroSetSwitch)sender;

            this.metroSetLabel2.ForeColor = switch_obj.CheckState == MetroSet_UI.Enums.CheckState.Checked ? Color.FromArgb(210, 210, 210) : Color.FromArgb(170, 170, 170);
        }

        public void SetBridge(LycaniteBridge lycaniteBridge)
        {
            this.lycaniteBridge = lycaniteBridge;
        }

        private void ListView1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (this.listView1.SelectedItems.Count <= 0) {
                this.readCheckbox.Visible = false;
                this.writeCheckbox.Visible = false;
            } else {
                Console.WriteLine(this.listView1.SelectedItems[0].Tag.ToString());
                this.readCheckbox.Visible = true;
                this.writeCheckbox.Visible = true;

                

                Dictionary<String, ELycanitePerm> fileinfo = this.fileListAuthorize[this.listView1.SelectedItems[0].Tag.ToString()];

                this.readCheckbox.Checked = fileinfo["read"] == ELycanitePerm.LYCANITE_READ;
                this.writeCheckbox.Checked = fileinfo["write"] == ELycanitePerm.LYCANITE_WRITE;
            }
        }

        private Dictionary<string, ELycanitePerm> NewPerms() {
            return new Dictionary<string, ELycanitePerm> { 
                { "read", ELycanitePerm.LYCANITE_NONE }, 
                { "write", ELycanitePerm.LYCANITE_NONE }, 
                { "delete", ELycanitePerm.LYCANITE_NONE } 
            };
        }

        private void ButtonFile_Click(object sender, EventArgs e)
        {
            System.Windows.Forms.OpenFileDialog fileDialog = new System.Windows.Forms.OpenFileDialog();
            fileDialog.ValidateNames = false;
            fileDialog.InitialDirectory = this.appPath;
            fileDialog.Multiselect = true;
            fileDialog.CheckPathExists = true;
            fileDialog.CheckFileExists = false;
            fileDialog.FileName = "Default";

            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                if (File.Exists(fileDialog.FileName))
                {
                    if (this.fileListAuthorize.ContainsKey(fileDialog.FileName))
                    {
                        return;
                    } else
                    {
                        this.fileListAuthorize.Add(fileDialog.FileName, this.NewPerms());
                        this.AddPathToList(fileDialog.FileName);
                    }
                }
                else if (Directory.Exists(Path.GetDirectoryName(fileDialog.FileName)))
                {
                    fileDialog.FileName = Path.GetDirectoryName(fileDialog.FileName);
                    if (this.fileListAuthorize.ContainsKey(fileDialog.FileName))
                    {
                        return;
                    } else
                    {
                        this.AddPathToList(fileDialog.FileName);
                        this.fileListAuthorize.Add(fileDialog.FileName, this.NewPerms());
                    }
                }
            }

        }

        private Thread graphThread;
        private double[] networkUsageArray = new double[60];

        private void GetPerformanceCounters()
        {
            PerformanceCounter data = new PerformanceCounter("Processor Information", "% Processor Time", "_Total");

            while (true)
            {
                this.networkUsageArray[this.networkUsageArray.Length - 1] = Math.Round(data.NextValue(), 0);

                Array.Copy(this.networkUsageArray, 1, this.networkUsageArray, 0, this.networkUsageArray.Length - 1);

                if (this.performanceChart.IsHandleCreated)
                {
                    this.Invoke((MethodInvoker)delegate { this.UpdateGraph(); });
                }

                Thread.Sleep(1000);
            }
        }

        private void UpdateGraph()
        {
            this.performanceChart.Series["Network"].Points.Clear();

            for (int i = 0; i < this.networkUsageArray.Length - 1; ++i)
            {
                this.performanceChart.Series["Network"].Points.AddY(this.networkUsageArray[i]);
            }
        }

        private void MetroSetTabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {
            TabPage current = (sender as TabControl).SelectedTab;

            if (current.Text == "Network")
            {
               /* this.graphThread = new Thread(new ThreadStart(this.GetPerformanceCounters));
                this.graphThread.IsBackground = true;
                this.graphThread.Start();*/
            }
        }

        private void ReadCheckbox_CheckedChanged(Object sender) {
            MetroSet_UI.Controls.MetroSetCheckBox checkbox = (MetroSet_UI.Controls.MetroSetCheckBox)sender;

            if (this.listView1.SelectedItems.Count <= 0)
                return;
            String fileName = this.listView1.SelectedItems[0].Tag.ToString();
            Dictionary<String, ELycanitePerm> dict = this.fileListAuthorize[fileName];

            dict["read"] = checkbox.Checked ? ELycanitePerm.LYCANITE_READ : ELycanitePerm.LYCANITE_NONE;


            String convertedPath = DevicePathMapper.ToDevicePath(fileName);

            if (convertedPath == null)
                return;

            if (!this.lycaniteBridge.SetPIDFilePermissions(this.UUID, convertedPath, dict["read"] | dict["write"] | dict["delete"]))
                MessageBox.Show("Error: Permission can't be assigned to the file {0}", Path.GetFileName(fileName));
        }

        private void WriteCheckbox_CheckedChanged(Object sender) {
            MetroSet_UI.Controls.MetroSetCheckBox checkbox = (MetroSet_UI.Controls.MetroSetCheckBox)sender;

            if (this.listView1.SelectedItems.Count <= 0)
                return;

            String fileName = this.listView1.SelectedItems[0].Tag.ToString();
            Dictionary<String, ELycanitePerm> dict = this.fileListAuthorize[fileName];

            dict["write"] = checkbox.Checked ? ELycanitePerm.LYCANITE_WRITE : ELycanitePerm.LYCANITE_NONE;

            String convertedPath = DevicePathMapper.ToDevicePath(fileName);

            if (convertedPath == null)
                return;

            if (!this.lycaniteBridge.SetPIDFilePermissions(this.UUID, convertedPath, dict["read"] | dict["write"] | dict["delete"]))
                MessageBox.Show("Error: Permission can't be assigned to the file {0}", Path.GetFileName(fileName));
        }
    }
}
