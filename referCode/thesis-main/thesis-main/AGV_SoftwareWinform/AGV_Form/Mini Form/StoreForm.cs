using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AGV_Form
{
    public partial class StoreForm : Form
    {
        private int Input;
        public StoreForm(int input)
        {
            InitializeComponent();
            Input = input;
        }
        private void StoreForm_Load(object sender, EventArgs e)
        {
            List<Pallet> palletsInStock = new List<Pallet>();
            lstvwPalletInStock.Items.Clear();

            switch (Display.Mode)
            {
                case "Real Time":
                    lbMode.Text = "Mode: Real Time";
                    Pallet.ListPallet = DBUtility.GetPalletInfoFromDB<List<Pallet>>("PalletInfoTable");
                    foreach (Pallet pallet in Pallet.ListPallet)
                    {
                        lstvwPalletInStock.Items.Add(pallet.Code, 0);
                        lstvwPalletInStock.Items[lstvwPalletInStock.Items.Count - 1].SubItems.Add(pallet.Name.Trim());
                        lstvwPalletInStock.Items[lstvwPalletInStock.Items.Count - 1].SubItems.Add(pallet.StoreTime.Trim());
                        lstvwPalletInStock.Items[lstvwPalletInStock.Items.Count - 1].SubItems.Add(pallet.AtBlock + "-" + pallet.AtColumn.ToString() + "-" + pallet.AtLevel.ToString());
                    }
                    break;
                case "Simulation":

                    lbMode.Text = "Mode: Simulation";
                    Pallet.SimListPallet = DBUtility.GetPalletInfoFromDB<List<Pallet>>("SimPalletInfoTable");
                    foreach (Pallet pallet in Pallet.SimListPallet)
                    {
                        lstvwPalletInStock.Items.Add(pallet.Code, 0);
                        lstvwPalletInStock.Items[lstvwPalletInStock.Items.Count - 1].SubItems.Add(pallet.Name.Trim());
                        lstvwPalletInStock.Items[lstvwPalletInStock.Items.Count - 1].SubItems.Add(pallet.StoreTime.Trim());
                        lstvwPalletInStock.Items[lstvwPalletInStock.Items.Count - 1].SubItems.Add(pallet.AtBlock + "-" + pallet.AtColumn.ToString() + "-" + pallet.AtLevel.ToString());
                    }


                    break;
            }

           
        }
        private void btnStore_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(txtPalletCode.Text) || String.IsNullOrEmpty(txtPalletName.Text) ||
             String.IsNullOrEmpty(cbbBlock.Text) || String.IsNullOrEmpty(cbbColumn.Text) || String.IsNullOrEmpty(cbbLevel.Text))
            {
                MessageBox.Show(" Please enter correct Pallet informarion !", "Pallet Information ", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            //int agvID = Task.AutoSelectAGV(AGV.SimListAGV, Input);
            int agvID = 1;
            RackColumn rack = RackColumn.ListColumn.Find(c => c.Block == cbbBlock.Text && c.Number == Convert.ToInt32(cbbColumn.Text));
            string palletcode = txtPalletCode.Text;
            string palletName = txtPalletName.Text;
            int pickNode = Input;
            int pickLevel = 1;

            int dropNode = rack.AtNode;
            int dropLevel = Convert.ToInt32(cbbLevel.Text);
            switch (Display.Mode)
            {
                case "Real Time":
                    foreach (Pallet palet in Pallet.ListPallet)
                    {
                        if (palet.Code.Trim() == txtPalletCode.Text)
                        {
                            MessageBox.Show("Pallet has stored in stock !", "Store Pallet Error !", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                        else if (palet.AtBlock == cbbBlock.Text && palet.AtColumn == Convert.ToInt32(cbbColumn.Text) && palet.AtLevel == Convert.ToInt32(cbbLevel.Text))
                        {
                            MessageBox.Show("Slot is not available !", "Store Pallet Error !", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                    }
                    break;
                case "Simulation":
                    foreach (Pallet palet in Pallet.SimListPallet)
                    {
                        if (palet.Code.Trim() == txtPalletCode.Text)
                        {
                            MessageBox.Show("Pallet has stored in stock !", "Store Pallet Error !", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                        else if (palet.AtBlock == cbbBlock.Text && palet.AtColumn == Convert.ToInt32(cbbColumn.Text) && palet.AtLevel == Convert.ToInt32(cbbLevel.Text))
                        {
                            MessageBox.Show("Slot is not available !", "Store Pallet Error !", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                    }
                    break;
            }
           
            
            Task task = new Task("Store1", "Store", palletcode, agvID,
                                 pickNode, dropNode, pickLevel, dropLevel
                                 , "Waiting");
            Pallet pallet = new Pallet(palletcode, palletName, true,"??", cbbBlock.Text, Convert.ToInt32(cbbColumn.Text), Convert.ToInt32(cbbLevel.Text));
            switch (Display.Mode)
            {
                case "Real Time":
                    int AGVindex = AGV.ListAGV.FindIndex(a => { return a.ID == agvID; });
                    AGV.ListAGV[AGVindex].Tasks.Add(task);
                    Task.ListTask.Add(task);
                    Pallet.StorePallet.Add(pallet);
                    break;
                case "Simulation":
                    int SimAGVindex = AGV.SimListAGV.FindIndex(a => { return a.ID == agvID; });
                    AGV.SimListAGV[SimAGVindex].Tasks.Add(task);
                    Task.SimListTask.Add(task);
                    Pallet.SimStorePallet.Add(pallet);
                    break;
            }
            ckbAutoSelectSlot.Checked = false;
            cbbBlock.Enabled = true;
            cbbColumn.Enabled = true;
            cbbLevel.Enabled = true;
        }

        private void timerListview_Tick(object sender, EventArgs e)
        {
            switch (Display.Mode)
            {
                case "Real Time":
                    //Display.UpdateListViewTasks(listViewTask, Task.ListTask);
                    if(AGV.ListAGV.Count >0)
                    Display.UpdateListViewTasks(listViewTask, AGV.ListAGV[0].Tasks);
                    break;
                case "Simulation":
                    Display.UpdateListViewTasks(listViewTask, Task.SimListTask);
                    break;
            }
            
        }

       

        private void ckbAutoSelectSlot_CheckedChanged(object sender, EventArgs e)
        {
            if (ckbAutoSelectSlot.Checked)
            {
                string Slot = Task.AutoSelecSlotPallet(Pallet.SimListPallet.Concat(Pallet.SimStorePallet).ToList());
                string[] arrSlot = Slot.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
                cbbBlock.Text = arrSlot[0];
                cbbColumn.Text = arrSlot[1];
                cbbLevel.Text = arrSlot[2];
                cbbBlock.Enabled = false;
                cbbColumn.Enabled = false;
                cbbLevel.Enabled = false;
            }
            else
            {
                cbbBlock.Enabled = true;
                cbbColumn.Enabled = true;
                cbbLevel.Enabled = true;
            }
        }
    }
}
