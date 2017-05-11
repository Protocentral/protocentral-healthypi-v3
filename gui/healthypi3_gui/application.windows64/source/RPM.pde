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

class RPM
{
  float min, max;                                      // Stores Minimum and Maximum Value
  double threshold;                                    // Stores the threshold
  float minimizedVolt[] = new float[pSize];            // Stores the absoulte values in the buffer
  int peaks = 0, rpm1 = 0;                             // Variables to store the no.of peaks and bpm

  ////////////////////////////////////////////////////////////////////////////////////////////
  //  - Respiration Rate is calculated by:
  //          * Setting a threshold value which is between the minimum and maximum value
  //          * Calculating no.of peaks crossing, the threshold value.
  //   
  ////////////////////////////////////////////////////////////////////////////////////////////

  void rpmCalc(float[] recVoltage)
  {
    int j = 0, n = 0, cntr = 0;
    // Making the array into absolute (positive values only)
    for (int i=0; i<pSize; i++)
    {
      minimizedVolt[i] = (float)Math.abs(recVoltage[i]);
    }
    min = min(minimizedVolt);
    max = max(minimizedVolt);

    threshold = min+max;                      // Calculating the threshold value
    threshold = (threshold) * 0.400;

    if (threshold != 0)
    {
      while (n < pSize)                       // scan through ECG samples
      {
        if (recVoltage[n] > threshold)        // ECG threshold crossed
        {
          peaks++;
          n = n+30;                           // skipping the some samples to avoid repeatation
        } else
          n++;
      }
      lblRR.setText("Respiration: " + peaks+ " bpm");
      peaks = 0;
    } else
    {
      lblRR.setText("Respiration: 0 bpm");
    }
  }
};