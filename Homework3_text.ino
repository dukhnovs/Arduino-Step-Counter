#include <Adafruit_CircuitPlayground.h>
#include <math.h>

#define numel(x)  (sizeof(x) / sizeof(*x))
#define SIZE_ALL 100 // size of sliding window for mag
#define SIZE_SMOOTH 5 // size of sliding window for averaging mag
#define SIZE_SMOOTH_CENTER (SIZE_SMOOTH / 2 + 1) // center of sliding window for averaging mag

float X, Y, Z, mag, magMeanSmooth, magMeanAll, magSmoothDataPoint;
float slidingWindowAll[SIZE_ALL]; // sliding window of raw data
float slidingWindowSmooth[SIZE_SMOOTH]; // sliding window of last 5 raw data points for smoothing
float slidingWindowSaveSmooth[SIZE_SMOOTH]; // sliding window of last 5 smooth data points for finding peaks
int ii = 0; // iteration of loop()
boolean peak; // does current array of last 5 smooth data points have a peak in the center
boolean peakNeedsReset = false; // peak needs to be reset before detecting another peak
int peakCounter = 0; // if current array of last 5 smooth data points does have a peak in the center then increment peak count

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
}

void loop() {
  if (ii < 1) {
    delay(1000);
  }

  boolean leftButtonPressed = CircuitPlayground.leftButton();
  boolean rightButtonPressed = CircuitPlayground.rightButton();

  // reset count if left or right button is pressed
  if (leftButtonPressed || rightButtonPressed) {
    peakCounter = 0;
    peakNeedsReset = false;
  } 

  
  ii = ii + 1;
  X = CircuitPlayground.motionX();
  Y = CircuitPlayground.motionY();
  Z = CircuitPlayground.motionZ();
  mag = sqrt(X*X + Y*Y + Z*Z);
  addToWindow(slidingWindowAll,SIZE_ALL, mag);
  addToWindow(slidingWindowSmooth,SIZE_SMOOTH, mag);
  magMeanSmooth = getMean(slidingWindowSmooth, SIZE_SMOOTH);
  magMeanAll = getMean(slidingWindowAll, SIZE_ALL);
  magSmoothDataPoint = magMeanSmooth - magMeanAll; // get smoothed data point over last 5 samples and remove overall mean value to 0 center on horizontal axis

  if (resetPeak(magSmoothDataPoint, peakNeedsReset) == true) {
    peakNeedsReset = false; // if it's ok to reset the peak (via zero-crossing), then flag that a peak reset is not needed
  }
  
  addToWindow(slidingWindowSaveSmooth,SIZE_SMOOTH, magSmoothDataPoint); 
  if (peakNeedsReset == false) { // if a peak has not yet been detected (before resetting), then check if last 5 smooth values have a peak at the center
    peak = isPeak(slidingWindowSaveSmooth,SIZE_SMOOTH, peakNeedsReset); // check if peak occured, delayed by 2 samples
  } else {
    peak = false; // else peak is false because previous peak has not been reset
  }

  if (peak == true) {
    peakCounter = peakCounter + 1;
    peakNeedsReset = true; // don't count another peak until this one is reset (by zero crossing)
  }

  binaryPrint(peakCounter);
    
//  Serial.print(ii);
//  Serial.print(", ");//X=");
  
  Serial.print(peak);
  Serial.print(", ");//isPeak=");
  Serial.print(X);
  Serial.print(", ");//Y=");
  Serial.print(Y);
  Serial.print(", ");//Z=");
  Serial.print(Z);
  Serial.print(", ");//mag=");
  Serial.print(mag);
//  Serial.print(", ");//sWA=");
//  Serial.print(slidingWindowAll[SIZE_ALL-1]);
//  Serial.print(", ");//sWS=");
//  Serial.print(slidingWindowSmooth[SIZE_SMOOTH-1]);
  Serial.print(", ");
  Serial.print(peakCounter);
  Serial.print(", ");
  Serial.print(peakNeedsReset);
  Serial.print(", ");//mMS=");
  Serial.print(magMeanSmooth);
  Serial.print(", ");//mMA=");
  Serial.print(magMeanAll);
  Serial.print(", ");//mSDP=");
  Serial.print(magSmoothDataPoint);
  
  
  
//  Serial.print(", ");
//  Serial.print(SIZE_ALL);
//  Serial.print(", ");
//  Serial.print(SIZE_SMOOTH);
//  Serial.print(", ");
//  Serial.println(SIZE_SMOOTH_CENTER);
  Serial.println();
 
 
  delay(10);
}

// add 1 element to a sliding window array
void addToWindow(float *slidingWindow, int ssize, float x) {
//  Serial.print(" addToWindow ");
//  Serial.print(", ssize=");
//  Serial.print(ssize);
//  Serial.print(", x=");
//  Serial.print(x);
  
  for (int i = 1; i < ssize; i++) {
    slidingWindow[i -  1] = slidingWindow[i]; // shift old values 1 to the left, 0th element is deleted by being overwritten
  }
  slidingWindow[ssize - 1] = x; // last element in array set to new value
}

float getMean(float window[], int ssize) {
  float sum = 0.0;
//  Serial.print("[ ");
  for (int i = 0; i < ssize; i++) {
    sum = sum + window[i];
//    Serial.print(window[i]);
//    Serial.print(" ,");
  }
//  Serial.print(" ] sum = ");
  

  if (ii < ssize) { // not enough samples to fill the array so there are still 0 elements that should be ignored in the average
//    Serial.print(sum / (float) ii);
//    Serial.print(" | ");
    return sum / (float) ii; // divide the sum by the number of non-zero elements
  } else {
//    Serial.print(sum / (float) ssize);
//    Serial.print(" | ");
    return sum / (float) ssize; // divide the sum by the number of array elements
  }
}

// check if center array value is larger than values on both sides
boolean isPeak(float window[], int ssize, boolean isPeakReset) {
  peak = true; // start by assuming the middle point in the last 5 data points is a peak
  for (int i = 0; i < SIZE_SMOOTH_CENTER - 1; i++) { // check if numbers before peak are smaller than potenial peak (i = 0, 1) , (center is at i = 2, since 0-based array)
    if (window[i] > window[SIZE_SMOOTH_CENTER - 1]) { // if previous value larger than the center value, then the center is not the peak
      peak = false;
    }
  }
  for (int i = SIZE_SMOOTH_CENTER; i < SIZE_SMOOTH; i++) { // check if numbers after peak are smaller than potenial peak (i = 3, 4), (center is at i = 2, since 0-based array)
    if (window[i] > window[SIZE_SMOOTH_CENTER - 1]) { // if later value is larger than the center value, then the center is not the peak
      peak = false;
    }
  }

  if (window[SIZE_SMOOTH_CENTER - 1] < 1.0) {
    peak = false;
  }
  return peak;
}

boolean resetPeak(float currentSmoothDataPoint, boolean peakNeedsReset) {
  if (peakNeedsReset == true && currentSmoothDataPoint <= 0) {
    return true;
  } else {
    return false;
  }
}

void binaryPrint(int num) {
  int binaryNum[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // start with eight 0 bits, up to 255 max steps
  for (int i = 7; i >= 0; i--) {
    binaryNum[7-i] = bitRead(num, i);
    if (binaryNum[7-i] == 1) {
      CircuitPlayground.setPixelColor(i, 0,   0,   255); // blue
    } else {
      CircuitPlayground.setPixelColor(i, 0,   0,   0); // off
    }
//    Serial.print(bitRead(num, i) ? "-1-" : "-0-");
  }
  for(int i = 0; i < 8; i++) {
//    Serial.print(binaryNum[i]);
//    Serial.print("_");
    
  }
//  Serial.print("  ");
}

