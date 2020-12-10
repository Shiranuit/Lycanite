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
using System.Threading;

namespace App
{
    public partial class LycaniteApplication : MetroSetForm
    {

        Dictionary<String, TabTemplate> tabDictionary = new Dictionary<String, TabTemplate>();
        int count = 0;

        public LycaniteApplication()
        {
            InitializeComponent();
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

                for (int i = 0; tabDictionary.Count != i; i++)
                {
                    if (tabDictionary[i.ToString()].GetChart().IsHandleCreated)
                    {
                        Invoke((MethodInvoker)delegate { tabDictionary[i.ToString()].UpdateGraph(networkUsageArray); });
                    }

                    Thread.Sleep(1000);
                }
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            metroSetTabControl1.Padding = new Point(20, 4);
            TabTemplate tabTemplate = new TabTemplate();
            graphThread = new Thread(new ThreadStart(getPerformanceCounters));
            graphThread.IsBackground = true;
            graphThread.Start();
        }

        private void Form1_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.All;
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
                    name = Path.GetFileName(file);

                    newTab.addPath(file);
                }
            }
            TabPage tab = new TabPage();
            tab.Controls.Add(newTab);
            newTab.Dock = DockStyle.Fill;
            tab.Text = name;
    
            metroSetTabControl1.Controls.Add(tab);

            tabDictionary.Add(count.ToString(), newTab);
            count++;
            metroSetLabel1.Visible = false;
            metroSetTabControl1.Visible = true;
        }
    }
}
