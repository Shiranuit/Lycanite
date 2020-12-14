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
using Lycanite;
using System.Threading;

namespace App {
    public partial class GlobalPathForm : MetroSetForm {

        private Dictionary<String, Dictionary<String, ELycanitePerm>> pathListAuthorize = new Dictionary<String, Dictionary<String, ELycanitePerm>>();
        private LycaniteBridge lycaniteBridge;
        private static String WindowsPath = "C:\\Windows";
        private LycaniteApplication lycanite;

        public GlobalPathForm(LycaniteApplication lycanite) {
            this.lycanite = lycanite;
            this.InitializeComponent();
        }

        private void GlobalPathForm_Load(Object sender, EventArgs e) {
            string path = DevicePathMapper.ToDevicePath(WindowsPath);
            Dictionary<String, ELycanitePerm> windowsPerm = new Dictionary<String, ELycanitePerm>() {
                {
                    "read", ELycanitePerm.LYCANITE_READ
                },
                {
                    "write", ELycanitePerm.LYCANITE_NONE
                },
                {
                    "delete", ELycanitePerm.LYCANITE_NONE
                }
            };
            this.AllowResize = false;

            this.globalPathBox.ForeColor = Color.White;

            if (path != null) {
                this.globalPathBox.AddItem(WindowsPath);
                this.pathListAuthorize.Add(WindowsPath, windowsPerm);
                this.lycaniteBridge.SetGlobalFilePermissions(path, ELycanitePerm.LYCANITE_READ);
            }
            this.Location = new Point(this.lycanite.Location.X + this.lycanite.Size.Width, this.lycanite.Location.Y);
        }

        public void SetBridge(LycaniteBridge bridge) {
            this.lycaniteBridge = bridge;
        }

        private void ReadPermissionBox_CheckedChanged(Object sender) {
            MetroSet_UI.Controls.MetroSetCheckBox checkbox = (MetroSet_UI.Controls.MetroSetCheckBox)sender;

            if (this.globalPathBox.SelectedText == null)
                return;
            String fileName = this.globalPathBox.SelectedText;
            Dictionary<String, ELycanitePerm> dict = this.pathListAuthorize[fileName];

            dict["read"] = checkbox.Checked ? ELycanitePerm.LYCANITE_READ : ELycanitePerm.LYCANITE_NONE;


            String convertedPath = DevicePathMapper.ToDevicePath(fileName);

            if (convertedPath == null)
                return;

            if (!this.lycaniteBridge.SetGlobalFilePermissions(convertedPath, dict["read"] | dict["write"] | dict["delete"]))
                MessageBox.Show("Error: Permission can't be assigned to the file {0}", Path.GetFileName(fileName));
        }

        private void WritePermissionBox_CheckedChanged(Object sender) {
            MetroSet_UI.Controls.MetroSetCheckBox checkbox = (MetroSet_UI.Controls.MetroSetCheckBox)sender;

            if (this.globalPathBox.SelectedText == null)
                return;
            String fileName = this.globalPathBox.SelectedText;
            Dictionary<String, ELycanitePerm> dict = this.pathListAuthorize[fileName];

            dict["write"] = checkbox.Checked ? ELycanitePerm.LYCANITE_WRITE : ELycanitePerm.LYCANITE_NONE;


            String convertedPath = DevicePathMapper.ToDevicePath(fileName);

            if (convertedPath == null)
                return;

            if (!this.lycaniteBridge.SetGlobalFilePermissions(convertedPath, dict["read"] | dict["write"] | dict["delete"]))
                MessageBox.Show("Error: Permission can't be assigned to the file {0}", Path.GetFileName(fileName));
        }

        private Dictionary<string, ELycanitePerm> NewPerms() {
            return new Dictionary<string, ELycanitePerm> {
                { "read", ELycanitePerm.LYCANITE_NONE },
                { "write", ELycanitePerm.LYCANITE_NONE },
                { "delete", ELycanitePerm.LYCANITE_NONE }
            };
        }

        private void AddNewGlobalPath_Click(Object sender, EventArgs e) {
            System.Windows.Forms.OpenFileDialog fileDialog = new System.Windows.Forms.OpenFileDialog();
            fileDialog.ValidateNames = false;
            fileDialog.Multiselect = true;
            fileDialog.CheckPathExists = true;
            fileDialog.CheckFileExists = false;
            fileDialog.FileName = "Default";

            if (fileDialog.ShowDialog() == DialogResult.OK) {
                if (File.Exists(fileDialog.FileName)) {
                    if (this.pathListAuthorize.ContainsKey(fileDialog.FileName)) {
                        return;
                    } else {
                        this.pathListAuthorize.Add(fileDialog.FileName, this.NewPerms());
                        this.globalPathBox.AddItem(fileDialog.FileName);
                    }
                } else if (Directory.Exists(Path.GetDirectoryName(fileDialog.FileName))) {
                    fileDialog.FileName = Path.GetDirectoryName(fileDialog.FileName);
                    if (this.pathListAuthorize.ContainsKey(fileDialog.FileName)) {
                        return;
                    } else {
                        this.globalPathBox.AddItem(fileDialog.FileName);
                        this.pathListAuthorize.Add(fileDialog.FileName, this.NewPerms());
                    }
                }
            }
        }

        private void RemoveGlobalPath_Click(Object sender, EventArgs e) {            
            if (this.globalPathBox.SelectedText == null)
                return;

            String fileName = this.globalPathBox.SelectedText;

            this.globalPathBox.RemoveItem(fileName);
            this.pathListAuthorize.Remove(fileName);

            String convertedPath = DevicePathMapper.ToDevicePath(fileName);

            if (convertedPath == null)
                return;

            if (!this.lycaniteBridge.SetGlobalFilePermissions(convertedPath, ELycanitePerm.LYCANITE_NONE))
                MessageBox.Show("Error: File {0} permissions could not be delete sucessfully", Path.GetFileName(fileName));
        }

        private void GlobalPathBox_SelectedValueChanged(Object sender) {
            if (this.globalPathBox.SelectedText == null) {
                this.readPermissionBox.Visible = false;
                this.writePermissionBox.Visible = false;
            } else {
                this.readPermissionBox.Visible = true;
                this.writePermissionBox.Visible = true;

                Dictionary<String, ELycanitePerm> fileinfo = this.pathListAuthorize[this.globalPathBox.SelectedText];

                this.readPermissionBox.Checked = fileinfo["read"] == ELycanitePerm.LYCANITE_READ;
                this.writePermissionBox.Checked = fileinfo["write"] == ELycanitePerm.LYCANITE_WRITE;
            }
        }

        private void GlobalPathBox_LocationChanged(Object sender, EventArgs e) {
            MetroSetForm GlobalPathForm = (MetroSetForm)sender;
            if (GlobalPathForm.Location != null && this.lycanite != null) {
                GlobalPathForm.Location = new Point(this.lycanite.Location.X + this.lycanite.Size.Width, this.lycanite.Location.Y);
            }
        }

        private void Timer1_Tick(Object sender, EventArgs e) {
            if (this.Location != null && this.lycanite != null) {
                this.Location = new Point(this.lycanite.Location.X + this.lycanite.Size.Width, this.lycanite.Location.Y);
            }
        }
    }
}
