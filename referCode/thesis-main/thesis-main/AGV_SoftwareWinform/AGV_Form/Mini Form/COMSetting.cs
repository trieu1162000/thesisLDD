using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;

namespace AGV_Form
{
    public partial class COMSetting : Form
    {
        public COMSetting()
        {
            InitializeComponent();
        }


        private void COMSetting_Load(object sender, EventArgs e)
        {

            string[] ports = SerialPort.GetPortNames();
            cbbCOMPort.Items.AddRange(ports);

            if (Communication.SerialPort.IsOpen)
            {
                btnConnect.Text = "Disconnect";
                btnConnect.ImageIndex = 1;
                btnConnect.ForeColor = Color.Brown;
                btnReScan.Enabled = false;
                cbbCOMPort.Text = Communication.SerialPort.PortName;
                cbbBaudRate.Text = Communication.SerialPort.BaudRate.ToString();
                cbbDataBits.Text = Communication.SerialPort.DataBits.ToString();
                cbbStopBits.Text = Communication.SerialPort.StopBits.ToString();
                cbbParity.Text = Communication.SerialPort.Parity.ToString();
                lbStatusCOM.Text = "Connected";
                lbStatusCOM.ForeColor = Color.Green;
                lbInfoCOM.Text = Communication.SerialPort.PortName + "-" + Communication.SerialPort.BaudRate + "-" +
                                 Communication.SerialPort.DataBits + "-" + Communication.SerialPort.StopBits + "-" +
                                 Communication.SerialPort.Parity;
            }
            else
            {
                btnConnect.Text = "Connect";
                btnConnect.ImageIndex = 0;
                btnConnect.ForeColor = Color.MediumBlue;
                btnReScan.Enabled = true;
                lbStatusCOM.Text = "Disconnected!";
                lbStatusCOM.ForeColor = Color.Red;
                lbInfoCOM.Text = "-----------------------------";
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (btnConnect.Text == "Connect")
            {
                btnConnect.Text = "Disconnect";
                btnConnect.ImageIndex = 1;
                btnConnect.ForeColor = Color.Brown;
                btnReScan.Enabled = false;
                try
                {
                    Communication.SerialPort.PortName = cbbCOMPort.Text;
                    Communication.SerialPort.BaudRate = Convert.ToInt32(cbbBaudRate.Text);
                    Communication.SerialPort.DataBits = Convert.ToInt32(cbbDataBits.Text);
                    Communication.SerialPort.StopBits = (StopBits)Enum.Parse(typeof(StopBits), cbbStopBits.Text);
                    Communication.SerialPort.Parity = (Parity)Enum.Parse(typeof(Parity), cbbParity.Text);
                    
                    Communication.SerialPort.Open();
                    //   btnConnClicked = true;
                    
                    MessageBox.Show(Communication.SerialPort.PortName + " Connection Successful !","Serial Connection", MessageBoxButtons.OK,MessageBoxIcon.Information);
                    lbStatusCOM.Text = "Connected";
                    lbStatusCOM.ForeColor = Color.Green;
                    lbInfoCOM.Text = Communication.SerialPort.PortName + "-" + Communication.SerialPort.BaudRate + "-" +
                                     Communication.SerialPort.DataBits + "-" + Communication.SerialPort.StopBits + "-" +
                                     Communication.SerialPort.Parity;
                    
                }
                catch (Exception error)
                {
                    MessageBox.Show(error.Message, "Connection Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    btnConnect.Text = "Connect";
                    btnConnect.ImageIndex = 0;
                    btnConnect.ForeColor = Color.MediumBlue;
                    btnReScan.Enabled = true;
                }
            }
            else
            {
                string mess = "This action will stop monitoring, controlling & detecting collision.\nMake sure AGVs have no task.\nDo you really want to Disconnect ?";
                DialogResult result = MessageBox.Show(mess, "Warning", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning);
                if (result == System.Windows.Forms.DialogResult.OK)
                {
                    btnConnect.Text = "Connect";
                    btnConnect.ImageIndex = 0;
                    btnConnect.ForeColor = Color.MediumBlue;
                    btnReScan.Enabled = true;

                    Communication.SerialPort.Close();
                    lbStatusCOM.Text = "Disconnected!";
                    lbStatusCOM.ForeColor = Color.Red;
                    lbInfoCOM.Text = "-----------------------------";
                    
                }
            }
        
        }

        private void btnReScan_Click(object sender, EventArgs e)
        {
            cbbCOMPort.Items.Clear();
            string[] ports = SerialPort.GetPortNames();
            cbbCOMPort.Items.AddRange(ports);
            
        }

        
    }

   
}
