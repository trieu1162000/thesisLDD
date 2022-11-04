using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AGV_Form
{
    class AGV
    {
        public int ID { get; set; }
        public int CurrentNode { get; set; }
        public char CurrentOrient { get; set; }
        public float DistanceToCurrentNode { get; set; }
        public string Status { get; set; }
        public float Velocity { get; set; }
        public bool HavePallet = false;
        public bool IsColision = false;
        public bool Stop = false;
       // public bool IsInitialized = false; // only use in Real Time mode

        // Constructor of AGV with some initial information
        public AGV(int id, int initNode, char initOrient, float distanceToInitNode, string status)
        {
            this.ID = id;
            this.CurrentNode = initNode;
            this.CurrentOrient = initOrient;
            this.DistanceToCurrentNode = distanceToInitNode;
            this.Status = status;
        }

        // Assume that max mumber of AGV is 100
        public static int MaxNumOfAGVs = 100;

        // Information list of real-time AGV
        public static List<AGV> ListAGV = new List<AGV>();

        // Information list of simulation AGV
        public static List<AGV> SimListAGV = new List<AGV>();

        // List of path of AGV will run, ListPathOfAGV[i] for AGV ID = i;
        public static List<int>[] ListPathOfAGV = new List<int>[MaxNumOfAGVs];

        // then next path will be at Path[0] because of List<> characteristic 
        public List<List<int>> Path = new List<List<int>>();
        public int PathCopmpleted = 0;

        public static string[] FullPathOfAGV = new string[10];
        public static string[] SimFullPathOfAGV = new string[10];
        //public string CurrentTask { get; set; }
        public List<Task> Tasks = new List<Task>();
    }
}

