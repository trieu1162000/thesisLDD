using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AGV_Form
{
    class Navigation
    {
        // Create the navigation frame will be sent to actuator system
        public static string GetNavigationFrame(List<int> path, char[,] MatrixOrient)
        {
            
            // Determin turn-direction and add it to frame
            string frame = path[0].ToString()+"-";
            for (int i = 1; i < path.Count; i++)
            {
                frame += MatrixOrient[path[i - 1], path[i]] + "-" + path[i].ToString() + "-"  ;              
            }
            frame += "G"; // Goal node
            return frame;
        }
    }
}
