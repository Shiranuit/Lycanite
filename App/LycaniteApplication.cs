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

namespace App
{
    public partial class LycaniteApplication : MetroSetForm
    {
        public LycaniteApplication()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.All;
        }

        private void Form1_DragDrop(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop, false);
            String name = "";

            foreach (string file in files)
            {
                if (File.Exists(file))
                {
                    MessageBox.Show(file);
                    name = file.Split('\\')[file.Split('\\').Length - 1];
                }
            }
            TabTemplate newTab = new TabTemplate();

            TabPage tab = new TabPage();
            tab.Controls.Add(newTab);
            newTab.Dock = DockStyle.Fill;
            tab.Text = name;
            
            metroSetTabControl1.Controls.Add(tab);

            metroSetLabel1.Visible = false;
            metroSetTabControl1.Visible = true;
        }

        private void metroSetTabControl1_Click(object sender, EventArgs e)
        {
            MetroSet_UI.Controls.MetroSetTabControl obj = (MetroSet_UI.Controls.MetroSetTabControl)sender;
            MouseEventArgs me = (MouseEventArgs)e;
/*            obj.TabIndex

            if (me.Button == MouseButtons.Right)
                obj.TabPages.Remove(obj.TabPages.IndexOfKey );
*/
        }
    }
}
