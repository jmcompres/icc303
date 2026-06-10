#include "mpi.h"
#include <cstdlib>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  
  int n,rank,size; 
  double PI25DT = 3.141592653589793238462643;
  double mypi,pi,h,sum;
  
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    cout<<"Introduce la precision del calculo (n>0): "<<endl;
    cin>>n;
  }

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (n<=0) {
    MPI_Finalize();
    exit(0);
  }
  else {
    h = 1.0/(double)n;
  }
  

  int lim = n/size;
  lim += (rank<(n%size)? 1 : 0);
  double offset = (double)min(rank,n%size)*(n/size + 1) + (double)max(0,rank-n%size)*(n/size);
  
  sum = 0.0;
  for (int i = 0; i<lim; i++) {
    double x = (offset + (double)i + 0.5) * h;
    sum += 4.0/(1.0 + x*x);
  }
  mypi = h*sum;

  MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (rank==0) {
    cout<<"El valor aproximado de PI es: " << pi << ", con un error de " <<(pi-PI25DT)<<"\n";
  }

  MPI_Finalize();

  return 0;
}

