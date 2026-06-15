// José Miguel Comprés Arias - 10153259

#include "mpi.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <random>

using namespace std;

void merge(vector<int>& arr, int left, int mid, int right) {
  int n1 = mid - left + 1;
  int n2 = right - mid;

  vector<int> L(n1);
  vector<int> R(n2);

  for (int i = 0; i < n1; i++)
    L[i] = arr[left + i];
  for (int j = 0; j < n2; j++)
    R[j] = arr[mid + 1 + j];

  int i = 0;
  int j = 0;
  int k = left;

  while (i < n1 && j < n2) {
    if (L[i] <= R[j]) {
      arr[k] = L[i];
      i++;
    } else {
      arr[k] = R[j];
      j++;
    }
    k++;
  }

  while (i < n1) {
    arr[k] = L[i];
    i++;
    k++;
  }

  while (j < n2) {
    arr[k] = R[j];
    j++;
    k++;
  }
}

void merge_sort(vector<int>& arr, int left, int right) {
  if (left >= right) {
    return;
  }

  int mid = left + (right - left) / 2;

  merge_sort(arr, left, mid);
  merge_sort(arr, mid + 1, right);

  merge(arr, left, mid, right);
}

int main (int argc, char *argv[]) {

  int rank, size;
  MPI_Status estado;
  double start_time, end_time;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  vector<int> arr, arrlocal, distribuciones, indices_distribuciones;
  int n;

  if (rank == 0) {
    cout<<"Ingrese el tamano del arreglo (n > 0): "<<endl;
    cin>>n;
  }

  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (n<=0) {
    MPI_Finalize();
    return 0;
  }

  if (rank == 0) {
    arr.resize(n);

    char random;
    cout<<"Rellenar el arreglo con valores random ? (Ingrese 1 para indicar que sí y cualquier otra tecla para indicar que no): "<<endl;
    cin>>random;
    if (random == '1') {
      random_device rd;
      mt19937 generador(rd());
      uniform_int_distribution<long> distribucion(1, 1000000000);
      for (int i = 0; i < n; i++) {
        arr[i] = distribucion(generador);
      }
      cout<<"Arreglo: "<<endl;
      for (int in : arr) {
        cout<<in<<' ';
      }
      cout<<endl;
    }
    else {
      cout<<"Ingrese los elementos del arreglo"<<endl;
      for (int i = 0; i<n; i++) {
        cin>>arr[i];
      }
    }

    distribuciones.resize(size, n/size);
    indices_distribuciones.resize(size);
    indices_distribuciones[0] = 0;
    for (int i = 0; i<size; i++) {
      distribuciones[i] += (i < n%size)? 1 : 0;
      if (i == 0) continue;
      indices_distribuciones[i] = indices_distribuciones[i-1] + distribuciones[i-1];
    }
  }

  arrlocal.resize(n/size + ((rank<n%size)?1:0));

  MPI_Barrier(MPI_COMM_WORLD);
  start_time = MPI_Wtime();

  //scatterv es para distribuir con diferentes cargas (ya que si hay un residuo de n/size se asigna esa parte extra a los primeros procesos)
  MPI_Scatterv(arr.data(), distribuciones.data(), indices_distribuciones.data(), MPI_INT, arrlocal.data(), arrlocal.size(), MPI_INT, 0, MPI_COMM_WORLD);

  if (!arrlocal.empty()) {
    merge_sort(arrlocal, 0, arrlocal.size() - 1);
  }


  int step = 1;
  while (step < size) {
    if (rank % (2 * step) == 0) {
      int ind_to_receive = rank + step;

      if (ind_to_receive < size) {
        MPI_Probe(ind_to_receive, 0, MPI_COMM_WORLD, &estado);
        int indtor_size;
        MPI_Get_count(&estado, MPI_INT, &indtor_size);

        int prev_size = arrlocal.size();
        arrlocal.resize(prev_size + indtor_size); 

        MPI_Recv(arrlocal.data() + prev_size, indtor_size, MPI_INT, ind_to_receive, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        merge(arrlocal, 0, prev_size - 1, arrlocal.size() - 1);
      }
    }

    else {
      MPI_Send(arrlocal.data(), arrlocal.size(), MPI_INT, rank-step, 0, MPI_COMM_WORLD);
      break;
    }

    step *= 2;
  }

  end_time = MPI_Wtime();

  if (rank == 0) {
    cout<<"Tiempo de ejecución: "<<(end_time - start_time)*1000<<" ms"<<endl<<endl;

    char mostrar;
    cout<<"Mostrar arreglo ordenado? (Ingrese 1 para indicar que sí y cualquier otra tecla para indicar que no): "<<endl;
    cin>>mostrar;
    if (mostrar == '1') {
      for (int i = 0; i<n; i++) {
        cout<<arrlocal[i]<<' ';
      }
      cout<<endl;
    }
  }

  MPI_Finalize();

  return 0;
}
