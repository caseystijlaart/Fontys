#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

// Function to generate a series of 100 random numbers
vector<double> generateRandomSeries(int size) {
  vector<double> series;
  for (int i = 0; i < size; ++i) {
    series.push_back(rand() % 100);  // Generate random numbers between 0 and 99
  }
  return series;
}

// Function to calculate Moving Average with length 10 using FIIR filter (Direct
// Approach)
vector<double> calculateFIIR_MA(const vector<double>& x) {
  vector<double> y(x.size(), 0.0);
  for (int i = 9; i < x.size(); ++i) {
    for (int j = 0; j < 10; ++j) {
      y[i] += x[i - j];
    }
    y[i] /= 10;
  }
  return y;
}

// Function to calculate Moving Average with length 10 using IIR filter
// (Alternative Approach)
vector<double> calculateIIR_MA(const vector<double>& x) {
  vector<double> y(x.size(), 0.0);
  double sum = 0.0;
  for (int i = 0; i < x.size(); ++i) {
    sum += x[i];
    if (i >= 10) {
      sum -= x[i - 10];
      y[i] = sum / 10.0;
    } else {
      y[i] = sum / (i + 1);
    }
  }
  return y;
}

// Function to write data into a text file
void writeDataToFile(const vector<double>& data, const string& filename) {
  ofstream outFile(filename);
  if (!outFile) {
    cerr << "Unable to open " << filename << " for writing." << endl;
    return;
  }
  for (int i = 0; i < data.size(); ++i) {
    outFile << data[i] << endl;
  }
  outFile.close();
  cout << "Data written to " << filename << endl;
}

int main() {
  srand(time(nullptr));  // Seed random number generator

  vector<double> x = generateRandomSeries(100);

  // Calculate Moving Averages using different methods
  vector<double> y1 = calculateFIIR_MA(x);
  vector<double> y2 = calculateIIR_MA(x);

  // Write data into text files for MATLAB
  writeDataToFile(y1, "y1.txt");
  writeDataToFile(y2, "y2.txt");

  return 0;
}
