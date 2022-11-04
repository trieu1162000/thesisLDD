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
    public partial class TasksForm : Form
    {
        public TasksForm()
        {
            InitializeComponent();
        }

        private void TasksForm_Load(object sender, EventArgs e)
        {
            if (Display.Mode == "Real Time")
                cbbModeList.SelectedItem = "Real Time";
            else
                cbbModeList.SelectedItem = "Simulation";
            DataTable table = new DataTable();
            table = DBUtility.GetHisTaskFromDB<DataTable>("SimHistoryTask");
            
            dgvHistoryTask.DataSource = table;
            dgvHistoryTask.Columns[0].Width = 70;
            dgvHistoryTask.Columns[1].Width = 60;
            dgvHistoryTask.Columns[2].Width = 250;
            dgvHistoryTask.Columns[3].Width = 70;
            dgvHistoryTask.Columns[4].Width = 60;
            dgvHistoryTask.Columns[5].Width = 50;
            dgvHistoryTask.Columns[6].Width = 80;
            dgvHistoryTask.Columns[7].Width = 80;
        }

        private void timerListview_Tick(object sender, EventArgs e)
        {
            switch (cbbModeList.Text)
            {
                case "Real Time":
                    // Update data in listView AGVs
                   

                    

                    break;
                case "Simulation":
                    // Update data in listView AGVs
                    
                    Display.UpdateListViewTasks(listViewTasks, Task.SimListTask);
                    // Update location of AGV icon (label)
                    //Display.UpdateListViewTasks(listViewTasks, Task.SimListTask);


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
            listViewTasks.Items.Add(task.Name, 0);
            listViewTasks.Items[listViewTasks.Items.Count - 1].SubItems.Add(task.Type);
            listViewTasks.Items[listViewTasks.Items.Count - 1].SubItems.Add(task.PalletCode);
            listViewTasks.Items[listViewTasks.Items.Count - 1].SubItems.Add("AGV#" + task.AGVID.ToString());
            listViewTasks.Items[listViewTasks.Items.Count - 1].SubItems.Add("Node " + task.PickNode.ToString() + "-" + task.PickLevel.ToString());
            listViewTasks.Items[listViewTasks.Items.Count - 1].SubItems.Add("Node " + task.DropNode.ToString() + "-" + task.DropLevel.ToString());
            //listNewTask.Add(task);
            int AGVindex = AGV.SimListAGV.FindIndex(a => { return a.ID == Convert.ToInt16(agvID[1]); });
            AGV.SimListAGV[AGVindex].Tasks.Add(task);
            Task.SimListTask.Add(task);
            // Clear textBox for next adding
            txbTaskName.Clear();
            txbPalletCode.Clear();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            SearchTask();
            //DataTable table1 = table.Clone();
            //for(int i=0;i<result.Length;i++)
            //{
            //    table1.Rows.Add(result[i]);
            //}
            //dataGridView1.DataSource = table1;
        }
        private void SearchTask()
        {
            DataTable table = new DataTable();

            switch (cbbModeList.Text)
            {
                case "Real Time":
                    // Update data in listView AGVs




                    break;
                case "Simulation":

                    table = DBUtility.GetHisTaskFromDB<DataTable>("SimHistoryTask");
                    //dgvPalletInfo.DataSource = table;
                    string searchQuery = cbbFilter.Text + "='" + txtName.Text + "'";
                    DataRow[] result = table.Select(searchQuery);
                    //result[0][1].ToString();
                    listviewSearch.Items.Clear();
                    for (int i = 0; i < result.Length; i++)
                    {
                        listviewSearch.Items.Add(result[i][0].ToString());
                        listviewSearch.Items[listviewSearch.Items.Count - 1].SubItems.Add(result[i][1].ToString());
                        listviewSearch.Items[listviewSearch.Items.Count - 1].SubItems.Add(result[i][2].ToString());
                        listviewSearch.Items[listviewSearch.Items.Count - 1].SubItems.Add(result[i][3].ToString());
                        listviewSearch.Items[listviewSearch.Items.Count - 1].SubItems.Add(result[i][4].ToString());
                        listviewSearch.Items[listviewSearch.Items.Count - 1].SubItems.Add(result[i][5].ToString());
                        listviewSearch.Items[listviewSearch.Items.Count - 1].SubItems.Add(result[i][6].ToString());
                        listviewSearch.Items[listviewSearch.Items.Count - 1].SubItems.Add(result[i][7].ToString());
                    }


                    break;
            }
        }

        private void txtName_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.KeyCode == Keys.Enter)
            {
                SearchTask();
            }
        }
    }
}
