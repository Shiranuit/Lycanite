
namespace App {
    partial class GlobalPathForm {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.components = new System.ComponentModel.Container();
            this.addNewGlobalPath = new MetroSet_UI.Controls.MetroSetButton();
            this.removeGlobalPath = new MetroSet_UI.Controls.MetroSetButton();
            this.globalPathBox = new MetroSet_UI.Controls.MetroSetListBox();
            this.readPermissionBox = new MetroSet_UI.Controls.MetroSetCheckBox();
            this.writePermissionBox = new MetroSet_UI.Controls.MetroSetCheckBox();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // addNewGlobalPath
            // 
            this.addNewGlobalPath.DisabledBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(120)))), ((int)(((byte)(65)))), ((int)(((byte)(177)))), ((int)(((byte)(225)))));
            this.addNewGlobalPath.DisabledBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(120)))), ((int)(((byte)(65)))), ((int)(((byte)(177)))), ((int)(((byte)(225)))));
            this.addNewGlobalPath.DisabledForeColor = System.Drawing.Color.Gray;
            this.addNewGlobalPath.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.addNewGlobalPath.HoverBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(95)))), ((int)(((byte)(207)))), ((int)(((byte)(255)))));
            this.addNewGlobalPath.HoverColor = System.Drawing.Color.FromArgb(((int)(((byte)(95)))), ((int)(((byte)(207)))), ((int)(((byte)(255)))));
            this.addNewGlobalPath.HoverTextColor = System.Drawing.Color.White;
            this.addNewGlobalPath.IsDerivedStyle = true;
            this.addNewGlobalPath.Location = new System.Drawing.Point(15, 385);
            this.addNewGlobalPath.Name = "addNewGlobalPath";
            this.addNewGlobalPath.NormalBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(65)))), ((int)(((byte)(177)))), ((int)(((byte)(225)))));
            this.addNewGlobalPath.NormalColor = System.Drawing.Color.FromArgb(((int)(((byte)(65)))), ((int)(((byte)(177)))), ((int)(((byte)(225)))));
            this.addNewGlobalPath.NormalTextColor = System.Drawing.Color.White;
            this.addNewGlobalPath.PressBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(35)))), ((int)(((byte)(147)))), ((int)(((byte)(195)))));
            this.addNewGlobalPath.PressColor = System.Drawing.Color.FromArgb(((int)(((byte)(35)))), ((int)(((byte)(147)))), ((int)(((byte)(195)))));
            this.addNewGlobalPath.PressTextColor = System.Drawing.Color.White;
            this.addNewGlobalPath.Size = new System.Drawing.Size(196, 50);
            this.addNewGlobalPath.Style = MetroSet_UI.Enums.Style.Dark;
            this.addNewGlobalPath.StyleManager = null;
            this.addNewGlobalPath.TabIndex = 0;
            this.addNewGlobalPath.Text = "Add New Global Path";
            this.addNewGlobalPath.ThemeAuthor = "Narwin";
            this.addNewGlobalPath.ThemeName = "MetroDark";
            this.addNewGlobalPath.Click += new System.EventHandler(this.AddNewGlobalPath_Click);
            // 
            // removeGlobalPath
            // 
            this.removeGlobalPath.DisabledBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(120)))), ((int)(((byte)(65)))), ((int)(((byte)(177)))), ((int)(((byte)(225)))));
            this.removeGlobalPath.DisabledBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(120)))), ((int)(((byte)(65)))), ((int)(((byte)(177)))), ((int)(((byte)(225)))));
            this.removeGlobalPath.DisabledForeColor = System.Drawing.Color.Gray;
            this.removeGlobalPath.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.removeGlobalPath.HoverBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(189)))), ((int)(((byte)(4)))), ((int)(((byte)(9)))));
            this.removeGlobalPath.HoverColor = System.Drawing.Color.FromArgb(((int)(((byte)(189)))), ((int)(((byte)(4)))), ((int)(((byte)(9)))));
            this.removeGlobalPath.HoverTextColor = System.Drawing.Color.White;
            this.removeGlobalPath.IsDerivedStyle = true;
            this.removeGlobalPath.Location = new System.Drawing.Point(240, 385);
            this.removeGlobalPath.Name = "removeGlobalPath";
            this.removeGlobalPath.NormalBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(159)))), ((int)(((byte)(4)))), ((int)(((byte)(9)))));
            this.removeGlobalPath.NormalColor = System.Drawing.Color.FromArgb(((int)(((byte)(159)))), ((int)(((byte)(4)))), ((int)(((byte)(9)))));
            this.removeGlobalPath.NormalTextColor = System.Drawing.Color.White;
            this.removeGlobalPath.PressBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(129)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.removeGlobalPath.PressColor = System.Drawing.Color.FromArgb(((int)(((byte)(129)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.removeGlobalPath.PressTextColor = System.Drawing.Color.White;
            this.removeGlobalPath.Size = new System.Drawing.Size(195, 50);
            this.removeGlobalPath.Style = MetroSet_UI.Enums.Style.Custom;
            this.removeGlobalPath.StyleManager = null;
            this.removeGlobalPath.TabIndex = 1;
            this.removeGlobalPath.Text = "Remove Selected Global Path";
            this.removeGlobalPath.ThemeAuthor = "Narwin";
            this.removeGlobalPath.ThemeName = "MetroLite";
            this.removeGlobalPath.Click += new System.EventHandler(this.RemoveGlobalPath_Click);
            // 
            // globalPathBox
            // 
            this.globalPathBox.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(25)))), ((int)(((byte)(25)))), ((int)(((byte)(25)))));
            this.globalPathBox.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.globalPathBox.DisabledBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(80)))), ((int)(((byte)(80)))), ((int)(((byte)(80)))));
            this.globalPathBox.DisabledForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(109)))), ((int)(((byte)(109)))), ((int)(((byte)(109)))));
            this.globalPathBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.globalPathBox.HoveredItemBackColor = System.Drawing.Color.LightGray;
            this.globalPathBox.HoveredItemColor = System.Drawing.Color.DimGray;
            this.globalPathBox.IsDerivedStyle = false;
            this.globalPathBox.ItemHeight = 30;
            this.globalPathBox.Location = new System.Drawing.Point(15, 50);
            this.globalPathBox.MultiSelect = false;
            this.globalPathBox.Name = "globalPathBox";
            this.globalPathBox.SelectedIndex = -1;
            this.globalPathBox.SelectedItem = null;
            this.globalPathBox.SelectedItemBackColor = System.Drawing.Color.FromArgb(((int)(((byte)(65)))), ((int)(((byte)(177)))), ((int)(((byte)(225)))));
            this.globalPathBox.SelectedItemColor = System.Drawing.Color.White;
            this.globalPathBox.SelectedText = null;
            this.globalPathBox.SelectedValue = null;
            this.globalPathBox.ShowBorder = false;
            this.globalPathBox.ShowScrollBar = true;
            this.globalPathBox.Size = new System.Drawing.Size(424, 317);
            this.globalPathBox.Style = MetroSet_UI.Enums.Style.Custom;
            this.globalPathBox.StyleManager = null;
            this.globalPathBox.TabIndex = 2;
            this.globalPathBox.ThemeAuthor = "Narwin";
            this.globalPathBox.ThemeName = "MetroDark";
            this.globalPathBox.SelectedIndexChanged += new MetroSet_UI.Controls.MetroSetListBox.SelectedIndexChangedEventHandler(this.GlobalPathBox_SelectedValueChanged);
            this.globalPathBox.SelectedValueChanged += new MetroSet_UI.Controls.MetroSetListBox.SelectedValueEventHandler(this.GlobalPathBox_SelectedValueChanged);
            this.globalPathBox.LocationChanged += new System.EventHandler(this.GlobalPathBox_LocationChanged);
            // 
            // readPermissionBox
            // 
            this.readPermissionBox.BackColor = System.Drawing.Color.Transparent;
            this.readPermissionBox.BackgroundColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
            this.readPermissionBox.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(155)))), ((int)(((byte)(155)))), ((int)(((byte)(155)))));
            this.readPermissionBox.Checked = false;
            this.readPermissionBox.CheckSignColor = System.Drawing.Color.FromArgb(((int)(((byte)(65)))), ((int)(((byte)(177)))), ((int)(((byte)(225)))));
            this.readPermissionBox.CheckState = MetroSet_UI.Enums.CheckState.Unchecked;
            this.readPermissionBox.Cursor = System.Windows.Forms.Cursors.Hand;
            this.readPermissionBox.DisabledBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.readPermissionBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.readPermissionBox.IsDerivedStyle = true;
            this.readPermissionBox.Location = new System.Drawing.Point(47, 15);
            this.readPermissionBox.Name = "readPermissionBox";
            this.readPermissionBox.SignStyle = MetroSet_UI.Enums.SignStyle.Sign;
            this.readPermissionBox.Size = new System.Drawing.Size(135, 16);
            this.readPermissionBox.Style = MetroSet_UI.Enums.Style.Dark;
            this.readPermissionBox.StyleManager = null;
            this.readPermissionBox.TabIndex = 3;
            this.readPermissionBox.Text = "Read Permission";
            this.readPermissionBox.ThemeAuthor = "Narwin";
            this.readPermissionBox.ThemeName = "MetroDark";
            this.readPermissionBox.Visible = false;
            this.readPermissionBox.CheckedChanged += new MetroSet_UI.Controls.MetroSetCheckBox.CheckedChangedEventHandler(this.ReadPermissionBox_CheckedChanged);
            // 
            // writePermissionBox
            // 
            this.writePermissionBox.BackColor = System.Drawing.Color.Transparent;
            this.writePermissionBox.BackgroundColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
            this.writePermissionBox.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(155)))), ((int)(((byte)(155)))), ((int)(((byte)(155)))));
            this.writePermissionBox.Checked = false;
            this.writePermissionBox.CheckSignColor = System.Drawing.Color.FromArgb(((int)(((byte)(65)))), ((int)(((byte)(177)))), ((int)(((byte)(225)))));
            this.writePermissionBox.CheckState = MetroSet_UI.Enums.CheckState.Unchecked;
            this.writePermissionBox.Cursor = System.Windows.Forms.Cursors.Hand;
            this.writePermissionBox.DisabledBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(85)))), ((int)(((byte)(85)))), ((int)(((byte)(85)))));
            this.writePermissionBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.writePermissionBox.IsDerivedStyle = true;
            this.writePermissionBox.Location = new System.Drawing.Point(265, 15);
            this.writePermissionBox.Name = "writePermissionBox";
            this.writePermissionBox.SignStyle = MetroSet_UI.Enums.SignStyle.Sign;
            this.writePermissionBox.Size = new System.Drawing.Size(136, 16);
            this.writePermissionBox.Style = MetroSet_UI.Enums.Style.Dark;
            this.writePermissionBox.StyleManager = null;
            this.writePermissionBox.TabIndex = 4;
            this.writePermissionBox.Text = "Write Permission";
            this.writePermissionBox.ThemeAuthor = "Narwin";
            this.writePermissionBox.ThemeName = "MetroDark";
            this.writePermissionBox.Visible = false;
            this.writePermissionBox.CheckedChanged += new MetroSet_UI.Controls.MetroSetCheckBox.CheckedChangedEventHandler(this.WritePermissionBox_CheckedChanged);
            // 
            // timer1
            // 
            this.timer1.Enabled = true;
            this.timer1.Tick += new System.EventHandler(this.Timer1_Tick);
            // 
            // GlobalPathForm
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.BackgroundColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
            this.ClientSize = new System.Drawing.Size(454, 450);
            this.Controls.Add(this.writePermissionBox);
            this.Controls.Add(this.readPermissionBox);
            this.Controls.Add(this.globalPathBox);
            this.Controls.Add(this.removeGlobalPath);
            this.Controls.Add(this.addNewGlobalPath);
            this.Name = "GlobalPathForm";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.SmallLineColor1 = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
            this.SmallLineColor2 = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
            this.Style = MetroSet_UI.Enums.Style.Custom;
            this.TextColor = System.Drawing.Color.White;
            this.ThemeName = "MetroDark";
            this.TopMost = true;
            this.Load += new System.EventHandler(this.GlobalPathForm_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private MetroSet_UI.Controls.MetroSetButton addNewGlobalPath;
        private MetroSet_UI.Controls.MetroSetButton removeGlobalPath;
        private MetroSet_UI.Controls.MetroSetListBox globalPathBox;
        private MetroSet_UI.Controls.MetroSetCheckBox readPermissionBox;
        private MetroSet_UI.Controls.MetroSetCheckBox writePermissionBox;
        private System.Windows.Forms.Timer timer1;
    }
}