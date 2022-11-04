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
    public partial class AddTaskAGVForm : Form
    {
        public AddTaskAGVForm()
        {
            InitializeComponent();
        }
        private List<Task> listOldTask = new List<Task>();
        private List<Task> listNewTask = new List<Task>();

        private void AddTaskAGVForm_Load(object sender, EventArgs e)
        {
            switch (Display.Mode)
            {
                case "Real Time":
                    // Create a copy of current ListTask
                    listOldTask = new List<Task>(Task.ListTask);
                    // Put existing Task in ListTask and listViewTask
                    foreach (Task task in Task.ListTask)
                    {
                        listViewTask.Items.Add(task.Name, 0);
                        listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add(task.Type);
                        listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add(task.PalletCode);
                        listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add("AGV#" + task.AGVID.ToString());
                        listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add("Node " + task.PickNode.ToString() + "-" + task.PickLevel.ToString());
                        listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add("Node " + task.DropNode.ToString() + "-" + task.DropLevel.ToString());
                    }
                    break;
                case "Simulation":
                    // Create a copy of current SimListTask
                   listOldTask = new List<Task>(Task.SimListTask);
                    // Put existing Task in SimListTask and listViewTask
                    foreach (Task task in Task.SimListTask)
                    {
                        listViewTask.Items.Add(task.Name, 0);
                        listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add(task.Type);
                        listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add(task.PalletCode);
                        listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add("AGV#" + task.AGVID.ToString());
                        listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add("Node " + task.PickNode.ToString() + "-" + task.PickLevel.ToString());
                        listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add("Node " + task.DropNode.ToString() + "-" + task.DropLevel.ToString());
                    }
                    break;
            }
        }

        

        

        private void cbbAGV_Click(object sender, EventArgs e)
        {
            // Display AGV in combobox if AGV exist
            cbbAGV.Items.Clear();
            switch (Display.Mode)
            {
                case "Real Time":
                    AGV.ListAGV.ForEach(agv => cbbAGV.Items.Add("AGV#" + agv.ID));
                    break;
                case "Simulation":
                    AGV.SimListAGV.ForEach(agv => cbbAGV.Items.Add("AGV#" + agv.ID));
                    break;
            }
        }

        private void btnAdd_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(txbTaskName.Text) || String.IsNullOrEmpty(cbbType.Text) ||
               String.IsNullOrEmpty(txbPalletCode.Text) || String.IsNullOrEmpty(cbbAGV.Text) ||
               String.IsNullOrEmpty(cbbPickNode.Text) || String.IsNullOrEmpty(cbbDropNode.Text))
                return;
            // Check whether TaskName exist in old and new list or not
            string[] agvID = cbbAGV.Text.Split(new char[] { '#' }, StringSplitOptions.RemoveEmptyEntries);
            string[] pickAt = cbbPickNode.Text.Split(new char[] { '-' }, StringSplitOptions.RemoveEmptyEntries);
            string[] dropAt = cbbDropNode.Text.Split(new char[] { '-' }, StringSplitOptions.RemoveEmptyEntries);
            Task task = new Task(txbTaskName.Text, cbbType.Text, txbPalletCode.Text,
                                 Convert.ToInt16(agvID[1]), Convert.ToInt16(pickAt[0]), Convert.ToInt16(dropAt[0]),
                                 Convert.ToInt16(pickAt[1]), Convert.ToInt16(dropAt[1]), "Waiting");
            // Put new Task in listView
            listViewTask.Items.Add(task.Name, 0);
            listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add(task.Type);
            listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add(task.PalletCode);
            listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add("AGV#" + task.AGVID.ToString());
            listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add("Node " + task.PickNode.ToString() + "-" + task.PickLevel.ToString());
            listViewTask.Items[listViewTask.Items.Count - 1].SubItems.Add("Node " + task.DropNode.ToString() + "-" + task.DropLevel.ToString());
            listNewTask.Add(task);
            int AGVindex = AGV.SimListAGV.FindIndex(a => { return a.ID == Convert.ToInt16(agvID[1]); });
            AGV.SimListAGV[AGVindex].Tasks.Add(task);
            Task.SimListTask.Add(task);
            // Clear textBox for next adding
            txbTaskName.Clear();
            txbPalletCode.Clear();
            
        }
    }
}
