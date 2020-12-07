using System;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Design;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using MetroSet_UI.Animates;
using MetroSet_UI.Child;
using MetroSet_UI.Components;
using MetroSet_UI.Design;
using MetroSet_UI.Enums;
using MetroSet_UI.Extensions;
using MetroSet_UI.Interfaces;
using MetroSet_UI.Controls;
using System.Windows.Forms;
using MetroSet_UI.Forms;
using System.IO;
using System.Drawing;

namespace MetroSet_UI.Controls
{
    [ToolboxItem(true)]
    [ToolboxBitmap(typeof(TabController), "Bitmaps.TabControl.bmp")]
    [Designer(typeof(MetroSetTabControlDesigner))]
    [ComVisible(true)]

    public class TabController : MetroSetTabControl
    {
        public TabController() : base()
        {
            
        }

        private void DrawTab(int i, Graphics g)
        {
            Rectangle tabRect = GetTabRect(i);
            tabRect.Offset(2, 2);
            Brush TitleBrush = new SolidBrush(Color.White);
            g.DrawString("x", Font, TitleBrush, new Point(tabRect.X + (tabRect.Width - 16), 16));
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            var graphics = e.Graphics;

            graphics.TextRenderingHint = System.Drawing.Text.TextRenderingHint.ClearTypeGridFit;

            switch (TabStyle)
            {
                case TabStyle.Style1:

                    for (var i = 0; i <= TabCount - 1; i++)
                    {
                        DrawTab(i, graphics);
                    }
                    break;
                case TabStyle.Style2:
                    for (var i = 0; i <= TabCount - 1; i++)
                    {
                        DrawTab(i, graphics);
                    }
                    break;
            }
        }

        protected override void OnMouseClick(MouseEventArgs e)
        {
            base.OnMouseClick(e);

            Size sizeText = TextRenderer.MeasureText("x", Font);

            for (var i = 0; i <= TabCount - 1; i++)
            {
                Rectangle r = GetTabRect(i);

                if (r.Contains(e.Location))
                {
                    Rectangle rect = new Rectangle(r.X + (r.Width - 16), 16, sizeText.Width, sizeText.Height);

                    if (rect.Contains(e.Location))
                    {
                        TabPage tabPage = sTabPages[i];
                        TabPages.Remove(tabPage);
                    }
                }
            }
        }
    }
}
