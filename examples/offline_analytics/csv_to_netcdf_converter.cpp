// compile with
// g++ -std=c++11 -L/usr/local/netcdf/lib -I/usr/local/netcdf/inlcude -lnetcdf csv_to_netcdf_converter.cpp

#include <iostream>
#include <fstream>
#include <netcdf.h>
#include <string>
#include <ctype.h>


// TODO calculate automatically (lines / #commas+1)
#define NUM_ROWS	64
#define NUM_COLS	16


// Create a NetCDF file comprising a variable "point".
#define CSV_FILENAME "data.csv"
#define NETCDF_FILENAME "testConvertedCSV1.nc"

#define VARNAME "point"

using namespace std;

int main() {
  int numElements = NUM_ROWS * NUM_COLS;
  double *points = new double[numElements]; // Variable data.

  // open csv file to read
  ifstream myfile;
  myfile.open(CSV_FILENAME);
  
  if (myfile.is_open()) {
    string line;
    int row;
    int col;
    size_t nextElementIndex = 0;
    for (row = 0; row < NUM_ROWS; row++) {
      // get next line
      getline (myfile,line);
      int startNextNum = 0;
      int endNextNum = 0;
      while(endNextNum < line.length()){
        if( isdigit(line.at(endNextNum)) || line.at(endNextNum) == '-' || line.at(endNextNum) == '.') {
          endNextNum++;
        } else if ( startNextNum != endNextNum) {
          cout << line.substr(startNextNum, endNextNum - startNextNum) << endl;
          points[nextElementIndex++] = std::stod(line.substr(startNextNum, endNextNum - startNextNum));
          startNextNum = endNextNum; 
        } else {
          startNextNum++;
          endNextNum++;
        }
      }
      // if line ends with a number
      if ( startNextNum != endNextNum) {
        cout << line.substr(startNextNum, endNextNum - startNextNum) << endl;
        points[nextElementIndex++] = std::stod(line.substr(startNextNum, endNextNum - startNextNum));
        startNextNum = endNextNum;
      }       
    }
    myfile.close();
  }
  // Now that data has been read into points, generate netCDF

  int fid;  // File ID.
  int dimid[1] = {numElements};  // Dimension ID.
  int varid;  // Variable ID.
  // Define the file schema.
  nc_create(NETCDF_FILENAME, NC_64BIT_OFFSET, &fid);
  nc_def_dim(fid, "len", numElements, &dimid[0]);
  nc_def_var(fid, VARNAME, NC_DOUBLE, 1, dimid, &varid);
  nc_enddef(fid);
  cout << "The generated file contains " << numElements << " points." << endl;

  // Write data.
  nc_put_var_double(fid, varid, points);

  nc_close(fid);

  delete [] points;

  cout << "The file " << NETCDF_FILENAME << " has been created." << endl;
  cout << "To view this file, enter the command \"ncdump " << NETCDF_FILENAME << "\"." << endl;

  return 0;
}
