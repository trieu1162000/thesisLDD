using System;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AGV_Form
{
    class DBUtility
    {
        private const string connectionStr = @"Data Source=DESKTOP-TN7L9R8\PERFECT;Initial Catalog=AGV_Datasource;Integrated Security=True";
        public static dynamic GetDataFromDB<T>(string tableName)
        {
            List<Node> listNode = new List<Node>();
            DataTable table = new DataTable();

            using (SqlConnection connection = new SqlConnection(connectionStr))
            {
                //SqlComnection
                connection.Open();

                //SqlCommand
                SqlCommand command = connection.CreateCommand();
                command.CommandText = "select * from " + tableName;

                //SqlAdapter
                SqlDataAdapter adapter = new SqlDataAdapter(command);

                //Get data
                table.Clear();
                adapter.Fill(table);
                connection.Close();
            }

            // Store DataTable into a List
            // Note: listNode[i].ID = i so instead of using ID, use i 
            // (because of the order of the rows in table)
            listNode = (from DataRow dr in table.Rows
                        select new Node()
                        {
                            ID = Convert.ToInt32(dr["Node"]),
                            X = Convert.ToInt32(dr["pos_X"]),
                            Y = Convert.ToInt32(dr["pos_Y"]),
                            AdjacentNode = (dr["AdjacentNode"].ToString()).
                                            Split(new char[] { ';', ' ' }, StringSplitOptions.RemoveEmptyEntries),
                            LocationCode = dr["LocationCode"].ToString()
                        }).ToList();

            if (typeof(T) == typeof(DataTable)) return table;
            else if (typeof(T) == typeof(List<Node>)) return listNode;
            else return null;
        }
        public static dynamic GetPalletInfoFromDB<T>(string tableName)
        {
            DataTable table = new DataTable();
            List<Pallet> listPallet = new List<Pallet>();

            using (SqlConnection connection = new SqlConnection(connectionStr))
            {
                //SqlComnection
                connection.Open(); // don't need because using SqlDataAdapter

                //SqlCommand
                SqlCommand command = connection.CreateCommand();
                command.CommandText = "select * from " + tableName;

                //SqlAdapter
                SqlDataAdapter adapter = new SqlDataAdapter(command);

                //Get data
                table.Clear();
                adapter.Fill(table);
                connection.Close(); // don't need because using SqlDataAdapter
            }

            // Store DataTable into a List
            listPallet = (from DataRow dr in table.Rows
                          where Convert.ToBoolean(dr["InStock"]) == true
                          select new Pallet(dr["PalletCode"].ToString(),dr["PalletName"].ToString(), Convert.ToBoolean(dr["InStock"]), dr["StoreTime"].ToString(),
                                            dr["AtBlock"].ToString(), Convert.ToInt16(dr["AtColumn"]), Convert.ToInt16(dr["AtLevel"]))
                          ).ToList();

            if (typeof(T) == typeof(DataTable)) return table;
            else if (typeof(T) == typeof(List<Pallet>)) return listPallet;
            else return null;
        }
        public static void InsertNewPalletToDB(string tableName, string palletCode,string palletName, bool inStock, string storeTime,
                                               string block, int column, int level)
        {
            using (SqlConnection connection = new SqlConnection(connectionStr))
            {
                //SqlComnection
                connection.Open();

                //SqlCommand
                SqlCommand command = connection.CreateCommand();
                command.CommandText = "insert into " + tableName + " (PalletCode,PalletName, InStock, StoreTime, AtBlock, AtColumn, AtLevel)"
                                                     + " values(@palletCode,@palletName, @inStock, @storeTime, @atBlock, @atColumn, @atLevel)";
                command.Parameters.Add("@palletCode", SqlDbType.NVarChar).Value = palletCode;
                command.Parameters.Add("@palletName", SqlDbType.NVarChar).Value = palletName;
                command.Parameters.Add("@inStock", SqlDbType.Bit).Value = inStock;
                command.Parameters.Add("@storeTime", SqlDbType.NVarChar).Value = storeTime;
                command.Parameters.Add("@atBlock", SqlDbType.NVarChar).Value = block;
                command.Parameters.Add("@atColumn", SqlDbType.Int).Value = column;
                command.Parameters.Add("@atLevel", SqlDbType.Int).Value = level;
                command.ExecuteNonQuery();

                connection.Close();
            }
        }
        public static void DeletePalletFromDB(string tableName, string palletCode)
        {
            using (SqlConnection connection = new SqlConnection(connectionStr))
            {
                //SqlComnection
                connection.Open();

                //SqlCommand
                SqlCommand command = connection.CreateCommand();
                command.CommandText = "delete " + tableName + " where PalletCode = @palletCode";
                command.Parameters.Add("@palletCode", SqlDbType.NVarChar).Value = palletCode;
                command.ExecuteNonQuery();

                connection.Close();
            }
        }

        public static dynamic GetHisTaskFromDB<T>(string tableName)
        {
            
            DataTable table = new DataTable();

            using (SqlConnection connection = new SqlConnection(connectionStr))
            {
                //SqlComnection
                connection.Open();

                //SqlCommand
                SqlCommand command = connection.CreateCommand();
                command.CommandText = "select * from " + tableName;

                //SqlAdapter
                SqlDataAdapter adapter = new SqlDataAdapter(command);

                //Get data
                table.Clear();
                adapter.Fill(table);
                connection.Close();
            }

           
            if (typeof(T) == typeof(DataTable)) return table;
            
            else return null;
        }
        public static void InsertCompleteTaskToDB(string tableName, Task task, string time, string status)
        {
            //string palletCode, bool inStock, string storeTime string block, int column, int level
            using (SqlConnection connection = new SqlConnection(connectionStr))
            {
                //SqlComnection
                connection.Open();

                //SqlCommand
                SqlCommand command = connection.CreateCommand();
                command.CommandText = "insert into " + tableName + " ([Task Name], Status,[Completion Time],[Pallet Code], Type, AGV,[Pick Node],[Drop Node])"
                                                     + " values(@taskname, @status, @completiontime, @palletcode, @type, @agv,@picknode,@dropnode)";
                command.Parameters.Add("@taskname", SqlDbType.NVarChar).Value = task.Name;
                command.Parameters.Add("@status", SqlDbType.NVarChar).Value = status;
                command.Parameters.Add("@completiontime", SqlDbType.NVarChar).Value = time;
                command.Parameters.Add("@palletcode", SqlDbType.NVarChar).Value = task.PalletCode;
                command.Parameters.Add("@type", SqlDbType.NVarChar).Value = task.Type;
                command.Parameters.Add("@agv", SqlDbType.NVarChar).Value = task.AGVID;
                command.Parameters.Add("@picknode", SqlDbType.NVarChar).Value = task.PickNode + "- Slot" + task.PickLevel;
                command.Parameters.Add("@dropnode", SqlDbType.NVarChar).Value = task.DropNode + "- Slot" + task.DropLevel;
                command.ExecuteNonQuery();

                connection.Close();
            }
        }
    }
}

