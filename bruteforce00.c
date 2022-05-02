//bruteforce.c
//nota: el key usado es bastante pequenio, cuando sea random speedup variara

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <rpc/des_crypt.h>


void decrypt(long key, char *ciph, int len) // funcion que descifra el texto
{
  //set parity of key and do decrypt
  long k = 0;
  for(int i=0; i<8; ++i){
    key <<= 1;
    k += (key & (0xFE << i*8));
  }
  des_setparity((char *)&k);  //el poder del casteo y &
  ecb_crypt((char *)&k, (char *) ciph, 16, DES_DECRYPT);
}

void encrypt(long key, char *ciph, int len) // funcion que encripta el texto
{
  //set parity of key and do decrypt
  long k = 0;
  for(int i=0; i<8; ++i){
    key <<= 1;
    k += (key & (0xFE << i*8));
  }
  des_setparity((char *)&k);  //el poder del casteo y &
  ecb_crypt((char *)&k, (char *) ciph, 16, DES_ENCRYPT);
}

char search[] = " the ";
int tryKey(long key, char *ciph, int len) // funcion que prueba si la llave es la correctoa
{
  char temp[len+1];
  memcpy(temp, ciph, len); // Copia len caracteres del espacio en memoria ciph al espacio en memoria temp
  temp[len]=0;
  decrypt(key, temp, len);
  return strstr((char *)temp, search) != NULL; // determina si search se encuentra en el string temp
}

unsigned char cipher[] = {108, 245, 65, 63, 125, 200, 150, 66, 17, 170, 207, 170, 34, 31, 70, 215, 0};
int main(int argc, char *argv[]){ //char **argv

  // Inicializacion de variables de MPI y de uso

  int N, id;
  long upper = (1L <<56); //upper bound DES keys 2^56
  long mylower, myupper;
  MPI_Status st; // Estructura que respresenta el estatus de el mensaje recibido
  MPI_Request req; // Estructura que representa un handle con MPI_wait
  int flag;
  int ciphlen = strlen(cipher);
  MPI_Comm comm = MPI_COMM_WORLD; // grupo logico de procesos de MPI

  MPI_Init(NULL, NULL); // inicializacion de entorno de ejecucion MPI
  MPI_Comm_size(comm, &N); // Se determina el tamanio del grupo de procesos
  MPI_Comm_rank(comm, &id); // Se determina el rank del calling process

  // Se determina el espacio de memoria de cada proceso

  long range_per_node = upper / N;
  mylower = range_per_node * id;
  myupper = range_per_node * (id+1) -1;
  if(id == N-1){
    //compensar residuo
    myupper = upper;
  }

  long found = 0;
  int ready = 0;

  MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req); // Comienza un nonblocking recieve

  for(long i = mylower; i<myupper; ++i) // for que recorre el espacio de memoria de cada proceso
  {
    MPI_Test(&req, &ready, MPI_STATUS_IGNORE); // Se revisa si ya se cumplio con la tarea aka se encontro la llave
    if(ready)
      break;  //ya encontraron, salir

    if(tryKey(i, (char *)cipher, ciphlen)) // if que determina si la llave es la correcta
    {
      found = i;
      for(int node=0; node<N; node++) // for que envia un blocking send a otros ranks
      {
        MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
      }
      break;
    }
  }

  if(id==0) // proceso rank 0 que espera hasta que se haya encontrado la llave y despliega el resultado
  {
    MPI_Wait(&req, &st);
    decrypt(found, (char *)cipher, ciphlen);
    printf("%li %s\n", found, cipher);
  }

  MPI_Finalize(); // Liberamos entorno MPI
}
