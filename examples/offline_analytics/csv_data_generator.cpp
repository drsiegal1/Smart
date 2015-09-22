#include <iostream>
#include <fstream>

#define FILENAME "data.csv"
#define NUM_ROWS	64
#define NUM_COLS	16


using namespace std;

int main() {
  ofstream myfile;
  myfile.open(FILENAME);

  if (myfile.is_open()) {
    int row;
    int col;
    for (row = 0; row < NUM_ROWS; row++) {
      for (col = 0; col < NUM_COLS - 1; col++) {
        myfile << row*NUM_COLS + col <<", ";
      }
      myfile << row*NUM_COLS + col << endl;
    }
    cout << "The file " << FILENAME << " has been created." << endl;
    myfile.close();
  } else {
    cout << "Unable to open file";
  }
  return 0;
}
