//////////////////////////////////////////////////////////////////////////////////////////
//
//   Desktop GUI for controlling the HealthyPi HAT [ Patient Monitoring System]
//
//   Copyright (c) 2016 ProtoCentral
//   
//   This software is licensed under the MIT License(http://opensource.org/licenses/MIT). 
//   
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT 
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
//   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
//   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
//   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//   Requires g4p_control graphing library for processing.  Built on V4.1
//   Downloaded from Processing IDE Sketch->Import Library->Add Library->G4P Install
//
/////////////////////////////////////////////////////////////////////////////////////////

import g4p_controls.*;                       // Processing GUI Library to create buttons, dropdown,etc.,
import processing.serial.*;                  // Serial Library
import grafica.*;

// Java Swing Package For prompting message
import java.awt.*;
import javax.swing.*;
import static javax.swing.JOptionPane.*;

// File Packages to record the data into a text file
import javax.swing.JFileChooser;
import java.io.FileWriter;
import java.io.BufferedWriter;

// Date Format
import java.util.*;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import processing.net.*;

// General Java Package
import java.math.*;
import controlP5.*;
import http.requests.*;

ControlP5 cp5;

Textlabel lblHR;
Textlabel lblSPO2;
Textlabel lblRR;
Textlabel lblBP;
Textlabel lblTemp;
Textlabel lblstatus;

Client myClient; 


/************** Packet Validation  **********************/
private static final int CESState_Init = 0;
private static final int CESState_SOF1_Found = 1;
private static final int CESState_SOF2_Found = 2;
private static final int CESState_PktLen_Found = 3;

/*CES CMD IF Packet Format*/
private static final int CES_CMDIF_PKT_START_1 = 0x0A;
private static final int CES_CMDIF_PKT_START_2 = 0xFA;
private static final int CES_CMDIF_PKT_STOP = 0x0B;

/*CES CMD IF Packet Indices*/
private static final int CES_CMDIF_IND_LEN = 2;
private static final int CES_CMDIF_IND_LEN_MSB = 3;
private static final int CES_CMDIF_IND_PKTTYPE = 4;
private static int CES_CMDIF_PKT_OVERHEAD = 5;

/************** Packet Related Variables **********************/

int ecs_rx_state = 0;                                        // To check the state of the packet
int CES_Pkt_Len;                                             // To store the Packet Length Deatils
int CES_Pkt_Pos_Counter, CES_Data_Counter;                   // Packet and data counter
int CES_Pkt_PktType;                                         // To store the Packet Type
char CES_Pkt_Data_Counter[] = new char[1000];                // Buffer to store the data from the packet
char CES_Pkt_ECG_Counter[] = new char[4];                    // Buffer to hold ECG data
char CES_Pkt_Resp_Counter[] = new char[4];                   // Respiration Buffer
char CES_Pkt_SpO2_Counter_RED[] = new char[4];               // Buffer for SpO2 RED
char CES_Pkt_SpO2_Counter_IR[] = new char[4];                // Buffer for SpO2 IR
int pSize = 1000;                                            // Total Size of the buffer
int arrayIndex = 0;                                          // Increment Variable for the buffer
float time = 0;                                              // X axis increment variable

// Buffer for ecg,spo2,respiration,and average of thos values
float[] xdata = new float[pSize];
float[] ecgdata = new float[pSize];
float[] respdata = new float[pSize];
float[] bpmArray = new float[pSize];
float[] ecg_avg = new float[pSize];                          
float[] resp_avg = new float[pSize];
float[] spo2data = new float[pSize];
float[] spo2Array_IR = new float[pSize];
float[] spo2Array_RED = new float[pSize];
float[] rpmArray = new float[pSize];
float[] ppgArray = new float[pSize];

/************** User Defined Class Objects **********************/


/************** Graph Related Variables **********************/

double maxe, mine, maxr, minr, maxs, mins;             // To Calculate the Minimum and Maximum of the Buffer
double ecg, resp, spo2_ir, spo2_red, spo2, redAvg, irAvg, ecgAvg, resAvg;  // To store the current ecg value
double respirationVoltage=20;                          // To store the current respiration value
boolean startPlot = false;                             // Conditional Variable to start and stop the plot

GPlot plotPPG;
GPlot plotECG;
GPlot plotResp;

int step = 0;
int stepsPerCycle = 100;
int lastStepTime = 0;
boolean clockwise = true;
float scale = 5;

/************** File Related Variables **********************/

boolean logging = false;                                // Variable to check whether to record the data or not
FileWriter output;                                      // In-built writer class object to write the data to file
JFileChooser jFileChooser;                              // Helps to choose particular folder to save the file
Date date;                                              // Variables to record the date related values                              
BufferedWriter bufferedWriter;
DateFormat dateFormat;

/************** Port Related Variables **********************/

Serial port = null;                                     // Oject for communicating via serial port
String[] comList;                                       // Buffer that holds the serial ports that are paired to the laptop
char inString = '\0';                                   // To receive the bytes from the packet
String selectedPort;                                    // Holds the selected port number

String dispPlatform;
/************** Logo Related Variables **********************/

PImage logo;
boolean gStatus;                                        // Boolean variable to save the grid visibility status

int nPoints1 = pSize;
int totalPlotsHeight=0;
int totalPlotsWidth=0;
int heightHeader=50;
int updateCounter=0;

boolean is_raspberrypi=true;

long  thingSpeakPostTime = 0;
import mqtt.*;

int global_hr;
int global_rr;
float global_temp;
int global_spo2;

MQTTClient client;

int global_test=0;

void updateMQTT()
{
   if(millis() > thingSpeakPostTime)
   {
      /*
        // Prepare a JSON payload string
      String payload = "{";
      //payload += "\"heartrate\":\"" +global_hr+ "\",\"rr\":\"" + global_rr +"\"";
      payload += "heartrate:" +global_hr+ ",rr:" + global_rr +"";
      
      //payload += ",\"spo2\":\"" +global_spo2+ "\",\"temperature\":\"" + global_temp +"\"";

      payload += "}";
    
      client.publish( "v1/devices/me/telemetry", payload );
      //client.publish( "akw/feeds/healthypi-hr", ""+ global_test +"");//payload );//"test");// 
      println(payload);
      global_test++;
      thingSpeakPostTime = millis()+ 5000;
      */
      /*
      PostRequest post = new PostRequest("https://groker.initialstate.com/api/events?accessKey=hEAEGKCJXAfxoqoeuh7DaumZuyZvOPlJ&bucketKey=VLFRQWYQEVF5");
      post.addData("heartrate", str(global_hr) );
      post.addData("rr", str(global_rr));
      post.addData("spo2", str(global_spo2));
      post.addData("temperature", str(global_temp));
      
      post.send();
      //System.out.println("Reponse Content: " + post.getContent());
      //System.out.println("Reponse Content-Length Header: " + post.getHeader("Content-Length"));

      thingSpeakPostTime = millis()+ 5000;    
      */
   }
}   

void initMQTT()
{
    client = new MQTTClient(this);
    client.connect("mqtt://pcEuAxu7ZQ6wMFevblhh@ec2-54-255-214-27.ap-southeast-1.compute.amazonaws.com/", "healthypi3");
    //client.connect("mqtt://akw:4746bf5b83de4d5db4e4c03ad8b304cd@io.adafruit.com", "");
    
    client.subscribe("akw/feeds/healthypi");
}

public void setup() 
{
  
  dispPlatform = System.getProperty("os.name") + " " + System.getProperty("os.arch");
  println(System.getProperty("os.name"));
  
  if(System.getProperty("os.arch")=="arm")
  {
    is_raspberrypi=true;
  }
  
  println(System.getProperty("os.arch"));
  
  GPointsArray pointsPPG = new GPointsArray(nPoints1);
  GPointsArray pointsECG = new GPointsArray(nPoints1);
  GPointsArray pointsResp = new GPointsArray(nPoints1);

  //size(800, 480, JAVA2D);
  fullScreen();
   
  // ch
  heightHeader=50;
  println("Height:"+height);

  totalPlotsHeight=height-heightHeader;
  
  makeGUI();
  
  plotECG = new GPlot(this);
  plotECG.setPos(0,0);
  plotECG.setDim(width, (totalPlotsHeight/3)-10);
  plotECG.setBgColor(0);
  plotECG.setBoxBgColor(0);
  plotECG.setLineColor(color(0, 255, 0));
  plotECG.setLineWidth(3);
  plotECG.setMar(0,0,0,0);
  
  plotPPG = new GPlot(this);
  plotPPG.setPos(0,(totalPlotsHeight/3+10));
  plotPPG.setDim(width, (totalPlotsHeight/3)-10);
  plotPPG.setBgColor(0);
  plotPPG.setBoxBgColor(0);
  plotPPG.setLineColor(color(255, 255, 0));
  plotPPG.setLineWidth(3);
  plotPPG.setMar(0,0,0,0);

  plotResp = new GPlot(this);
  plotResp.setPos(0,(totalPlotsHeight/3+totalPlotsHeight/3+10));
  plotResp.setDim(width, (totalPlotsHeight/3)-10);
  plotResp.setBgColor(0);
  plotResp.setBoxBgColor(0);
  plotResp.setLineColor(color(0,0,255));
  plotResp.setLineWidth(3);
  plotResp.setMar(0,0,0,0);

  for (int i = 0; i < nPoints1; i++) 
  {
    pointsPPG.add(i,0);
    pointsECG.add(i,0);
    pointsResp.add(i,0); 
  }

  plotECG.setPoints(pointsECG);
  plotPPG.setPoints(pointsPPG);
  plotResp.setPoints(pointsPPG);

  /*******  Initializing zero for buffer ****************/

  for (int i=0; i<pSize; i++) 
  {
    time = time + 1;
    xdata[i]=time;
    ecgdata[i] = 0;
    respdata[i] = 0;
    ppgArray[i] = 0;
  }
  time = 0;
  
  if(true==is_raspberrypi)
  {
    //startSerial();
  }
  
  startPlot = true;
  //initMQTT();
}

public void makeGUI()
{
    cp5 = new ControlP5(this);
   
   cp5.addButton("Close")
     .setValue(0)
     .setPosition(110,height-heightHeader)
     .setSize(100,40)
     .setFont(createFont("Impact",15))
     .addCallback(new CallbackListener() {
      public void controlEvent(CallbackEvent event) {
        if (event.getAction() == ControlP5.ACTION_RELEASED) 
        {
          CloseApp();
          //cp5.remove(event.getController().getName());
        }
      }
     } 
     );
     
      cp5.addTextfield("devicename")
     .setPosition(215,height-heightHeader)
     .setSize(200,20)
     .setFont(createFont("Impact",15))
     .setAutoClear(false)
     .setText("healthypi.local")
     ;
     
      cp5.addButton("Connect")
     .setValue(0)
     .setPosition(420,height-heightHeader)
     .setSize(100,40)
     .setFont(createFont("Impact",15))
     .addCallback(new CallbackListener() {
      public void controlEvent(CallbackEvent event) {
        if (event.getAction() == ControlP5.ACTION_RELEASED) 
        {
            openConnect();
        }
      }
     } 
     );
     
     lblstatus = cp5.addTextlabel("lblstatus")
      .setText("Status: Not Connected")
      .setPosition((width/3)+100,height-60)
      .setColorValue(color(255,255,255))
      .setFont(createFont("Verdana",16));

     cp5.addTextlabel("lblplatform")
      .setText("Client Platform: "+ dispPlatform)
      .setPosition((width/3)+100,height-30)
      .setColorValue(color(255,255,255))
      .setFont(createFont("Verdana",14));

   cp5.addButton("Record")
     .setValue(0)
     .setPosition(5,height-heightHeader)
     .setSize(100,40)
     .setFont(createFont("Impact",15))
     .addCallback(new CallbackListener() {
      public void controlEvent(CallbackEvent event) {
        if (event.getAction() == ControlP5.ACTION_RELEASED) 
        {
          RecordData();
          //cp5.remove(event.getController().getName());
        }
      }
     } 
     );
    
  //List l = Arrays.asList("a", "b", "c", "d", "e", "f", "g", "h");
  /* add a ScrollableList, by default it behaves like a DropdownList */
  /*
  cp5.addScrollableList("dropdownPorts")
     .setPosition(100, 100)
     .setSize(200, 100)
     .setBarHeight(20)
     .setItemHeight(20)
     .addItems(port.list())
     .setType(ScrollableList.DROPDOWN) // currently supported DROPDOWN and LIST
     ;
    */ 
    
   lblHR = cp5.addTextlabel("lblHR")
      .setText("Heartrate: --- bpm")
      .setPosition(width-350,5)
      .setColorValue(color(255,255,255))
      .setFont(createFont("Impact",40));
      
    lblSPO2 = cp5.addTextlabel("lblSPO2")
    .setText("SpO2: 95 %")
    .setPosition(width-350,(totalPlotsHeight/3+10))
    .setColorValue(color(255,255,255))
    .setFont(createFont("Impact",40));

    lblRR = cp5.addTextlabel("lblRR")
    .setText("Respiration: --- bpm")
    .setPosition(width-350,(totalPlotsHeight/3+totalPlotsHeight/3+10))
    .setColorValue(color(255,255,255))
    .setFont(createFont("Impact",40));
   
    /*
    lblBP = cp5.addTextlabel("lblBP")
      .setText("BP: --- / ---")
      .setPosition((width-250),height-25)
      .setColorValue(color(255,255,255))
      .setFont(createFont("Verdana",20));
    */
    
    lblTemp = cp5.addTextlabel("lblTemp")
      .setText("Temperature: --- \u00B0 C")
      .setPosition((width/3)*2,height-70)
      .setColorValue(color(255,255,255))
      .setFont(createFont("Verdana",40));
      
     cp5.addButton("logo")
     .setPosition(5,5)
     .setImages(loadImage("protocentral.png"), loadImage("protocentral.png"), loadImage("protocentral.png"))
     .updateSize();
}

public void openConnect()
{
    myClient = new Client(this,cp5.get(Textfield.class,"devicename").getText(), 7777);   //heartypatch.local
    lblstatus.setText("Status: Connected to " + cp5.get(Textfield.class,"devicename").getText());
    startPlot = true;
}

public void draw() 
{
  background(0);

  GPointsArray pointsPPG = new GPointsArray(nPoints1);
  GPointsArray pointsECG = new GPointsArray(nPoints1);
  GPointsArray pointsResp = new GPointsArray(nPoints1);

  if (startPlot)                             // If the condition is true, then the plotting is done
  {
    for(int i=0; i<nPoints1;i++)
    {    
      pointsECG.add(i,ecgdata[i]);
      pointsPPG.add(i,spo2data[i]); 
      pointsResp.add(i,respdata[i]);  
    }
  } 
  else                                     // Default value is set
  {
  }

  plotECG.setPoints(pointsECG);
  plotPPG.setPoints(pointsPPG);
  plotResp.setPoints(pointsResp);
  
  plotECG.beginDraw();
  plotECG.drawBackground();
  plotECG.drawLines();
  plotECG.endDraw();
  
  plotPPG.beginDraw();
  plotPPG.drawBackground();
  plotPPG.drawLines();
  plotPPG.endDraw();

  plotResp.beginDraw();
  plotResp.drawBackground();
  plotResp.drawLines();
  plotResp.endDraw();

  //updateMQTT();
}

public void CloseApp() 
{
  int dialogResult = JOptionPane.showConfirmDialog (null, "Would You Like to Close The Application?");
  if (dialogResult == JOptionPane.YES_OPTION) {
    try
    {
      //Runtime runtime = Runtime.getRuntime();
      //Process proc = runtime.exec("sudo shutdown -h now");
      System.exit(0);
    }
    catch(Exception e)
    {
      exit();
    }
  } else
  {
  }
}

public void RecordData()
{
    try
  {
    jFileChooser = new JFileChooser();
    jFileChooser.setSelectedFile(new File("log.csv"));
    jFileChooser.showSaveDialog(null);
    String filePath = jFileChooser.getSelectedFile()+"";

    if ((filePath.equals("log.txt"))||(filePath.equals("null")))
    {
    } else
    {    
      logging = true;
      date = new Date();
      output = new FileWriter(jFileChooser.getSelectedFile(), true);
      bufferedWriter = new BufferedWriter(output);
      bufferedWriter.write(date.toString()+"");
      bufferedWriter.newLine();
      bufferedWriter.write("TimeStamp,ECG,SpO2,Respiration");
      bufferedWriter.newLine();
    }
  }
  catch(Exception e)
  {
    println("File Not Found");
  }
}
void startSerial()
{
  try
  {
    port = new Serial(this, "/dev/cu.usbmodem1441", 115200);
    port.clear();
    startPlot = true;
  }
  catch(Exception e)
  {

    showMessageDialog(null, "Port is busy", "Alert", ERROR_MESSAGE);
    System.exit (0);
  }
}

///////////////////////////////////////////////////////////////////
//
//  Event Handler To Read the packets received from the Device
//
//////////////////////////////////////////////////////////////////

void serialEvent (Serial blePort) 
{
  inString = blePort.readChar();
  ecsProcessData(inString);
}

void clientEvent(Client someClient) 
{
  inString = myClient.readChar();
  ecsProcessData(inString);
  //println(inString);
}

void ecsProcessData(char rxch)
{
  switch(ecs_rx_state)
  {
  case CESState_Init:
    if (rxch==CES_CMDIF_PKT_START_1)
      ecs_rx_state=CESState_SOF1_Found;
    break;

  case CESState_SOF1_Found:
    if (rxch==CES_CMDIF_PKT_START_2)
      ecs_rx_state=CESState_SOF2_Found;
    else
      ecs_rx_state=CESState_Init;                    //Invalid Packet, reset state to init
    break;

  case CESState_SOF2_Found:
    //    println("inside 3");
    ecs_rx_state = CESState_PktLen_Found;
    CES_Pkt_Len = (int) rxch;
    CES_Pkt_Pos_Counter = CES_CMDIF_IND_LEN;
    CES_Data_Counter = 0;
    break;

  case CESState_PktLen_Found:
    //    println("inside 4");
    CES_Pkt_Pos_Counter++;
    if (CES_Pkt_Pos_Counter < CES_CMDIF_PKT_OVERHEAD)  //Read Header
    {
      if (CES_Pkt_Pos_Counter==CES_CMDIF_IND_LEN_MSB)
        CES_Pkt_Len = (int) ((rxch<<8)|CES_Pkt_Len);
      else if (CES_Pkt_Pos_Counter==CES_CMDIF_IND_PKTTYPE)
        CES_Pkt_PktType = (int) rxch;
    } else if ( (CES_Pkt_Pos_Counter >= CES_CMDIF_PKT_OVERHEAD) && (CES_Pkt_Pos_Counter < CES_CMDIF_PKT_OVERHEAD+CES_Pkt_Len+1) )  //Read Data
    {
      if (CES_Pkt_PktType == 2)
      {
        CES_Pkt_Data_Counter[CES_Data_Counter++] = (char) (rxch);          // Buffer that assigns the data separated from the packet
      }
    } else  //All  and data received
    {
      if (rxch==CES_CMDIF_PKT_STOP)
      {     
        // The Buffer is splitted and assigned separatly in different buffer
        // First 4 bytes for ECG
        // Next 4 bytes for REspiration
        // Next 8 bytes for SpO2/PPG

        CES_Pkt_ECG_Counter[0] = CES_Pkt_Data_Counter[0];
        CES_Pkt_ECG_Counter[1] = CES_Pkt_Data_Counter[1];
        CES_Pkt_ECG_Counter[2] = CES_Pkt_Data_Counter[2];
        CES_Pkt_ECG_Counter[3] = CES_Pkt_Data_Counter[3];

        CES_Pkt_Resp_Counter[0] = CES_Pkt_Data_Counter[4];
        CES_Pkt_Resp_Counter[1] = CES_Pkt_Data_Counter[5];
        CES_Pkt_Resp_Counter[2] = CES_Pkt_Data_Counter[6];
        CES_Pkt_Resp_Counter[3] = CES_Pkt_Data_Counter[7];

        CES_Pkt_SpO2_Counter_IR[0] = CES_Pkt_Data_Counter[8];
        CES_Pkt_SpO2_Counter_IR[1] = CES_Pkt_Data_Counter[9];
        CES_Pkt_SpO2_Counter_IR[2] = CES_Pkt_Data_Counter[10];
        CES_Pkt_SpO2_Counter_IR[3] = CES_Pkt_Data_Counter[11];

        CES_Pkt_SpO2_Counter_RED[0] = CES_Pkt_Data_Counter[12];
        CES_Pkt_SpO2_Counter_RED[1] = CES_Pkt_Data_Counter[13];
        CES_Pkt_SpO2_Counter_RED[2] = CES_Pkt_Data_Counter[14];
        CES_Pkt_SpO2_Counter_RED[3] = CES_Pkt_Data_Counter[15];

        float Temp_Value = (float) ((int) CES_Pkt_Data_Counter[16]| CES_Pkt_Data_Counter[17]<<8)/100;                // Temperature
        // BP Value Systolic and Diastolic
        int global_RespirationRate = (int) (CES_Pkt_Data_Counter[18]);
        int global_HeartRate = (int) (CES_Pkt_Data_Counter[20]);
        //int BP_Value_Sys = (int) CES_Pkt_Data_Counter[17];
        //int BP_Value_Dia = (int) CES_Pkt_Data_Counter[18];

        int data1 = ecsParsePacket(CES_Pkt_ECG_Counter, CES_Pkt_ECG_Counter.length-1);
        ecg = (double) data1/(Math.pow(10, 3));

        int data2 = ecsParsePacket(CES_Pkt_Resp_Counter, CES_Pkt_Resp_Counter.length-1);
        resp = (double) data2/(Math.pow(10, 3));

        int data3 = ecsParsePacket(CES_Pkt_SpO2_Counter_IR, CES_Pkt_SpO2_Counter_IR.length-1);
        spo2_ir = (double) data3;

        int data4 = ecsParsePacket(CES_Pkt_SpO2_Counter_RED, CES_Pkt_SpO2_Counter_RED.length-1);
        spo2_red = (double) data4;

        ecg_avg[arrayIndex] = (float)ecg;
        ecgAvg = averageValue(ecg_avg);
        ecg = (ecg_avg[arrayIndex] - ecgAvg);

        spo2Array_IR[arrayIndex] = (float)spo2_ir;
        spo2Array_RED[arrayIndex] = (float)spo2_red;
        redAvg = averageValue(spo2Array_RED);
        irAvg = averageValue(spo2Array_IR);
        spo2 = (spo2Array_IR[arrayIndex] - irAvg);

        resp_avg[arrayIndex]= (float)resp;
        resAvg =  averageValue(resp_avg);
        resp = (resp_avg[arrayIndex] - resAvg);

        // Assigning the values for the graph buffers

        time = time+1;
        xdata[arrayIndex] = time;

        ecgdata[arrayIndex] = (float)ecg;
        respdata[arrayIndex]= (float)resp;
        spo2data[arrayIndex] = (float)spo2;

        lblRR.setText("Respiration: " + global_RespirationRate+ " rpm");
        lblHR.setText("Heart Rate: " + global_HeartRate + " bpm");
        
        arrayIndex++;
        updateCounter++;

 
        if(updateCounter==100)
        {
          if (startPlot)
          {

            global_temp=Temp_Value;
            lblTemp.setText("Temperature: "+Temp_Value+"\u00B0 C");
            
          }
          updateCounter=0;
        }
        
        if (arrayIndex == pSize)
        {  
          arrayIndex = 0;
          time = 0;
        }       

        // If record button is clicked, then logging is done

        if (logging == true)
        {
          try {
            date = new Date();
            dateFormat = new SimpleDateFormat("HH:mm:ss");
            bufferedWriter.write(dateFormat.format(date)+","+ecg+","+spo2+","+resp);
            bufferedWriter.newLine();
          }
          catch(IOException e) {
            println("It broke!!!");
            e.printStackTrace();
          }
        }
        ecs_rx_state=CESState_Init;
      } else
      {
        ecs_rx_state=CESState_Init;
      }
    }
    break;

  default:
    break;
  }
}

/*********************************************** Recursive Function To Reverse The data *********************************************************/

public int ecsParsePacket(char DataRcvPacket[], int n)
{
  if (n == 0)
    return (int) DataRcvPacket[n]<<(n*8);
  else
    return (DataRcvPacket[n]<<(n*8))| ecsParsePacket(DataRcvPacket, n-1);
}

/*************** Function to Calculate Average *********************/

double averageValue(float dataArray[])
{

  float total = 0;
  for (int i=0; i<dataArray.length; i++)
  {
    total = total + dataArray[i];
  }
  return total/dataArray.length;
}