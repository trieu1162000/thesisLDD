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
    public partial class AddRemoveAGVForm : Form
    {
        public AddRemoveAGVForm()
        {
            InitializeComponent();
        }

        private List<AGV> listNewAGV = new List<AGV>();
        private void AddRemoveAGVForm_Load(object sender, EventArgs e)
        {
            if(Display.Mode == "Real Time")
            cbbModeList.SelectedItem = "Real Time";
            else
            cbbModeList.SelectedItem = "Simulation";
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(txbID.Text) || String.IsNullOrEmpty(cbbExitNode.Text) ||
               String.IsNullOrEmpty(cbbOrientation.Text) || String.IsNullOrEmpty(txbDistance.Text))
                return;
            AGV agv = new AGV(Convert.ToInt16(txbID.Text), Convert.ToInt16(cbbExitNode.Text),
                              Convert.ToChar(cbbOrientation.Text), Convert.ToSingle(txbDistance.Text), "Stop");
            
            switch (cbbModeList.Text)
            {
                case "Real Time":
                    // Check whether AGV ID exist in old and new list or not
                    foreach (AGV a in AGV.ListAGV)
                    {
                        if (Convert.ToInt16(txbID.Text) == a.ID)
                        {
                            MessageBox.Show("AGV ID already exists.\nPlease choose other AGV ID", "Error",
                                             MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                    }
                    // If not exist, add new AGV into listNewAGV
                    AGV.ListAGV.Add(agv);
                    Display.AddLabelAGV(cbbModeList.Text, agv.ID, agv.CurrentNode, agv.CurrentOrient, agv.DistanceToCurrentNode);
                    // Put new AGV ID in listView
                    listViewAGV.Items.Add(" AGV#" + agv.ID, 0);
                    MessageBox.Show(" Add AGV#" + agv.ID.ToString() + " Successful !", "Updates AGV", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    break;
                case "Simulation":
                    // Check whether AGV ID exist in old and new list or not
                    foreach (AGV a in AGV.SimListAGV)
                    {
                        if (Convert.ToInt16(txbID.Text) == a.ID)
                        {
                            MessageBox.Show("AGV ID already exists.\nPlease choose other AGV ID", "Error",
                                             MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                    }
                    // If not exist, add new AGV into listNewAGV
                    agv.Velocity = Convert.ToSingle(txtVelocity.Text);                    
                    AGV.SimListAGV.Add(agv);
                    AGV.SimFullPathOfAGV[agv.ID] = agv.CurrentNode.ToString() + "-G";
                    Display.AddLabelAGV(cbbModeList.Text, agv.ID, agv.CurrentNode, agv.CurrentOrient, agv.DistanceToCurrentNode);
                    // Put new AGV ID in listView
                    listViewAGV.Items.Add(" AGV#" + agv.ID, 0);
                    MessageBox.Show(" Add AGV#" + agv.ID.ToString() + " Successful !", "Updates AGV", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    break;
            }

            
            // Clear textBox for next adding
            txbID.Clear();
            txbDistance.Clear();
            txtVelocity.Clear();
        }



        private void btnRemove_Click(object sender, EventArgs e)
        {
            // Remove an AGV has ID in comboBox
            if (String.IsNullOrEmpty(cbbIDRemove.Text) == false)
            {

                switch (cbbModeList.Text)
                {
                    case "Real Time":
                        AGV agvRealRemove = AGV.ListAGV.Find(a => { return a.ID == Convert.ToInt16(cbbIDRemove.Text); });
                        if (AGV.ListAGV.Contains(agvRealRemove))
                            AGV.ListAGV.Remove(agvRealRemove);
                        // Put remaining AGV in listView  
                        listViewAGV.Clear();
                        foreach (AGV agv in AGV.ListAGV)
                        {
                            listViewAGV.Items.Add(" AGV#" + agv.ID, 0);
                        }
                        cbbIDRemove.Items.Clear();
                        MessageBox.Show(" Remove AGV#" + agvRealRemove.ID.ToString() + " Successful !", "Updates AGV", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        break;
                    case "Simulation":
                        AGV agvSimRemove = AGV.SimListAGV.Find(a => { return a.ID == Convert.ToInt16(cbbIDRemove.Text); });
                        if (AGV.SimListAGV.Contains(agvSimRemove))
                        {
                            AGV.SimListAGV.Remove(agvSimRemove);
                            Display.RemoveLabelAGV(cbbModeList.Text,agvSimRemove.ID);
                        }
                            
                        // Put remaining AGV in listView  
                        listViewAGV.Clear();
                        foreach (AGV agv in AGV.SimListAGV)
                        {
                            listViewAGV.Items.Add(" AGV#" + agv.ID, 0);
                        }
                        cbbIDRemove.Items.Clear();
                        MessageBox.Show(" Remove AGV#" + agvSimRemove.ID.ToString() + " Successful !", "Updates AGV", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        break;
                }
                
            }                  
        }
        private void txbID_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (!char.IsDigit(e.KeyChar) && !char.IsControl(e.KeyChar))
            {
                e.Handled = true;
                MessageBox.Show("Only allow digit", "AGV ID Type", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void txbDistance_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (!char.IsDigit(e.KeyChar) && !char.IsControl(e.KeyChar))
            {
                e.Handled = true;
                MessageBox.Show("Only allow digit", "AGV ID Type", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void cbbModeList_SelectedValueChanged(object sender, EventArgs e)
        {
            listViewAGV.Clear();
            switch (cbbModeList.Text)
            {
                case "Real Time":

                    // Put existing AGV in ListAGV and listView
                    txtVelocity.Enabled = false;
                    foreach (AGV agv in AGV.ListAGV)
                    {
                        listViewAGV.Items.Add(" AGV#" + agv.ID, 0);
                    }
                    break;
                case "Simulation":
                    txtVelocity.Enabled = true;
                    // Put existing AGV in SimListAGV and listView
                    foreach (AGV agv in AGV.SimListAGV)
                    {
                        listViewAGV.Items.Add(" AGV#" + agv.ID, 0);
                    }
                    break;
            }
        }


        private void cbbIDRemove_Click(object sender, EventArgs e)
        {
            // Display ID in combobox if AGV exist on listView
            cbbIDRemove.Items.Clear();
            switch (cbbModeList.Text)
            {
                case "Real Time":

                    // Put existing AGV in ListAGV and listView

                    foreach (AGV agv in AGV.ListAGV)
                    {
                        cbbIDRemove.Items.Add(agv.ID);
                    }
                    break;
                case "Simulation":

                    // Put existing AGV in SimListAGV and listView
                    foreach (AGV agv in AGV.SimListAGV)
                    {
                        cbbIDRemove.Items.Add(agv.ID);
                    }
                    break;
            }
        }

        private void btnEdit_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(txbID.Text) || String.IsNullOrEmpty(cbbExitNode.Text) ||
               String.IsNullOrEmpty(cbbOrientation.Text) || String.IsNullOrEmpty(txbDistance.Text))
                return;
         
            switch (cbbModeList.Text)
            {
                case "Real Time":
                    // Check whether AGV ID exist in old and new list or not
                    bool agvRealAdded = false;
                    foreach (AGV a in AGV.ListAGV)
                    {
                        if (Convert.ToInt16(txbID.Text) == a.ID)
                        {
                            a.CurrentNode = Convert.ToInt16(cbbExitNode.Text);
                            a.CurrentOrient = Convert.ToChar(cbbOrientation.Text);
                            a.DistanceToCurrentNode = Convert.ToInt16(txbDistance.Text);
                            a.Status = "Stop";
                            agvRealAdded = true;
                            MessageBox.Show(" Edit AGV#" + a.ID.ToString() + " Successful !", "Updates AGV", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        }
                    }
                    if (!agvRealAdded)
                    MessageBox.Show("AGV ID is not in AGV List !", "Updates AGV", MessageBoxButtons.OK, MessageBoxIcon.Hand);
                    break;
                case "Simulation":
                    // Check whether AGV ID exist in old and new list or 
                    bool agvSimAdded = false;
                    foreach (AGV a in AGV.SimListAGV)
                    {
                        if (Convert.ToInt16(txbID.Text) == a.ID)
                        {
                            Display.RemoveLabelAGV(cbbModeList.Text,a.ID);
;                            a.CurrentNode = Convert.ToInt16(cbbExitNode.Text);
                            a.CurrentOrient = Convert.ToChar(cbbOrientation.Text);
                            a.DistanceToCurrentNode = Convert.ToInt16(txbDistance.Text);
                            a.Status = "Stop";
                            Display.AddLabelAGV(cbbModeList.Text,a.ID,a.CurrentNode,a.CurrentOrient,a.DistanceToCurrentNode);
                            agvSimAdded = true;
                            MessageBox.Show(" Edit AGV#" + a.ID.ToString() + " Successful !", "Updates AGV", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        }
                    }
                    if (!agvSimAdded)
                        MessageBox.Show("AGV ID is not in AGV List !", "Updates AGV", MessageBoxButtons.OK, MessageBoxIcon.Hand);
                    break;
            }
        }

        private void txtVelocity_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (!char.IsDigit(e.KeyChar) && !char.IsControl(e.KeyChar) && !(e.KeyChar=='.') )
            {
                e.Handled = true;
                MessageBox.Show("Only allow digit", "AGV ID Type", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }
    }
}
