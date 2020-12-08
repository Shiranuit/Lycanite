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

namespace App
{
    public partial class TabTemplate : UserControl, IMetroSetControl
    {
        private String _actual_path = "";
        private String _selectedFileName = "";
        private Dictionary<String, String> _fileListAuthorize = new Dictionary<String, String>();
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
            ColumnHeader h = new ColumnHeader();
            h.Width = listView1.ClientSize.Width - SystemInformation.VerticalScrollBarWidth;
            listView1.Columns.Add(h);

            ListViewItem tmp_item = new ListViewItem(Path.GetFileName(filename));
            if (Directory.Exists(filename))
                tmp_item.ImageIndex = 0;
            else
                tmp_item.ImageIndex = 1;
            listView1.Items.Add(tmp_item);
        }

        public void addPath(String path)
        {
            _actual_path = path;
        }

        private void chart1_Click(object sender, EventArgs e)
        {

        }

        private void metroSetSetTabPage1_Click(object sender, EventArgs e)
        {

        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {
            if (_fileListAuthorize.Count() == 0 || listView1.Items.Count == 0)
                return;
            Debug.WriteLine("\n\nBEGIN: " + Path.GetFullPath(listView1.SelectedItems[0].Text) + "\n");

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

            System.Diagnostics.Debug.WriteLine(switch_obj.CheckState.ToString());

            if (switch_obj.CheckState.ToString().CompareTo("Checked") == 0)
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
            String[] allDir = (Directory.GetDirectories(path));
            List<String> allFileAndDir = new List<String>();

            foreach (string filepath in allDir)
            {
                String file_tmp = filepath.Split('\\')[filepath.Split('\\').Length - 1];
                allFileAndDir.Add(file_tmp);
            }
            foreach (string filepath in allFiles)
            {
                String file_tmp = filepath.Split('\\')[filepath.Split('\\').Length - 1];
                allFileAndDir.Add(file_tmp);
            }

            _actual_path = path;

            return allFileAndDir.ToArray();
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void listView1_DoubleClick(object sender, EventArgs e)
        {
            ListView listview = (ListView)sender;

            _selectedFileName = listview.Items[listview.SelectedIndices[0]].Text;

/*            if (Directory.Exists(_actual_path + "\\" + name))
            {
                addDirList(getAllFileAndDirFromPath(_actual_path + "\\" + name));
            }*/
        }

        private void buttonDirectory_Click(object sender, EventArgs e)
        {
            using (var fbd = new FolderBrowserDialog())
            {
                fbd.SelectedPath = Path.Combine(_actual_path, _selectedFileName);
                DialogResult result = fbd.ShowDialog();

                if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
                {
                    string[] files = Directory.GetFiles(fbd.SelectedPath);
                    /*foreach (String file in files)
                        _fileListAuthorize.Add(file, Path.Combine(fbd.SelectedPath, file));*/
                    _fileListAuthorize.Add(Path.GetFileName(fbd.SelectedPath), fbd.SelectedPath);
                    addDirList(fbd.SelectedPath);
                }
            }
        }

        private void buttonFile_Click(object sender, EventArgs e)
        {
            System.Windows.Forms.OpenFileDialog fileDialog = new System.Windows.Forms.OpenFileDialog();
            System.Windows.Forms.FolderBrowserDialog dirDialog = new System.Windows.Forms.FolderBrowserDialog();
            fileDialog.InitialDirectory = Path.Combine(_actual_path, _selectedFileName);
            fileDialog.Filter = "All files (*.*)|*.*";

            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                addDirList(fileDialog.FileName);
                _fileListAuthorize.Add(Path.GetFileName(fileDialog.FileName), fileDialog.FileName);
            }
        }
    }
}
