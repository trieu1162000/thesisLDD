using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AGV_Form
{
    class RackColumn
    {
        public string Block { get; set; }
        public int Number { get; set; }
        //public int Slot { get; set; }
        public int AtNode { get; set; }
        public Label ColumnLabel = new Label();

        // Store pallet code, PalletCodes[i - 1] for level i of rack
        private static int NumOfLevel = 3;
        public string[] PalletCodes = new string[NumOfLevel];

        public static List<RackColumn> ListColumn = RackColumn.GetRackColums();
        

        private RackColumn(int atNode)
        {
            Node n = Node.ListNode.Find(nd => nd.ID == atNode);
            string[] locationCode = n.LocationCode.Split(new char[] { '-' }, StringSplitOptions.RemoveEmptyEntries);
            this.AtNode = n.ID;
            this.Block = locationCode[0];
            this.Number = Convert.ToInt16(locationCode[1]);
        }

        // Get list of rack column
        private static List<RackColumn> GetRackColums()
        {
            List<RackColumn> listCol = new List<RackColumn>();
            foreach (Node n in Node.ListNode)
            {
                if (n.LocationCode.Length == 0 || n.ID == 55 || n.ID == 56) continue;

                RackColumn col = new RackColumn(n.ID);
                listCol.Add(col);
            }
            return listCol;
        }
    }
    class Pallet
    {
        public string Code { get; set; }
        public string Name { get; set; }
        public bool InStock { get; set; }
        public string StoreTime { get; set; }
        public string DeliverTime { get; set; }
        public string AtBlock { get; set; }
        public int AtColumn { get; set; }
        public int AtLevel { get; set; }
        public Label label = new Label();

        public Pallet(string code,string name, bool isInWarehouse, string storeTime, string block, int column, int level)
        {
            this.Code = code;
            this.Name = name;
            this.StoreTime = storeTime;
            this.InStock = isInWarehouse;
            this.AtBlock = block;
            this.AtColumn = column;
            this.AtLevel = level;
        }
        
        // Save pallet info (the present and the past)
        public static List<Pallet> ListPallet = new List<Pallet>();
        public static List<Pallet> SimListPallet = new List<Pallet>();
       
        public static List<Pallet> SimStorePallet = new List<Pallet>();
        public static List<Pallet> StorePallet = new List<Pallet>();
        public static void SaveDeliveryTime(string palletCode, List<Pallet> listPallet)
        {
            Pallet pallet = listPallet.Find(p => p.Code == palletCode);
            if (pallet == null) return;
            pallet.DeliverTime = DateTime.Now.ToString("dddd, MMMM dd, yyyy  h:mm:ss tt");
            pallet.InStock = false;
        }
    }
}
