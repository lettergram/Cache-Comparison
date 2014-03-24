/**
 * File: main.cpp
 * Description: Manipulates and image.
 *
 * Created: 01/19/2013
 */

#include <iostream>
#include <fstream>
#include <chrono>

#include "opencl.h"


using namespace std;


int main(){
    
    ofstream myfile;
    myfile.open("cl_speed_comparison.txt");
    std::chrono::time_point<std::chrono::system_clock> start, end;
    
    bool success = true;
    long int size = 4098;
    
    while(size > 0){
        int * inputmatrix = new int [size];
    
        for(int x = 0; x < size; x++){
            inputmatrix[x] = 1;
        }
    
        cl matrix;
        
        matrix.createQueue(inputmatrix, size);
       
        start = std::chrono::system_clock::now();
        success = matrix.processMatrix();
        end = std::chrono::system_clock::now();
    
        std::chrono::duration<double> elapsed_seconds = end-start;
    
        myfile << size << ",";
        myfile << elapsed_seconds.count() << ",";
        myfile << success << endl;
        size = size >> 1;
    }
    myfile.close();
  
  return 0;
}
