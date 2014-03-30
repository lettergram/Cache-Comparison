#include <algorithm>
#include <iostream>
#include <fstream>
#include <omp.h>

using namespace std;

double bad_base(int ** matrix, long int size){
  
  double total_begin = omp_get_wtime();
  
  for(int x = 0; x < size; x++){
    for(int y = 0; y < size; y++){
      matrix[y][x] = matrix[y][x] * 2;
    }
  }
  double total_end = omp_get_wtime();
  return double(total_end - total_begin);
}

double good_base(int ** matrix, long int size){
  
  double total_begin = omp_get_wtime();
  
  for(int x = 0; x < size; x++){
    for(int y = 0; y < size; y++){
      matrix[x][y] = matrix[x][y] * 2;
    }
  }
  
  double total_end = omp_get_wtime();
  return double(total_end - total_begin);
}

double cache_opt(int ** matrix, long int size, int blocksize){
  
  double total_begin = omp_get_wtime();
  
  for(int x = 0; x < size; x += blocksize){
    for(int i = x; i < x + blocksize; i++){
      for(int y = 0; y < size; y+= blocksize){
        __builtin_prefetch(&matrix[i][y + blocksize]);
        for(int j = y; j < y + blocksize; j++){
          matrix[i][j] = matrix[i][j] * 2;
        }
      }
    }
  }
  
  double total_end = omp_get_wtime();
  return double(total_end - total_begin);
}

double good_openmp_cache_opt(int ** matrix, long int size, int blocksize){
  
  double total_begin = omp_get_wtime();
  
#pragma omp parallel for
  for(int x = 0; x < size; x++){
    for(int y = 0; y < size; y++){
      matrix[x][y] = matrix[x][y] * 2;
    }
  }
  
  double total_end = omp_get_wtime();
  return double(total_end - total_begin);
  
}

double bad_openmp_cache_opt(int ** matrix, long int size, int blocksize){
  
  double total_begin = omp_get_wtime();
  
  for(int x = 0; x < size; x++){
    if(x + 1 < size){ __builtin_prefetch(matrix[x+1]); }
    #pragma omp parallel for
    for(int y = 0; y < size; y++){
      matrix[x][y] = matrix[x][y] * 2;
    }
  }
  
  double total_end = omp_get_wtime();
  return double(total_end - total_begin);
}

double thrash_openmp(int ** matrix, long int size){
  
  double total_begin = omp_get_wtime();
  
#pragma omp parallel for
  for(int x = 0; x < size; x++){
    for(int y = 0; y < size; y++){
      matrix[y][x] = matrix[y][x] * 2;
    }
  }
  double total_end = omp_get_wtime();
  return double(total_end - total_begin);
}

double possible_thrash_openmp(int ** matrix, long int size){
  
  double total_begin = omp_get_wtime();
  
  #pragma omp parallel for schedule(dynamic)
  for(int x = 0; x < size; x++){
    for(int y = 0; y < size; y++){
      matrix[x][y] = matrix[x][y] * 2;
    }
  }
  double total_end = omp_get_wtime();
  return double(total_end - total_begin);
}

void cache(){
  
  long int size = 32768;
  size = 2048;
  
  ofstream myfile;
  myfile.open("speed_comparison.txt");
  
  myfile << "Matrix Size,";
  myfile << "Poor Base Case,";
  myfile << "Proper Base Case,";
  myfile << "'Supposed' Cache Optimized,";
  myfile << "Poor OpenMP,";
  myfile << "Thrashing OpenMP,";
  myfile << "Correct OpenMP" << endl;
  
  while(size > 16){
    
    int blocksize = 1024;
    while(blocksize % size == 0){ blocksize = blocksize >> 1; }
    blocksize = blocksize >> 1;
    
    int ** matrix;
    matrix = new int* [size];
    
    for(long int i = 0; i < size; i++)
      matrix[i] = new int[size];
    
    for(int x = 0; x < size; x++){
      for(int y = 0; y < size; y++){
        matrix[x][y] = 1;
      }
    }
    
    double avg_good_base = 0;
    double avg_bad_base = 0;
    double avg_cache = 0;
    double avg_good_mp = 0;
    double avg_bad_mp = 0;
    double avg_p_thrash_mp = 0;
    double avg_thrash_mp = 0;
    
    double num = 5.0;
    
    for(int i = 0; i < num; i++){
      avg_bad_base += bad_base(matrix, size);
      avg_good_base += good_base(matrix, size);
      avg_cache += cache_opt(matrix, size, blocksize);
      avg_bad_mp += bad_openmp_cache_opt(matrix, size, blocksize);
      avg_thrash_mp += thrash_openmp(matrix, size);
      avg_good_mp += good_openmp_cache_opt(matrix, size, blocksize);
      avg_p_thrash_mp += possible_thrash_openmp(matrix, size);
    }
    
    cout << "\nmatrix size: " << size << " x " << size << endl;
    myfile << size << ",";
    myfile << avg_bad_base / num << ",";
    myfile << avg_good_base / num << ",";
    myfile << avg_cache / num << ",";
    
    myfile << avg_p_thrash_mp / num << ",";
    myfile << avg_thrash_mp / num << ",";
    myfile << avg_bad_mp / num << ",";
    myfile << avg_good_mp / num << endl;
    
    size = size >> 1;
    
  }
  
  myfile.close();
}

void openmp(){
  
  long int size = 32768;
  int min = 2048;
  
  ofstream myfile;
  myfile.open("mp_comparison.txt");
  
  myfile << "Matrix Size,";
  myfile << "Poor OpenMP,";
  myfile << "Thrashing OpenMP,";
  myfile << "'Possible' Thrashing OpenMP,";
  myfile << "Correct OpenMP" << endl;
  
  while(size > min){
    
    int blocksize = 1024;
    while(blocksize % size == 0){ blocksize = blocksize >> 1; }
    blocksize = blocksize >> 1;
    
    int ** matrix;
    matrix = new int* [size];
    
    for(long int i = 0; i < size; i++)
      matrix[i] = new int[size];
    
    for(int x = 0; x < size; x++){
      for(int y = 0; y < size; y++){
        matrix[x][y] = 1;
      }
    }
    
    double avg_good_mp = 0;
    double avg_bad_mp = 0;
    double avg_thrash_mp = 0;
    double avg_p_thrash_mp = 0;
    
    double num = 5.0;
    
    for(int i = 0; i < num; i++){
      avg_bad_mp += bad_openmp_cache_opt(matrix, size, blocksize);
      avg_thrash_mp += thrash_openmp(matrix, size);
      avg_good_mp += good_openmp_cache_opt(matrix, size, blocksize);
      avg_p_thrash_mp += possible_thrash_openmp(matrix, size);
    }
    
    cout << "\nmatrix size: " << size << " x " << size << endl;
    myfile << size << ",";
    myfile << avg_bad_mp / num << ",";
    myfile << avg_thrash_mp / num << ",";
    myfile << avg_p_thrash_mp / num << ",";
    myfile << avg_good_mp / num << endl;
    
    for (int i = 0; i < size; i++)
      delete [] matrix[i];
    delete matrix;
    
    size = size - min;
    
  }
  
  myfile.close();
  
}

int main(){
  
  cache();
  
  openmp();
  
  return 0;
  
}
