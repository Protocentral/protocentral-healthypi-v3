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

class BPM
{

  float min, max;                                      // Stores Minimum and Maximum Value
  double threshold;                                    // Stores the threshold 
  float minimizedVolt[] = new float[pSize];            // Stores the absoulte values in the buffer
  int beats = 0, bpm = 0;                              // Variables to store the no.of peaks and bpm

   
      

  ////////////////////////////////////////////////////////////////////////////////////////////
  //  - Heart Value is calculated by:
  //          * Setting a threshold value which is between the minimum and maximum value
  //          * Calculating no.of peaks crossing, the threshold value.
  //          * Calculate the Heart rate with the no.of peaks achieved with the no.of seconds
  //   
  ////////////////////////////////////////////////////////////////////////////////////////////

      int[] QRS(float[] lowPass, int nsamp) 
      {
          int[] QRS = new int[nsamp];
   
          double treshold = 0;
   
          //先從所有值中找出最大值當Threshold
          for(int i=0; i<200; i++) {
              if(lowPass[i] > treshold) {
                  treshold = lowPass[i];
              }
          }
   
          int frame = 250; //window size 取前250個中最大的值當PEAK
          
          for(int i=0; i<lowPass.length; i+=frame) { //每250筆data算一次
              float max = 0;
              int index = 0;
              if(i + frame > lowPass.length) { //如果超過則為最後一個
                  index = lowPass.length;
              }
              else {
                  index = i + frame;
              }
              for(int j=i; j<index; j++) {
                  if(lowPass[j] > max) max = lowPass[j]; //250個data中的最大值
              }
              boolean added = false;
              for(int j=i; j<index; j++) {
                  if(lowPass[j] > treshold && !added) {
                      QRS[j] = 1; //找到R點，250個裡面就不再繼續找 (約0.5秒)
                            //若之後改成real time則frame可以改為1
                      added = true;
                  }
                  else {
                      QRS[j] = 0;
                  }
              }   
              double gama = (Math.random() > 0.5) ? 0.15 : 0.20;
              double alpha = 0.01 + (Math.random() * ((0.1 - 0.01)));
   
              treshold = alpha * gama * max + (1 - alpha) * treshold;   
          }
   
          return QRS;
      }
      
  void bpmCalc(float[] recVoltage)
  {

    int j = 0, n = 0, cntr = 0;

    // Making the array into absolute (positive values only)

    for (int i=0; i<pSize; i++)
    {
      recVoltage[i] = (float)Math.abs(recVoltage[i]);
    }

    j = 0;
    for (int i = 0; i < pSize; i++)
    {
      minimizedVolt[j++] = recVoltage[i];
    }
    
    // Calculating the minimum and maximum value
    
    min = min(minimizedVolt);
    max = max(minimizedVolt);

    if ((int)min == (int)max)
    {
      //lblHR.setText("Heart Rate: 0 bpm");
    } else
    {
      threshold = min+max;                                     // Calculating the threshold value
      threshold = (threshold) * 0.400;

      if (threshold != 0)
      {
        while (n < pSize)                                      // scan through ECG samples
        {
          if (minimizedVolt[n] > threshold)                    // ECG threshold crossed
          {
            beats++;
            n = n+30;                                          // skipping the some samples to avoid repeatation
          } else
            n++;
        }
        bpm = (beats*60)/8;

        //lblHR.setText("Heart Rate:" + bpm+" bpm");
        beats = 0;
        global_hr=bpm;
      } else
      {
        
        //lblHR.setText("Heart Rate:0 bpm");

      }
    }
  }
};