using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Threading;
using System.Diagnostics;

namespace AGV_Form
{
    public partial class DashboardForm : Form
    {

        public static string PathSend;
        public static List<string> textComStatus = new List<string>();
        public static List<Color> colorComStatus = new List<Color>();
        public DashboardForm()
        {
            InitializeComponent();
            LoadLabel();
            Communication.SerialPort.DataReceived += new SerialDataReceivedEventHandler(this.SerialPort_ReceiveData);
        }

        private void SerialPort_ReceiveData(object sender, SerialDataReceivedEventArgs e)
        {
            Communication.GetDataRecieve();
        }

        public static int delay = 0;
        private void DashboardForm_Load(object sender, EventArgs e)
        {
           
            switch (Display.Mode)
            {
                case "Real Time":
                    rdbtnRealTime.Checked = true;
                    rdbtnSimulation.Checked = false;
                    foreach (AGV agv in AGV.ListAGV)
                    {
                        pnFloor.Controls.Add(Display.LabelPalletInAGV[agv.ID]);
                        Display.LabelPalletInAGV[agv.ID].BringToFront();
                    }
                    break;
                case "Simulation":
                    rdbtnRealTime.Checked = false;
                    rdbtnSimulation.Checked = true;
                    foreach (AGV agv in AGV.SimListAGV)
                    {
                        pnFloor.Controls.Add(Display.SimLabelPalletInAGV[agv.ID]);
                        Display.SimLabelPalletInAGV[agv.ID].BringToFront();
                    }
                    break;
            }

            

            //AGV.ListPathOfAGV[2] = Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance, 54, 30);
        }
        private void btnCOMSetting_Click(object sender, EventArgs e)
        {
            COMSetting form = new COMSetting();
            form.ShowDialog();
            if (Communication.SerialPort.IsOpen)
            {
                lbStatusCOM.Text = Communication.SerialPort.PortName + " Connected.";
            }
            else
            {
                lbStatusCOM.Text = "Please setting COM !";

            }
        }

        private void rdbtnSimulation_CheckedChanged(object sender, EventArgs e)
        {
            if (rdbtnSimulation.Checked)
            {
                lbModeStatus.Text = "Simulation Mode.";
                timerListview.Interval = 100;
                timerSimAGV.Interval = 100;
                Display.Mode = "Simulation";
                foreach (AGV agv in AGV.ListAGV)
                {
                    Label label = Display.LabelAGV[agv.ID];
                    pnFloor.Controls.Remove(label);
                    //label.Dispose();
                }
                    foreach (AGV agv in AGV.SimListAGV)
                {                   
                    pnFloor.Controls.Add(Display.SimLabelAGV[agv.ID]);
                }
                Pallet.SimListPallet = DBUtility.GetPalletInfoFromDB<List<Pallet>>("SimPalletInfoTable");

                // Display Pallet in Navigation Map
                foreach (Pallet pallet in Pallet.ListPallet)
                {
                    Display.DeleteLabelPallet(pallet);


                }

                foreach (Pallet pallet in Pallet.SimListPallet)
                {

                    Display.UpdateLabelPallet(pallet);
                    
                        
                }
            }
        }

        private void rdbtnRealTime_CheckedChanged(object sender, EventArgs e)
        {
            if (rdbtnRealTime.Checked)
            {
                lbModeStatus.Text = "Realtime Mode.";
                timerListview.Interval = 100;
                timerSimAGV.Interval = 250;
                Display.Mode = "Real Time";
                foreach (AGV agv in AGV.SimListAGV)
                {
                    //Label label = pnFloor.Controls.OfType<Label>().FirstOrDefault(lb => lb.Name == "AGV" + agv.ID.ToString());
                    Label label = Display.SimLabelAGV[agv.ID];
                    pnFloor.Controls.Remove(label);
                    //label.Dispose();
                }
                foreach (AGV agv in AGV.ListAGV)
                {
                    pnFloor.Controls.Add(Display.LabelAGV[agv.ID]);
                }

                Pallet.ListPallet = DBUtility.GetPalletInfoFromDB<List<Pallet>>("PalletInfoTable");
                foreach (Pallet pallet in Pallet.SimListPallet)
                {

                    Display.DeleteLabelPallet(pallet);


                }
                foreach (Pallet pallet in Pallet.ListPallet)
                {
                    Display.UpdateLabelPallet(pallet);

                }

            }
        }

        private void timerListview_Tick(object sender, EventArgs e)
        {
            if (textComStatus.Count != 0)
            {
                rtxtbComStatus.SelectionColor = colorComStatus[0];
                rtxtbComStatus.SelectedText = textComStatus[0];

                textComStatus.RemoveAt(0);
                colorComStatus.RemoveAt(0);
            }

            switch (Display.Mode)
            {
                case "Real Time":
                    // Update data in listView AGVs
                    Display.UpdateListViewAGVs(listViewAGVs, AGV.ListAGV);
                    if(AGV.ListAGV.Count >0)
                    {
                        Display.UpdateListViewTasks(listViewTask, AGV.ListAGV[0].Tasks);
                        Display.UpdatePositionAGV(AGV.ListAGV[0].ID, Display.LabelAGV[AGV.ListAGV[0].ID]);
                    }
                    
                    // Do something here
                    // Update serial port status
                  
                    break;
                case "Simulation":
                    // Update data in listView AGVs
                    Display.UpdateListViewAGVs(listViewAGVs, AGV.SimListAGV);
                    Display.UpdateListViewTasks(listViewTask, Task.SimListTask);
                    // Update location of AGV icon (label)
                    //Display.UpdateListViewTasks(listViewTasks, Task.SimListTask);


                    break;
            }
           ///if(AGV.SimListAGV.Count >=2)
          /// Collision.DetectColission(AGV.SimListAGV[0], AGV.SimListAGV[1]);

            //label21.Text = Collision.CollisionType.ToString();

        }
        private void pnFloor_Paint(object sender, PaintEventArgs e)
        {
            // Draw lines with Points is got by Display.AddPath function
            Graphics graphic = pnFloor.CreateGraphics();
            Pen pen = new Pen(Color.Blue, 4);
            graphic.DrawLines(pen, Display.Points);
            graphic.Dispose();
        }



        

        private void AddAGV_Click(object sender, EventArgs e)
        {
            // Clone a list for ListAGV (amazing way to clone a reference type)
            List<AGV> oldSimListAGV = new List<AGV>();
          
            using (AddRemoveAGVForm AddRemoveForm = new AddRemoveAGVForm())
            {
                AddRemoveForm.ShowDialog();
            }
            //foreach (AGV agv in oldSimListAGV)
            //{
             //   Display.RemoveLabelAGV(pnFloor, agv.ID);
           // }
            switch (Display.Mode)
            {
                case "Real Time":
                    foreach (AGV agv in AGV.ListAGV)
                    {
                        pnFloor.Controls.Add(Display.LabelAGV[agv.ID]);
                        Display.LabelAGV[agv.ID].BringToFront();
                        pnFloor.Controls.Add(Display.LabelPalletInAGV[agv.ID]);
                        Display.LabelPalletInAGV[agv.ID].BringToFront();
                    }
                    break;
                case "Simulation":
                    foreach (AGV agv in AGV.SimListAGV)
                    {
                        
                        pnFloor.Controls.Add(Display.SimLabelAGV[agv.ID]);
                        Display.SimLabelAGV[agv.ID].BringToFront();
                        pnFloor.Controls.Add(Display.SimLabelPalletInAGV[agv.ID]);
                        Display.SimLabelPalletInAGV[agv.ID].BringToFront();
                    }
                    break;
            }
            
        }

        private void timerSimAGV_Tick(object sender, EventArgs e)
        {
            //listViewTask.Items.Clear();
            switch (Display.Mode)
            {
                case "Real Time":
                    foreach (AGV agv in AGV.ListAGV)
                    {
                        Task.UpdatePathFromTaskOfAGV(agv);
                       

                    }
                    break;
                case "Simulation":
                    foreach (AGV agv in AGV.SimListAGV)
                    {
                        Task.SimUpdatePathFromTaskOfAGVs(agv);
                        Display.SimUpdatePositionAGV(agv.ID, agv.Velocity,pnFloor, Display.SimLabelAGV[agv.ID],Display.SimLabelPalletInAGV[agv.ID]);
                       
                        
                        if(agv.HavePallet)
                            Display.SimLabelPalletInAGV[agv.ID].Visible = true;
                        else
                            Display.SimLabelPalletInAGV[agv.ID].Visible = false;
                    }
                    break;
            }


           
            
        }

       

        

        

        private void button6_Click(object sender, EventArgs e)
        {
            AddTaskAGVForm AddTaskForm = new AddTaskAGVForm();                
            AddTaskForm.Show();            
        }

        private void cntxMenuStrip_Opening(object sender, CancelEventArgs e)
        {
            List<string> strItems = new List<string>();
            switch (Display.Mode)
            {
                case "Real Time":
                    AGV.ListAGV.ForEach(a => strItems.Add("AGV#" + a.ID.ToString()));
                    break;
                case "Simulation":
                    AGV.SimListAGV.ForEach(a => strItems.Add("AGV#" + a.ID.ToString()));
                    break;
            }
            showPathToolStripMenuItem.DropDownItems.Clear();
            foreach (string items in strItems)
            {
                ToolStripMenuItem item = new ToolStripMenuItem(items, imgList.Images[0]);
                showPathToolStripMenuItem.DropDownItems.Add(item);
                item.Click += new EventHandler(AGVDrawPath_Click);
            }
            runCurrentAGVToolStripMenuItem.DropDownItems.Clear();
            foreach (string items in strItems)
            {
                ToolStripMenuItem item = new ToolStripMenuItem(items, imgList.Images[0]);
                runCurrentAGVToolStripMenuItem.DropDownItems.Add(item);
                item.Click += new EventHandler(RunAGV_Click);
            }
            stopCurrentAGVToolStripMenuItem.DropDownItems.Clear();
            foreach (string items in strItems)
            {
                ToolStripMenuItem item = new ToolStripMenuItem(items, imgList.Images[0]);
                stopCurrentAGVToolStripMenuItem.DropDownItems.Add(item);
                item.Click += new EventHandler(StopAGV_Click);
            }

        }
        private void StopAGV_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem item = sender as ToolStripMenuItem;
            string[] arrItem = item.Text.Split(new char[] { '#' }, StringSplitOptions.RemoveEmptyEntries);
            int agvID = Convert.ToInt16(arrItem[1]);

            switch (Display.Mode)
            {
                case "Real Time":

                    break;
                case "Simulation":
                    AGV agv = AGV.SimListAGV.Find(a => a.ID == agvID);
                    agv.Stop = true;
                    agv.Status = "Stopping";
                    Display.SimLabelAGV[agv.ID].BackColor = Color.Red;
                    break;
            }
        }
        private void RunAGV_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem item = sender as ToolStripMenuItem;
            string[] arrItem = item.Text.Split(new char[] { '#' }, StringSplitOptions.RemoveEmptyEntries);
            int agvID = Convert.ToInt16(arrItem[1]);
           
            switch (Display.Mode)
            {
                case "Real Time":
                   
                    break;
                case "Simulation":
                    AGV agv = AGV.SimListAGV.Find(a=>a.ID == agvID);
                    agv.Stop = false;
                    agv.Status = "Running";
                    Display.SimLabelAGV[agv.ID].BackColor = Color.CornflowerBlue;
                    break;
            }
        }

        private void AGVDrawPath_Click(object sender, EventArgs e)
        {
            ToolStripMenuItem item = sender as ToolStripMenuItem;
            string[] arrItem = item.Text.Split(new char[] { '#' }, StringSplitOptions.RemoveEmptyEntries);
            int agvID = Convert.ToInt16(arrItem[1]);
            
            switch (Display.Mode)
            {
                case "Real Time":
                    int i = AGV.ListAGV.FindIndex(a => a.ID == agvID);
                    if (AGV.ListAGV[i].Path.Count == 0) return;
                    //Display.AddPath(pnFloor, AGV.ListAGV[i].Path[0], Color.Blue, 4);
                    break;
                case "Simulation":
                    int j = AGV.SimListAGV.FindIndex(a => a.ID == agvID);
                    if (AGV.SimListAGV[j].Path.Count == 0) return;
                    
                    Display.AddPath(pnFloor, AGV.SimListAGV[j].Path[0],Node.ListNode, Color.Blue, 4);
                    break;
            }
        }
        
        
        private void hidePathToolStripMenuItem_Click(object sender, EventArgs e)
        {
            
            Display.Points = new Point[] { new Point(), new Point() };
            
            pnFloor.Refresh();
        }

        

       

        private void btnPause_Click(object sender, EventArgs e)
        {
            btnPause.BackColor = Color.DodgerBlue;
            btnRun.BackColor = Color.LightSteelBlue;
            switch (Display.Mode)
            {
                case "Real Time":
                    timerSimAGV.Enabled = false;
                    break;
                case "Simulation":
                    timerSimAGV.Enabled = false;
                    break;
            }
           
        }

        public static System.Windows.Forms.Timer timerToSendLineAgain;
        public static System.Windows.Forms.Timer timerToSendPathAgain;
        public static System.Windows.Forms.Timer timerToSendSpeedAgain;
        public static System.Windows.Forms.Timer timerToSendPath2Again;
        private void btnRun_Click(object sender, EventArgs e)
        {
            btnRun.BackColor = Color.DodgerBlue;
            btnPause.BackColor = Color.LightSteelBlue;
            switch (Display.Mode)
            {
                case "Real Time":
                  //  timerToSendPathAgain = new System.Windows.Forms.Timer();
                   // timerToSendPathAgain.Tick += new EventHandler(timerToSendPathAgain_Tick);
                  //  timerToSendPathAgain.Interval = 100;
                  /*  timerToSendPath2Again = new System.Windows.Forms.Timer();
                    timerToSendPath2Again.Tick += new EventHandler(timerToSendPath2Again_Tick);
                    timerToSendPath2Again.Interval = 400;   */

                    timerSimAGV.Enabled = true;

                    break;
                case "Simulation":
                    timerSimAGV.Enabled = true;
                    break;
            }
        }
        public static void timerToSendPathAgain_Tick(object sender, EventArgs e)
        {
            timerToSendPathAgain.Stop();
            if (AGV.ListAGV[0].PathCopmpleted == 0)
            {
                Task currentTask = AGV.ListAGV[0].Tasks[0];
                currentTask.Status = "Waiting";
               // AGV.ListAGV[0].PathCopmpleted = 0;
            }
        

           
        }
 /*       public static void timerToSendPath2Again_Tick(object sender, EventArgs e)
        {
            timerToSendPath2Again.Stop();
            AGV.ListAGV[0].PathCopmpleted = 1;
            
        }   */



        private void btnOrder1_Click(object sender, EventArgs e)
        {
            OrderForm orderForm = new OrderForm(51);

            orderForm.Show();
        }

        private void btnOrder2_Click(object sender, EventArgs e)
        {
            OrderForm orderForm = new OrderForm(52);

            orderForm.Show();
        }




        private void btnStore1_Click(object sender, EventArgs e)
        {
            StoreForm storeForm = new StoreForm(53);
            storeForm.Show();
        }

        private void btnStore2_Click(object sender, EventArgs e)
        {
            StoreForm storeForm = new StoreForm(54);
            storeForm.Show();
        }
        private void LoadLabel()
        {
            // Load Pallet Label Block A
            Display.ASlotLabel[0, 0] = lbA11;
            Display.ASlotLabel[0, 1] = lbA12;
            Display.ASlotLabel[0, 2] = lbA13;

            Display.ASlotLabel[1, 0] = lbA21;
            Display.ASlotLabel[1, 1] = lbA22;
            Display.ASlotLabel[1, 2] = lbA23;

            Display.ASlotLabel[2, 0] = lbA31;
            Display.ASlotLabel[2, 1] = lbA32;
            Display.ASlotLabel[2, 2] = lbA33;

            Display.ASlotLabel[3, 0] = lbA41;
            Display.ASlotLabel[3, 1] = lbA42;
            Display.ASlotLabel[3, 2] = lbA43;

            Display.ASlotLabel[4, 0] = lbA51;
            Display.ASlotLabel[4, 1] = lbA52;
            Display.ASlotLabel[4, 2] = lbA53;

            Display.ASlotLabel[5, 0] = lbA61;
            Display.ASlotLabel[5, 1] = lbA62;
            Display.ASlotLabel[5, 2] = lbA63;
            // Load Pallet Label Block B
            Display.BSlotLabel[0, 0] = lbB11;
            Display.BSlotLabel[0, 1] = lbB12;
            Display.BSlotLabel[0, 2] = lbB13;

            Display.BSlotLabel[1, 0] = lbB21;
            Display.BSlotLabel[1, 1] = lbB22;
            Display.BSlotLabel[1, 2] = lbB23;

            Display.BSlotLabel[2, 0] = lbB31;
            Display.BSlotLabel[2, 1] = lbB32;
            Display.BSlotLabel[2, 2] = lbB33;

            Display.BSlotLabel[3, 0] = lbB41;
            Display.BSlotLabel[3, 1] = lbB42;
            Display.BSlotLabel[3, 2] = lbB43;

            Display.BSlotLabel[4, 0] = lbB51;
            Display.BSlotLabel[4, 1] = lbB52;
            Display.BSlotLabel[4, 2] = lbB53;

            Display.BSlotLabel[5, 0] = lbB61;
            Display.BSlotLabel[5, 1] = lbB62;
            Display.BSlotLabel[5, 2] = lbB63;
            // Load Pallet Label Block C
            Display.CSlotLabel[0, 0] = lbC11;
            Display.CSlotLabel[0, 1] = lbC12;
            Display.CSlotLabel[0, 2] = lbC13;

            Display.CSlotLabel[1, 0] = lbC21;
            Display.CSlotLabel[1, 1] = lbC22;
            Display.CSlotLabel[1, 2] = lbC23;

            Display.CSlotLabel[2, 0] = lbC31;
            Display.CSlotLabel[2, 1] = lbC32;
            Display.CSlotLabel[2, 2] = lbC33;

            Display.CSlotLabel[3, 0] = lbC41;
            Display.CSlotLabel[3, 1] = lbC42;
            Display.CSlotLabel[3, 2] = lbC43;

            Display.CSlotLabel[4, 0] = lbC51;
            Display.CSlotLabel[4, 1] = lbC52;
            Display.CSlotLabel[4, 2] = lbC53;

            Display.CSlotLabel[5, 0] = lbC61;
            Display.CSlotLabel[5, 1] = lbC62;
            Display.CSlotLabel[5, 2] = lbC63;
            // Load Pallet Label Block D
            Display.DSlotLabel[0, 0] = lbD11;
            Display.DSlotLabel[0, 1] = lbD12;
            Display.DSlotLabel[0, 2] = lbD13;

            Display.DSlotLabel[1, 0] = lbD21;
            Display.DSlotLabel[1, 1] = lbD22;
            Display.DSlotLabel[1, 2] = lbD23;

            Display.DSlotLabel[2, 0] = lbD31;
            Display.DSlotLabel[2, 1] = lbD32;
            Display.DSlotLabel[2, 2] = lbD33;

            Display.DSlotLabel[3, 0] = lbD41;
            Display.DSlotLabel[3, 1] = lbD42;
            Display.DSlotLabel[3, 2] = lbD43;

            Display.DSlotLabel[4, 0] = lbD51;
            Display.DSlotLabel[4, 1] = lbD52;
            Display.DSlotLabel[4, 2] = lbD53;

            Display.DSlotLabel[5, 0] = lbD61;
            Display.DSlotLabel[5, 1] = lbD62;
            Display.DSlotLabel[5, 2] = lbD63;


        }

       

        private void button4_Click(object sender, EventArgs e)
        {
            AGV.SimListAGV[1].Stop = true;
          
        }

        

        private void button2_Click(object sender, EventArgs e)
        {
            int agvID = 1;
            AGV agv = AGV.SimListAGV.Find(p => p.ID == agvID);
            if(agv.Status == "Stop")
            {
                
                List<int> path = Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance, agv.CurrentNode, 55);
                AGV.SimFullPathOfAGV[agvID] = Navigation.GetNavigationFrame(path, Node.MatrixNodeOrient);
                AGV.SimListAGV[0].Path.Add(path);
            }
            //agv.PathCopmpleted = 4;
        }

        private void btnConfigPIDLine_Click(object sender, EventArgs e)
        {
            timerToSendLineAgain = new System.Windows.Forms.Timer();
            timerToSendLineAgain.Tick += new EventHandler(timerToSendLineAgain_Tick);
            timerToSendLineAgain.Interval = 400;
            sendLineFrame();
         //   Thread thrdSendLineFrame = new Thread(sendLineFrame);
         // thrdSendLineFrame.IsBackground = true;
         // thrdSendLineFrame.Start();
        }
        public void timerToSendLineAgain_Tick(object sender, EventArgs e)
        {
            timerToSendLineAgain.Stop();
            sendLineFrame();
            
        }  
      
       
        public static void sendLineFrame()
        {
            Communication.SetLinePosPacket sendFrame = new Communication.SetLinePosPacket();

            sendFrame.Header = new byte[2] { 0xAA, 0xFF };
            sendFrame.FunctionCode = 0xAD;
            sendFrame.AGVID = 0x01;
            sendFrame.Kp = Convert.ToSingle("0.5");
            sendFrame.Ki = Convert.ToSingle("0.0005");
            sendFrame.Kd = Convert.ToSingle("0.05");
            sendFrame.CheckSum = new byte[2];

            // calculate check sum
            CRC16_Calculator(sendFrame.ToArrayCRC(), sendFrame.CheckSum);

            //sendFrame.CheckSum = crc;
            sendFrame.EndOfFrame = new byte[2] { 0x0A, 0x0D };
            if (!Communication.SerialPort.IsOpen) return;
            try { Communication.SerialPort.Write(sendFrame.ToArray(), 0, sendFrame.ToArray().Length); }
            catch { };
            if (timerToSendLineAgain.Enabled == false)
                timerToSendLineAgain.Start();
            
        }


        public static void CRC16_Calculator(byte[] messageArray, byte[] CRC)
        {
            ushort CRCFull = 0xFFFF;
            char CRCLSB;

            for (int i = 0; i < (messageArray.Length); i++)
            {
                CRCFull = (ushort)(CRCFull ^ messageArray[i]);

                for (int j = 0; j < 8; j++)
                {
                    CRCLSB = (char)(CRCFull & 0x0001);
                    CRCFull = (ushort)((CRCFull >> 1) & 0x7FFF);

                    if (CRCLSB == 1)
                        CRCFull = (ushort)(CRCFull ^ 0xA001);
                }
            }
            CRC[0] = (byte)(CRCFull & 0xFF);
            CRC[1] = (byte)((CRCFull >> 8) & 0xFF);
        }

        private void btnConfigPIDSpeed_Click(object sender, EventArgs e)
        {
            timerToSendSpeedAgain = new System.Windows.Forms.Timer();
            timerToSendSpeedAgain.Tick += new EventHandler(timerToSendSpeedAgain_Tick);
            timerToSendSpeedAgain.Interval = 400;
            sendSpeedFrame();
          // Thread thrdSendSpeedFrame = new Thread(sendSpeedFrame);
          // thrdSendSpeedFrame.IsBackground = true;
          // thrdSendSpeedFrame.Start();
        }
        public void timerToSendSpeedAgain_Tick(object sender, EventArgs e)
        {
            timerToSendSpeedAgain.Stop();
            sendSpeedFrame();

        }

        public static void sendSpeedFrame()
        {
            Communication.SetSpeedPacket sendFrame = new Communication.SetSpeedPacket();

            sendFrame.Header = new byte[2] { 0xAA, 0xFF };
            sendFrame.CheckSum = new byte[2];
            sendFrame.FunctionCode = 0xAC;
            sendFrame.AGVID = 0x01;
            sendFrame.Kp = Convert.ToSingle("2.0");
            sendFrame.Ki = Convert.ToSingle("2.0");
            sendFrame.Kd = Convert.ToSingle("0.015");
            sendFrame.Velocity = Convert.ToSingle("15.0");
            sendFrame.CheckSum = new byte[2];
            // calculate check sum
            CRC16_Calculator(sendFrame.ToArrayCRC(), sendFrame.CheckSum);
            //   sendFrame.CheckSum = crc;           
            sendFrame.EndOfFrame = new byte[2] { 0x0A, 0x0D };
            if (!Communication.SerialPort.IsOpen) return;
            try { Communication.SerialPort.Write(sendFrame.ToArray(), 0, sendFrame.ToArray().Length); }
            catch { };
            if (timerToSendSpeedAgain.Enabled == false)
                timerToSendSpeedAgain.Start();

           
        }

        private void button3_Click_1(object sender, EventArgs e)
        {
            label22.Text = PathSend;
             //
            
            //label22.Text= Collision.ReturnToNode(16).ToString(); 
           
        }

        private void button1_Click(object sender, EventArgs e)
        {
           // label19.Text= Collision. (45).ToString();
            //AGV.ListAGV[0].CurrentOrient = 'N';
            //AGV.ListAGV[0].CurrentNode = 11;
            //AGV.ListAGV[0].Tasks.Clear();
            //AGV.ListAGV[0].Path.Clear();
        }
    }
}
