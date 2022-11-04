using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AGV_Form
{
    class Task
    {
        public string Name { get; set; }
        public string Type { get; set; }
        public string PalletCode { get; set; }
        public int AGVID { get; set; }
        public int PickNode { get; set; }
        public int DropNode { get; set; }
        public int PickLevel { get; set; }
        public int DropLevel { get; set; }
        public string Status { get; set; }

        // Constructor of Task
        public Task(string name, string type, string palletCode,
                    int agvID, int pickNode, int dropNode, int pickLevel, int dropLevel, string status)
        {
            this.Name = name;
            this.Type = type;
            this.PalletCode = palletCode;
            this.AGVID = agvID;
            this.PickNode = pickNode;
            this.DropNode = dropNode;
            this.PickLevel = pickLevel;
            this.DropLevel = dropLevel;
            this.Status = status;
        }

        // Information list of real-time Task
        public static List<Task> ListTask = new List<Task>();

        // Information list of simulation Task
        public static List<Task> SimListTask = new List<Task>();
        
        public static List<Task> SimHistoryTask = new List<Task>();
        public static void UpdatePathFromTaskOfAGV(AGV agv)
        {
            if (agv.Tasks.Count != 0)
            {
                Task currentTask = agv.Tasks[0];
                if (currentTask.Status == "Waiting")
                {
                    Debug.WriteLine("AAAAA");
                    agv.Path.Clear();
                    agv.Path.Add(Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance, agv.CurrentNode, agv.Tasks[0].PickNode));
                    AGV.FullPathOfAGV[agv.ID] = "N-0-"+agv.CurrentOrient+"-"+  Navigation.GetNavigationFrame(agv.Path[0], Node.MatrixNodeOrient)+"-N-0";
                    Communication.SendPathData(AGV.FullPathOfAGV[agv.ID]);
                    Display.UpdateComStatus("send",agv.ID,"Send Path 1", Color.Green);

                   // if(DashboardForm.timerToSendPathAgain.Enabled == false)
                      //   DashboardForm.timerToSendPathAgain.Start();

                  //  agv.Tasks[0].Status = "Doing";
                    agv.PathCopmpleted = 0;
                  //  agv.Status = "Running";
                }
                else if(agv.CurrentNode == currentTask.PickNode && currentTask.Status == "Doing" && agv.PathCopmpleted == 1)
                {
                    Debug.WriteLine("BBBB");
                    agv.Path.RemoveAt(0);
                    agv.Path.Add(Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance, agv.CurrentNode, agv.Tasks[0].DropNode));
                    string pick, drop;
                    if (currentTask.PickLevel == 1 || currentTask.PickLevel == 2 || currentTask.PickLevel == 3)
                    {
                        pick = "P-" + currentTask.PickLevel.ToString() + "-";
                    }
                    else
                    {
                        pick = "N-0-";
                    }
                    if (currentTask.DropLevel == 1 || currentTask.DropLevel == 2 || currentTask.DropLevel == 3)
                    {
                        drop = "-D-" + currentTask.DropLevel.ToString();
                    }
                    else
                    {
                        drop = "-N-0";
                    }
                    AGV.FullPathOfAGV[agv.ID] = pick+agv.CurrentOrient+"-" + Navigation.GetNavigationFrame(agv.Path[0], Node.MatrixNodeOrient)+drop;
                    Communication.SendPathData(AGV.FullPathOfAGV[agv.ID]);

                    Display.UpdateComStatus("send", agv.ID, "Send Path 2", Color.Green);

                    // if (DashboardForm.timerToSendPathAgain.Enabled == false)
                    //    DashboardForm.timerToSendPathAgain.Start();

                    //  agv.PathCopmpleted = 2;
                }
                else if (agv.CurrentNode == currentTask.DropNode && currentTask.Status == "Doing" && agv.PathCopmpleted == 3)
                {
                    Debug.WriteLine("CCCC");
                    agv.Tasks.RemoveAt(0);
                    agv.Path.Clear();
                    agv.Status = "Stop";
                    
                }
            }
        }

        public static void SimUpdatePathFromTaskOfAGVs(AGV agv)
        {

            // Clear all path (this do not affect Task.SimListTask)
            //agv.Tasks.Clear();

            // Find all task of this AGV
            //agv.Tasks = Task.SimListTask.FindAll(t => t.AGVID == agv.ID);&& agv.Path.Count ==0

            // if not having task or path has been initialized, skip to next AGV
            if (agv.Tasks.Count != 0  )
            {
                Task currentTask = agv.Tasks[0];

                //agv.Path.RemoveAt(0);agv.CurrentNode != currentTask.PickNode &&
                if (currentTask.Status == "Waiting")
                {
                    agv.Path.Clear();
                    agv.Path.Add(Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance, agv.CurrentNode, agv.Tasks[0].PickNode));
                    AGV.SimFullPathOfAGV[agv.ID] = Navigation.GetNavigationFrame(agv.Path[0], Node.MatrixNodeOrient);
                    agv.Tasks[0].Status = "Doing";
                    agv.PathCopmpleted = 0;                   
                    agv.Status = "Running";
                    Display.SimLabelAGV[agv.ID].BackColor = Color.CornflowerBlue;
                }

                else if (agv.CurrentNode == currentTask.PickNode && currentTask.Status == "Doing" && agv.PathCopmpleted == 1)
                {
                    
                    agv.Path.RemoveAt(0);
                    agv.Path.Add(Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance, agv.CurrentNode, agv.Tasks[0].DropNode));
                    
                    AGV.SimFullPathOfAGV[agv.ID] = Navigation.GetNavigationFrame(agv.Path[0], Node.MatrixNodeOrient);
                    agv.HavePallet = true;
                    if (currentTask.Type == "Order")
                    {
                        Pallet pallet = Pallet.SimListPallet.Find(c => c.Code == currentTask.PalletCode);
                        Display.DeleteLabelPallet(pallet);

                    }
                    else if (currentTask.Type == "Store")
                    {
                        
                    }
                    

                }
                else if ((agv.CurrentNode == currentTask.DropNode && currentTask.Status == "Doing" && agv.PathCopmpleted == 2))
                {

                    agv.Tasks.RemoveAt(0);
                    agv.Path.Clear();
                    agv.Status = "Stop";
                    Display.SimLabelAGV[agv.ID].BackColor = Color.Silver;
                    string timeComplete = DateTime.Now.ToString("dddd, MMMM dd, yyyy  h:mm:ss tt");
                    DBUtility.InsertCompleteTaskToDB("SimHistoryTask", currentTask, timeComplete, "Done");
                    agv.HavePallet = false;
                    if (currentTask.Type == "Order")
                    {
                        DBUtility.DeletePalletFromDB("SimPalletInfoTable", currentTask.PalletCode);
                        
                        Task.SimListTask.Remove(currentTask);
                    }
                    else if (currentTask.Type == "Store")
                    {
                        
                        //agv.HavePallet = false;
                        Pallet pallet = Pallet.SimStorePallet.Find(c => c.Code == currentTask.PalletCode);
                        pallet.DeliverTime = DateTime.Now.ToString("dddd, MMMM dd, yyyy  h:mm:ss tt");
                        DBUtility.InsertNewPalletToDB("SimPalletInfoTable", pallet.Code,pallet.Name, pallet.InStock, pallet.DeliverTime, pallet.AtBlock, pallet.AtColumn, pallet.AtLevel);
                        Pallet.SimStorePallet.Remove(pallet);
                        Pallet.SimListPallet.Add(pallet);
                        Display.UpdateLabelPallet(pallet);
                        Task.SimListTask.Remove(currentTask);

                    }
                    else if (currentTask.Type == "Input" || currentTask.Type == "Output")
                    {
                        Task.SimListTask.Remove(currentTask);
                    }


                }
            }     
            else
            {
                if (agv.CurrentNode == 55 )
                {
                    agv.Status = "Stop";
                    agv.Path.Clear() ;
                }
            }
           
        }

        public static void OutputAutoAdd(string palletCode, List<Task> listTaskToAdd, List<AGV> listAGV, List<RackColumn> listColumn)
        {
            // auto select agv
            if (AGV.SimListAGV.Count == 0) return;
            int agvID = 1;//AutoSelectAGV(AGV.SimListAGV);

            // find pick node & level
            RackColumn col = listColumn.Find(c => c.PalletCodes.Contains(palletCode));
            int pickNode = col.AtNode;
            int pickLevel = Array.IndexOf(col.PalletCodes, palletCode) + 1;

            // select drop node (output1 or output2)
            int dropNode = agvID % 2 == 1 ? 51 : 52;

            Task newTask = new Task("Auto " + palletCode, "Output", palletCode, agvID, pickNode, dropNode, pickLevel, 1, "Waiting");
            listTaskToAdd.Add(newTask);
        }
                               #region Function for auto select AGV

        public static int AutoSelectAGV(List<AGV> listAGV,int pickNode)
        {
            int selectedAGVID=0;
            int minDistance=10000;
            List<int> totalDistanceAllTasks = new List<int>();
            

            foreach (AGV agv in listAGV)
            {
                // if this agv has no task, select this agv
                if (agv.Status == "Stop")
                {
                    int distance =0;
                    List<int> path = Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance, agv.CurrentNode, pickNode);
                    for(int i=0; i<path.Count -2;i++)
                    {
                        
                        int dx = Math.Abs(Node.ListNode[path[i + 1]].X - Node.ListNode[path[i]].X);
                        int dy = Math.Abs(Node.ListNode[path[i + 1]].Y - Node.ListNode[path[i]].Y);                  
                        distance += (int)Math.Sqrt(dx * dx + dy * dy);
                       
                    }
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        selectedAGVID = agv.ID;
                    }
                }               
            }
            if (selectedAGVID == 0)
            {
                foreach (AGV agv in listAGV)
                {
                    
                    int distance = 0;
                    List<int> path = Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance, agv.Tasks[agv.Tasks.Count - 1].DropNode, pickNode);
                    for (int i = 0; i < path.Count - 2; i++)
                    {

                        int dx = Math.Abs(Node.ListNode[path[i + 1]].X - Node.ListNode[path[i]].X);
                        int dy = Math.Abs(Node.ListNode[path[i + 1]].Y - Node.ListNode[path[i]].Y);
                        distance += (int)Math.Sqrt(dx * dx + dy * dy);

                    }
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        selectedAGVID = agv.ID;
                    }
                }
            }
            return selectedAGVID;
        }
        public static string AutoSelecSlotPallet(List<Pallet> listPallet)
        {
            string palletLocation = "NaNN" ;
            string AtBlock = "D";
            
            int AtColumn = 6;
            int AtLevel = 1; 
            //Pallet pl = listPallet.Find(p=> pallet.AtBlock == AtBlock && pallet.AtColumn == AtColumn && pallet.AtLevel == AtLevel)
            //listPallet.Contains(p=> p.AtBlock == AtBlock && p.AtColumn == AtColumn && p.AtLevel == AtLevel)
            Pallet pallet = listPallet.Find(p=> p.AtBlock == AtBlock && p.AtColumn == AtColumn && p.AtLevel == AtLevel);
            while(listPallet.FindIndex(p => p.AtBlock == AtBlock && p.AtColumn == AtColumn && p.AtLevel == AtLevel)!= -1)
            {
                AtLevel += 1;
                if (AtLevel == 4)
                {
                    AtLevel = 1;
                    AtColumn -= 1;
                    if (AtColumn == 0)
                    {
                        AtColumn = 6;

                        switch (AtBlock)
                        {
                            case "D":
                                AtBlock = "C";
                                break;
                            case "C":
                                AtBlock = "B";
                                break;
                            case "B":
                                AtBlock = "A";
                                break;
                            case "A":
                                AtBlock = "NaN";
                                break;
                        }
                    }
                }

                
            }
            palletLocation = AtBlock + "," + AtColumn.ToString() + "," + AtLevel.ToString();
            //palletLocation = "FullSlot";
            return palletLocation;

        }
        private static int CalculateTotalRemainingDistance(AGV agv)
        {
            int totalDistance = 0;

            // get list remaning path of this agv
            List<List<int>> listRemainingPath = new List<List<int>>();
            for (int i = 0; i < agv.Tasks.Count; i++)
            {
                if (i == 0 && agv.Tasks[0].Status == "Doing")
                    listRemainingPath.Add(Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance,agv.CurrentNode, agv.Tasks[0].DropNode));
                else if (i == 0 && agv.Tasks[0].Status == "Waiting")
                {
                    listRemainingPath.Add(Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance,agv.CurrentNode, agv.Tasks[0].PickNode));
                    listRemainingPath.Add(Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance,agv.Tasks[0].PickNode, agv.Tasks[0].DropNode));
                }
                else
                    listRemainingPath.Add(Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance,agv.Tasks[i].PickNode, agv.Tasks[i].DropNode));

                if (i < agv.Tasks.Count - 1)
                    listRemainingPath.Add(Algorithm.A_starFindPath(Node.ListNode, Node.MatrixNodeDistance,agv.Tasks[i].DropNode, agv.Tasks[i + 1].PickNode));
            }

            // calculate total distance of list remaining path
            foreach (List<int> path in listRemainingPath)
            {
                for (int i = 0; i < path.Count - 1; i++)
                    totalDistance += Node.MatrixNodeDistance[path[i], path[i + 1]];
            }

            return totalDistance;
        }

        #endregion
    }
}
