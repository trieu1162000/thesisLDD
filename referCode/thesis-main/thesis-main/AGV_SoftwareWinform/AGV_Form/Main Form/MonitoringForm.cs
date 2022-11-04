using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using System.Windows.Forms;
using System.IO.Ports;
using ZedGraph;

namespace AGV_Form
{
    public partial class MonitoringForm : Form
    {
        public MonitoringForm()
        {
            InitializeComponent();
            InitZedGraph();
        }
        private int tickStart;
        public static int selectedAGVID;
        double vantoc = 0;

        private void InitZedGraph()
        {
            #region Init Velocity graph
            GraphPane velocityPane = zedGraphVelocity.GraphPane;

            RollingPointPairList velocityPointBuffer = new RollingPointPairList(10000);
            LineItem velocityCurve = velocityPane.AddCurve("velocity", velocityPointBuffer, Color.Red, SymbolType.None);

            RollingPointPairList setPointBuffer = new RollingPointPairList(10000);
            LineItem setPointCurve = velocityPane.AddCurve("desired velocity", setPointBuffer, Color.Blue, SymbolType.None);

            // Add titles
            velocityPane.Title.Text = "Velocity of AGV";
            velocityPane.Title.FontSpec.FontColor = Color.Navy;
            velocityPane.XAxis.Title.Text = "t (s)";
            velocityPane.XAxis.Title.FontSpec.FontColor = Color.Navy;
            velocityPane.XAxis.Title.FontSpec.IsBold = false;
            velocityPane.YAxis.Title.Text = "velocity (cm/s)";
            velocityPane.YAxis.Title.FontSpec.FontColor = Color.Navy;
            velocityPane.YAxis.Title.FontSpec.IsBold = false;

            // Add gridlines to the plot, and make them gray
            velocityPane.XAxis.MajorGrid.IsVisible = true;
            velocityPane.YAxis.MajorGrid.IsVisible = true;
            velocityPane.XAxis.MajorGrid.Color = Color.Gray;
            velocityPane.YAxis.MajorGrid.Color = Color.Gray;

            // Set scale of axis
            velocityPane.XAxis.Scale.Min = 0;
            velocityPane.XAxis.Scale.Max = 30;
            velocityPane.XAxis.Scale.MinorStep = 1;
            velocityPane.XAxis.Scale.MajorStep = 5;

            // Move the legend location
            velocityPane.Legend.Position = LegendPos.InsideTopRight;
            velocityPane.Legend.Border.IsVisible = false;
            velocityPane.Legend.Fill.IsVisible = false;
            velocityPane.Legend.IsHStack = false;

            // Make both curves thicker
            velocityCurve.Line.Width = 2.0F;
            setPointCurve.Line.Width = 2.0F;

            //// Fill the area under the curves
            //velocityCurve.Line.Fill = new Fill(Color.White, Color.FromArgb(255, 175, 175), -90F);
            //setPointCurve.Line.Fill = new Fill(Color.White, Color.FromArgb(255, 175, 175), -90F);

            // Fill the Axis and Pane backgrounds
            velocityPane.Chart.Fill = new Fill(Color.White, Color.FromArgb(255, 255, 210), -45F);
            velocityPane.Fill = new Fill(Color.WhiteSmoke);
            velocityPane.Border.IsVisible = false;

            // Tell ZedGraph to refigure the axes since the data have changed
            zedGraphVelocity.AxisChange();
            #endregion

            #region Init Line Tracking graph
            GraphPane linetrackPane = zedGraphLineTrack.GraphPane;

            RollingPointPairList linetrackPointBuffer = new RollingPointPairList(10000);
            LineItem linetrackCurve = linetrackPane.AddCurve("error", linetrackPointBuffer, Color.Red, SymbolType.None);

            // Add titles
            linetrackPane.Title.Text = "Error of line tracking";
            linetrackPane.Title.FontSpec.FontColor = Color.Navy;
            linetrackPane.XAxis.Title.Text = "t (s)";
            linetrackPane.XAxis.Title.FontSpec.FontColor = Color.Navy;
            linetrackPane.XAxis.Title.FontSpec.IsBold = false;
            linetrackPane.YAxis.Title.Text = "Error";
            linetrackPane.YAxis.Title.FontSpec.FontColor = Color.Navy;
            linetrackPane.YAxis.Title.FontSpec.IsBold = false;

            // Add gridlines to the plot, and make them gray
            linetrackPane.XAxis.MajorGrid.IsVisible = true;
            linetrackPane.YAxis.MajorGrid.IsVisible = true;
            linetrackPane.XAxis.MajorGrid.Color = Color.Gray;
            linetrackPane.YAxis.MajorGrid.Color = Color.Gray;

            // Set scale of axis
            linetrackPane.XAxis.Scale.Min = 0;
            linetrackPane.XAxis.Scale.Max = 30;
            linetrackPane.XAxis.Scale.MinorStep = 1;
            linetrackPane.XAxis.Scale.MajorStep = 5;

            // Move the legend location
            linetrackPane.Legend.Position = LegendPos.InsideTopRight;
            linetrackPane.Legend.Border.IsVisible = false;
            linetrackPane.Legend.Fill.IsVisible = false;
            linetrackPane.Legend.IsHStack = false;

            // Make both curves thicker
            linetrackCurve.Line.Width = 2.0F;

            //// Fill the area under the curves
            //linetrackCurve.Line.Fill = new Fill(Color.White, Color.FromArgb(255, 175, 175), -90F);

            // Fill the Axis and Pane backgrounds
            linetrackPane.Chart.Fill = new Fill(Color.White, Color.FromArgb(255, 255, 210), -45F);
            linetrackPane.Fill = new Fill(Color.WhiteSmoke);
            linetrackPane.Border.IsVisible = false;

            // Tell ZedGraph to refigure the axes since the data have changed
            zedGraphLineTrack.AxisChange();
            #endregion

            tickStart = Environment.TickCount;
        }
        private void DrawGraph(ZedGraphControl zedGraphControl, double value)
        {
            if (zedGraphControl.GraphPane.CurveList.Count <= 0) return;

            // time is measure in seconds
            double time = (Environment.TickCount - tickStart) / 1000.0;

            LineItem curve = zedGraphControl.GraphPane.CurveList[0] as LineItem;
            IPointListEdit pointBuffer = curve.Points as IPointListEdit;
            // add point to buffer to draw
            pointBuffer.Add(time, value);
            
            if (zedGraphControl == zedGraphVelocity)
            {
                LineItem curve1 = zedGraphControl.GraphPane.CurveList[1] as LineItem;
                IPointListEdit pointBuffer1 = curve1.Points as IPointListEdit;
                // add point to buffer to draw
                if (Display.Mode == "Real Time") pointBuffer1.Add(time, 15);
                else if (Display.Mode == "Simulation") pointBuffer1.Add(time, 15);
            }

            // make xAxis scroll
            Scale xScale = zedGraphControl.GraphPane.XAxis.Scale;
            if (time > xScale.Max - xScale.MajorStep)
            {
                xScale.Max = time + xScale.MajorStep;
                xScale.Min = xScale.Max - 30.0;
            }

            // re-draw graph
            zedGraphControl.AxisChange();
            zedGraphControl.Invalidate();
        }
        private void button4_Click(object sender, EventArgs e)
        {
            Communication.SetLinePosPacket sendFrame = new Communication.SetLinePosPacket();

            sendFrame.Header = new byte[2] { 0xAA, 0xFF };
            sendFrame.FunctionCode = 0xAD;
            sendFrame.AGVID = 0x01;
            sendFrame.Kp = Convert.ToSingle("0.5");
            sendFrame.Ki = Convert.ToSingle("0.0005");
            sendFrame.Kd = Convert.ToSingle("0.05");
            sendFrame.CheckSum = new byte[2];
            //  sendFrame.unNecessary = 0;

            // calculate check sum
            //CRC16_Calculator(sendFrame.ToArrayCRC(), sendFrame.CheckSum);

            //sendFrame.CheckSum = crc;
            sendFrame.EndOfFrame = new byte[2] { 0x0A, 0x0D };
            if (!Communication.SerialPort.IsOpen) return;
            try { Communication.SerialPort.Write(sendFrame.ToArray(), 0, sendFrame.ToArray().Length); }
            catch { };
        }

        private void button5_Click(object sender, EventArgs e)
        {
            string fullpath1 = "N-0-N-55-N-46-E-47-E-48-E-49-S-53-G-N-0";
            //string fullpath = "N,0,S,11,N,3,W,0,S,42,E,46,G,N,0";
            Communication.SendPathData(fullpath1);
            //delay();
            
        }
        private static void delay()
        {
            System.Timers.Timer timer1 = new System.Timers.Timer(10000);
            timer1.Elapsed += (sender, e) => timer1_Elapsed(sender, e);

            timer1.Start();
        }
        private static void timer1_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            //string fullpath = "N,0,N,46,E,50,N,8,W,3,S,11,G,N,0";
            string fullpath = "P-2-S-53-N-49-W-44-N-37-G-D-3";
            Communication.SendPathData(fullpath);
            ((System.Timers.Timer)sender).Dispose();
        }

        
        private void button1_Click(object sender, EventArgs e)
        {
            Communication.SetSpeedPacket sendFrame = new Communication.SetSpeedPacket();

            sendFrame.Header = new byte[2] { 0xAA, 0xFF };
            sendFrame.CheckSum = new byte[2];
            sendFrame.FunctionCode = 0xAC;
            sendFrame.AGVID = 0x01;
            sendFrame.Kp = Convert.ToSingle("2.0");
            sendFrame.Ki = Convert.ToSingle("2.0");
            sendFrame.Kd = Convert.ToSingle("0.015");
            sendFrame.Velocity = Convert.ToSingle("15.0");
            // calculate check sum
            //sCRC16_Calculator(sendFrame.ToArrayCRC(), sendFrame.CheckSum);
            //   sendFrame.CheckSum = crc;           
            sendFrame.EndOfFrame = new byte[2] { 0x0A, 0x0D };
            if (!Communication.SerialPort.IsOpen) return;
            try { Communication.SerialPort.Write(sendFrame.ToArray(), 0, sendFrame.ToArray().Length); }
            catch { };
        }

        private void button3_Click(object sender, EventArgs e)
        {
            string fullpath1 = "P-1-S-53-N-49-W-48-W-47-W-46-N-25-N-4-W-3-S-11-G-D-2";
            //string fullpath = "N,0,S,11,N,3,W,0,S,42,E,46,G,N,0";
            Communication.SendPathData(fullpath1);
        }

        private void timerGraph_Tick(object sender, EventArgs e)
        {
            DrawGraph(zedGraphVelocity,Communication.speed);

            DrawGraph(zedGraphLineTrack, Communication.line);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            vantoc = vantoc + 5;
        }
    }
}
