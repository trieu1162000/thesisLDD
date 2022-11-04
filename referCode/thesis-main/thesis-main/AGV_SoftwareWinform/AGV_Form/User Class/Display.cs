using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AGV_Form
{
    class Display
    {
        public static string Mode;
        public static Color LineColor;
        public static float LineWidth;
        public static Point[] Points = new Point[2];
        public static Label[] LabelAGV = new Label[AGV.MaxNumOfAGVs]; // LabelAGV[i] for AGV ID = i
        public static Label[] SimLabelAGV = new Label[AGV.MaxNumOfAGVs];
        public static Label[] LabelPalletInAGV = new Label[AGV.MaxNumOfAGVs];
        public static Label[] SimLabelPalletInAGV = new Label[AGV.MaxNumOfAGVs];
        public static float Scale = 585f / 235; // pixel/cm (2.480851f)
        public static Label[,] ASlotLabel = new Label[6, 3];
        public static Label[,] BSlotLabel = new Label[6, 3];
        public static Label[,] CSlotLabel = new Label[6, 3];
        public static Label[,] DSlotLabel = new Label[6, 3];
        public static int wait =1;

        public static void AddPath(Panel panel, List<int> path, List<Node> Nodes, Color color, float width)
        {
            LineColor = color;
            LineWidth = width;
            List<Point> listPoint = new List<Point>();
            for (int i = 0; i < path.Count; i++)
            {
                listPoint.Add(new Point(Nodes[path[i]].X, Nodes[path[i]].Y));
            }
            Points = listPoint.ToArray();
            panel.Refresh();
        }
        public static void UpdateListViewAGVs(ListView listView, List<AGV> listData)
        {
            listView.Items.Clear();


            foreach (AGV agv in listData)
            {
                listView.Items.Add("AGV#" + agv.ID, 0);
                listView.Items[listView.Items.Count - 1].SubItems.Add(agv.Status);
                listView.Items[listView.Items.Count - 1].SubItems.Add(agv.CurrentNode.ToString());
                listView.Items[listView.Items.Count - 1].SubItems.Add(agv.CurrentOrient.ToString());
                listView.Items[listView.Items.Count - 1].SubItems.Add((Math.Round(agv.DistanceToCurrentNode, 2)).ToString() + " cm");
                listView.Items[listView.Items.Count - 1].SubItems.Add(agv.Velocity.ToString().Trim() + " cm/s");
                
               switch(agv.Status)
                {
                    case "Running":
                        listView.Items[listData.IndexOf(agv)].BackColor = Color.PaleGreen;
                        break;
                    case "Stop":
                       listView.Items[listData.IndexOf(agv)].BackColor = Color.White;
                        break;
                    case "Stopping":
                        listView.Items[listData.IndexOf(agv)].BackColor = Color.Red;
                        break;
                    case "Pausing":
                        listView.Items[listData.IndexOf(agv)].BackColor = Color.Yellow;
                        break;
                    case "Waitting":
                        listView.Items[listData.IndexOf(agv)].BackColor = Color.LightBlue;
                        break;
                    case "Avoid":
                        listView.Items[listData.IndexOf(agv)].BackColor = Color.LightCyan;
                        break;
                    case "Other Way":
                        listView.Items[listData.IndexOf(agv)].BackColor = Color.Orange;
                        break;
                }    
                                
            }


        }
        public static void UpdatePositionAGV(int agvID, Label lbagv)
        {
            var index = AGV.ListAGV.FindIndex(a => a.ID == agvID);
            AGV agv = AGV.ListAGV[index];
            Point oldAGVPosition = Display.LabelAGV[agvID].Location;
            Point newAGVPosition = new Point();
            int pixelDistance = (int)Math.Round(agv.DistanceToCurrentNode * Scale);
            List<Node> Nodes = DBUtility.GetDataFromDB<List<Node>>("NodeInfoTable");
            int x, y;
            try
            {
                x = Nodes[agv.CurrentNode].X - (int)(50 / 2);
                y = Nodes[agv.CurrentNode].Y - (int)(50 / 2);
            }
            catch
            {
                x = oldAGVPosition.X;
                y = oldAGVPosition.Y;
            }
            
            switch (agv.CurrentOrient)
            {
                case 'E':
                    newAGVPosition = new Point(x + pixelDistance, y);

                    break;
                case 'W':
                    newAGVPosition = new Point(x - pixelDistance, y);

                    break;
                case 'S':
                    newAGVPosition = new Point(x, y + pixelDistance);


                    break;
                case 'N':
                    newAGVPosition = new Point(x, y - pixelDistance);

                    break;
                default:
                    newAGVPosition = oldAGVPosition;

                    break;
            }
            lbagv.Location = newAGVPosition;
        }
        public static void SimUpdatePositionAGV(int agvID, float speed, Panel pnFloor,Label lbagv, Label lbpallet)
        {
            //int step = (int)speed * 2 / 10;              //1pixel = 0.5cm =>> 20cm/s=40pixel/s
            var index = AGV.SimListAGV.FindIndex(a => a.ID == agvID);
            AGV agv = AGV.SimListAGV[index];
            //List<char> fullpath = AGV.FullPathOfAGV[agvID].ToList();

            string[] frameArr = AGV.SimFullPathOfAGV[agvID].Split(new char[] { '-' }, StringSplitOptions.RemoveEmptyEntries);
            
            Point oldAGVPosition = Display.SimLabelAGV[agvID].Location;
            Point newAGVPosition = new Point();
            Point oldPalletPosition = Display.SimLabelPalletInAGV[agvID].Location;
            Point newPalletPosition = new Point();
            Size oldPalletSize = Display.SimLabelPalletInAGV[agvID].Size;
            Size newPalletSize = new Size();

            int indexNode = Array.FindIndex(frameArr, a => a == agv.CurrentNode.ToString());
            if (agv.Stop)
            {
                return;
            } 
            
            if (agv.IsColision) return;
            if (frameArr[indexNode + 1] == "G" || frameArr[indexNode + 1] == null)
            {
                agv.PathCopmpleted++;
                if (agv.PathCopmpleted == 1 || agv.PathCopmpleted == 0)

                {
                    Display.Points = new Point[] { new Point(), new Point() };
                    pnFloor.Refresh();
                }
            }
            else
            {
                int currentNode = agv.CurrentNode;
                int nextNode = Convert.ToInt32(frameArr[indexNode + 2]);
                if (agv.CurrentOrient != Display.UpdateOrient(frameArr[indexNode + 1]))
                {
                    agv.DistanceToCurrentNode -= speed / 5.2f;
                    if (agv.DistanceToCurrentNode <= 0)
                    {
                        agv.CurrentOrient = Display.UpdateOrient(frameArr[indexNode + 1]);
                        agv.DistanceToCurrentNode = 0;
                    }

                }
                else
                {

                    agv.DistanceToCurrentNode += speed / 5.2f;
                    if (agv.DistanceToCurrentNode * 2 >= Node.MatrixNodeDistance[currentNode, nextNode])
                    {
                        
                        agv.DistanceToCurrentNode = 0;
                        agv.CurrentNode = nextNode;
                        
                        if (frameArr[indexNode + 3] != "G")
                            agv.CurrentOrient = Display.UpdateOrient(frameArr[indexNode + 3]);
                    }                                                                                  
                }
            }

            int pixelDistance = (int)Math.Round(agv.DistanceToCurrentNode * 2);
            List<Node> Nodes = DBUtility.GetDataFromDB<List<Node>>("NodeInfoTable");
            int x = Nodes[agv.CurrentNode].X - 50 / 2;
            int y = Nodes[agv.CurrentNode].Y - 50 / 2;
            switch (agv.CurrentOrient)
            {
                case 'E':
                    newAGVPosition = new Point(x + pixelDistance, y);
                    newPalletSize = new Size(15, 50);
                    newPalletPosition = new Point(x + pixelDistance - 15, y);
                    break;
                case 'W':
                    newAGVPosition = new Point(x - pixelDistance, y);
                    newPalletSize = new Size(15, 50);
                    newPalletPosition = new Point(x - pixelDistance + 50, y);
                    break;
                case 'S':
                    newAGVPosition = new Point(x, y + pixelDistance);
                    newPalletSize = new Size(50, 15);
                    newPalletPosition = new Point(x, y + pixelDistance - 15);

                    break;
                case 'N':
                    newAGVPosition = new Point(x, y - pixelDistance);
                    newPalletSize = new Size(50, 15);
                    newPalletPosition = new Point(x, y - pixelDistance + 50);
                    break;
                default: 
                    newAGVPosition = oldAGVPosition;
                    newPalletPosition = oldPalletPosition;
                    newPalletSize = oldPalletSize;
                    break;
            }

            lbpallet.Size = newPalletSize;
            lbpallet.Location = newPalletPosition;
            //lbpallet.Visible = true;
            lbagv.Location= newAGVPosition;

        }
       


        public static char UpdateOrient(string direction)
        {
            char orient = new char();
            
            switch (direction)
            {
                case "E":
                
                    orient = 'E';
                    break;
                case "W":
             
                    orient = 'W';
                    break;
                case "S":
                
                    orient = 'S';
                    break;
                case "N":
               
                    orient = 'N';
                    break;
                
            }
            return orient;
        }

        public static void GoUp(int agvID)
        {
            int x = SimLabelAGV[agvID].Location.X;
            int y = SimLabelAGV[agvID].Location.Y;
            SimLabelAGV[agvID].Location = new Point(x, y+3);

        }
        public static void AddLabelAGV(string Mode,int agvID, int initExitNode, char initOrientation, float initDistanceToExitNode)
        {
            /// Create new label for AGV
            Label lbAGV = new Label();
            lbAGV.BackColor = Color.Silver;
            lbAGV.Font = new Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular,
                                                    System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            lbAGV.Size = new Size(50, 50);
            lbAGV.ForeColor = Color.Black;
            lbAGV.Text = "AGV#" + agvID.ToString();
            lbAGV.TextAlign = ContentAlignment.MiddleCenter;
            lbAGV.Name = "AGV" + agvID.ToString();
            /// Create new label for Pallet in AGV
            Label pallet = new Label();
            pallet.BackColor = Color.Goldenrod;
            pallet.Size = new Size(50,11);
            pallet.ForeColor = Color.Black;
            pallet.Font = new Font("Microsoft Sans Serif", 8.0F, System.Drawing.FontStyle.Regular,
                                                    System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            pallet.Text = "Pallet";
            pallet.TextAlign = ContentAlignment.MiddleCenter;
            //pallet.BorderStyle = BorderStyle.FixedSingle;
            pallet.Name = "PalletInAGV" + agvID.ToString();
            pallet.Visible = false;

            int initPixelDistance = (int)Math.Round(initDistanceToExitNode*2);
            // Init the Location of new AGV
            List<Node> Nodes = DBUtility.GetDataFromDB<List<Node>>("NodeInfoTable");
            int x = Nodes[initExitNode].X - lbAGV.Size.Width / 2;
            int y = Nodes[initExitNode].Y - lbAGV.Size.Height / 2;
            switch (initOrientation)
            {
                case 'E':
                    lbAGV.Location = new Point(x + initPixelDistance, y);
                    pallet.Size = new Size(15, 50);
                    pallet.Location = new Point(x- initPixelDistance-15,y);
                    break;
                case 'W':
                    lbAGV.Location = new Point(x - initPixelDistance, y);
                    pallet.Size = new Size(15, 50);
                    pallet.Location = new Point(x + initPixelDistance+50, y);
                    break;
                case 'N':
                    lbAGV.Location = new Point(x, y - initPixelDistance);
                    pallet.Size = new Size(50, 15);
                    pallet.Location = new Point(x, y + initPixelDistance + 50);
                   
                    break;
                case 'S':
                    lbAGV.Location = new Point(x, y + initPixelDistance);
                    pallet.Size = new Size(50, 15);
                    pallet.Location = new Point(x, y - initPixelDistance - 15);
                    break;
            }
            // Add to Array for use
            switch (Mode)
            {
                case "Real Time":
                    LabelAGV[agvID] = lbAGV;
                    LabelPalletInAGV[agvID] = pallet;
                    break;
                case "Simulation":
                    SimLabelAGV[agvID] = lbAGV;
                    SimLabelPalletInAGV[agvID] = pallet;
                    break;
            }
        }

        public static void RemoveLabelAGV(string Mode,int agvID)
        {
            //var label = panel.Controls.OfType<Label>().FirstOrDefault(lb => lb.Name == "AGV" + agvID.ToString());
            
                //panel.Controls.Remove(label);
                //label.Dispose();

                switch (Mode)
                {
                    case "Real Time":
                        Array.Clear(LabelAGV, agvID, 1);
                        break;
                    case "Simulation":
                        Array.Clear(SimLabelAGV, agvID, 1);
                        break;
                }
            
        }
        public static void UpdateComStatus(int ID, Color messageColor)
        {
            HomeScreenForm.colorComStatus.Add(messageColor);
            string timeNow = DateTime.Now.ToString(" h:mm:ss.fff tt");
            if(ID == 1)
            {
                HomeScreenForm.textComStatus.Add(timeNow + " ID: "+ ID.ToString() + "asdfasdfasdfasfasdfasdfasdfasdfasdfsdfasdfasdfasdfasdfsadfasdfasdf\n");
            }
            else if (ID == 2)
            {
                HomeScreenForm.textComStatus.Add(timeNow + " ID: " + ID.ToString() + "\n");
            }
        }
        public static void UpdateListViewTasks(ListView listView, List<Task> listData )
        {
            
            listView.Items.Clear();
            foreach (Task task in listData)
            {
                RackColumn PickRack = RackColumn.ListColumn.Find(c => c.AtNode == task.PickNode);
                RackColumn DropRack = RackColumn.ListColumn.Find(c => c.AtNode == task.DropNode);
                listView.Items.Add(task.Name, 1);
                listView.Items[listView.Items.Count - 1].SubItems.Add(task.Status);
                listView.Items[listView.Items.Count - 1].SubItems.Add(task.Type);
                listView.Items[listView.Items.Count - 1].SubItems.Add("AGV#" + task.AGVID.ToString());
                listView.Items[listView.Items.Count - 1].SubItems.Add(PickRack.Block.ToString() + PickRack.Number.ToString()+" - " + task.PickLevel.ToString());
                listView.Items[listView.Items.Count - 1].SubItems.Add(DropRack.Block.ToString() + DropRack.Number.ToString() + " - " + task.DropLevel.ToString());
                listView.Items[listView.Items.Count - 1].SubItems.Add(task.PalletCode);
                if (task.Status == "Doing")
                        listView.Items[listData.IndexOf(task)].BackColor = Color.PaleGreen;
                    else
                        listView.Items[listData.IndexOf(task)].BackColor = Color.White;
                
            }
        }

        public static void UpdateLabelPallet(Pallet pallet)
        {
            switch (pallet.AtBlock)
            {
                case "A":
                    Display.ASlotLabel[pallet.AtColumn - 1, pallet.AtLevel - 1].BackColor = Color.Goldenrod;
                    break;
                case "B":
                    Display.BSlotLabel[pallet.AtColumn - 1, pallet.AtLevel - 1].BackColor = Color.Goldenrod;
                    break;
                case "C":
                    Display.CSlotLabel[pallet.AtColumn - 1, pallet.AtLevel - 1].BackColor = Color.Goldenrod;
                    break;
                case "D":
                    Display.DSlotLabel[pallet.AtColumn - 1, pallet.AtLevel - 1].BackColor = Color.Goldenrod;
                    break;
            }
        }

        public static void DeleteLabelPallet(Pallet pallet)
        {
            switch (pallet.AtBlock)
            {
                case "A":
                    Display.ASlotLabel[pallet.AtColumn - 1, pallet.AtLevel - 1].BackColor = Color.Silver;
                    break;
                case "B":
                    Display.BSlotLabel[pallet.AtColumn - 1, pallet.AtLevel - 1].BackColor = Color.Silver;
                    break;
                case "C":
                    Display.CSlotLabel[pallet.AtColumn - 1, pallet.AtLevel - 1].BackColor = Color.Silver;
                    break;
                case "D":
                    Display.DSlotLabel[pallet.AtColumn - 1, pallet.AtLevel - 1].BackColor = Color.Silver;
                    break;
            }
        }
        public static void UpdateComStatus(string type, int agvID, string message, Color messageColor)
        {
            string timeNow = DateTime.Now.ToString(" h:mm:ss.fff tt");

            DashboardForm.colorComStatus.Add(messageColor);
            if (type == "send")
                DashboardForm.textComStatus.Add(timeNow + "\t-> to AGV#" + agvID.ToString() + ":\t" + message + "\n");
            else if (type == "receive")
                DashboardForm.textComStatus.Add(timeNow + "\t<- from AGV#" + agvID.ToString() + ":\t" + message + "\n");
            else if (type == "timeout")
                DashboardForm.textComStatus.Add(timeNow + "\t...  AGV#" + agvID.ToString() + "\ttimeout" + "\n");
            else if (type == "status")
                DashboardForm.textComStatus.Add(timeNow + "\t" + message + "\n");
        }
    }
}
