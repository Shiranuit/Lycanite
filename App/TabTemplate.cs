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

        public void addDirList(String[] filename)
        {
            listView1.ForeColor = Color.White;
            listView1.Clear();

            listView1.View = View.Details;
            listView1.HeaderStyle = ColumnHeaderStyle.None;
            ColumnHeader h = new ColumnHeader();
            h.Width = listView1.ClientSize.Width - SystemInformation.VerticalScrollBarWidth;
            listView1.Columns.Add(h);

            foreach (String file in filename)
            {
                ListViewItem tmp_item = new ListViewItem(file);
                if (Directory.Exists(_actual_path + "\\" + file))
                    tmp_item.ImageIndex = 0;
                else
                    tmp_item.ImageIndex = 1;
                listView1.Items.Add(tmp_item);
            }
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
            int i = _actual_path.Length - 1;
            for (; _actual_path[i].CompareTo('\\') != 0; i--) ;
            _actual_path = _actual_path.Substring(0, i);

            addDirList(getAllFileAndDirFromPath(_actual_path));
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

            String name = listview.Items[listview.SelectedIndices[0]].Text;

            if (Directory.Exists(_actual_path + "\\" + name))
            {
                addDirList(getAllFileAndDirFromPath(_actual_path + "\\" + name));
            }
        }
    }
}
