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

public class SPO2_cal
{
  float Vdc = 0;
  float Vac = 0;
  float spo2_cal_array[] = new float[pSize];

  float SPO2 = 0;

  ///////////////////////////////////////////////////////////////////////////
  //  
  //  To Calulate the Spo2 value any one of the following Emprical Formal are used:
  //    1. float SpO2 = 10.0002*(value)-52.887*(value) + 26.817*(value) + 98.293;
  //    2. float SpO2 =((0.81-0.18*(value))/(0.73+0.11*(value)));
  //    3. float SpO2=110-25*(value);
  //  In this Program, the 3rd formulae is used
  //
  //////////////////////////////////////////////////////////////////////////

  public void rawDataArray(float ir_array[], float red_array[], double ir_avg, double red_avg)
  {
    float RedAC = s.SPO2_Value(red_array);
    float IrAC = s.SPO2_Value(ir_array);
    float value = (RedAC/abs((float)red_avg))/(IrAC/abs((float)ir_avg));
    float SpO2=110-25*(value);
    SpO2 = (int)(SpO2 * 100);
    SpO2 = Math.round(SpO2/100);
    lblSPO2.setText("SpO2: "+ (SpO2+10)+" %");
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////
  //  SPo2 Value is calculated by:
  //    * Calculate the square of the spo2 values and store it in the buffer
  //    * Sum of the values in the squared buffer is calculated.
  //    * This sum is sent to the main function
  //   
  ////////////////////////////////////////////////////////////////////////////////////////////

  public float SPO2_Value(float spo2_array[])
  {
    SPO2 = 0;
    int k = 0;
    for (int i = 50; i < spo2_array.length; i++)
    {
      spo2_cal_array[k++] = spo2_array[i] * spo2_array[i];
    }
    SPO2 = sum(spo2_cal_array, k);
    return (SPO2);
  }

  public float sum(float array[], int len)
  {
    float spo2 = 0;
    for (int p = 0; p < len; p++)
    {
      spo2 = spo2 + array[p];
    }
    Vac = (float)Math.sqrt(spo2/len);
    return Vac;
  }
}