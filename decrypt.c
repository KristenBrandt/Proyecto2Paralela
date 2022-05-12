// Archivo: decrypt.c

#include <rpc/des_crypt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>

void decrypt(long key, char *ciph, int len)
{
    //
    long k = 0;
    for (int i = 0; i < 8; i++)
    {
        key <<= 1;
        k += (key & (0xFE << i * 8));
    }

    des_separaty((char *)&k);
    //
    ecb_crypt((char *)&k, (char *)ciph, 16, DES_DECRYPT);
}

char search[] = " the ";
//
bool tryKey(long key, char *ciph, int len)
{
    char temp[len + 1];
    memcpy(temp, ciph, len); // Copia len caracteres del espacio en memoria ciph al espacio en memoria temp
    temp[len] = 0;
    decrypt(key, temp, len);
    return strstr((char *)temp, search) != NULL; // determina si search se encuentra en el string temp
}

unsigned char cipher[] = {108, 245, 65, 63, 125, 200, 150, 66, 17, 170, 207, 170, 34, 31, 70, 215, 0};
int main(int argc, char **argv)
{
    // Inicializacion de variables de MPI y de uso
    int N, id;
    long upper = (1L << 56); // upper bound DES keys 2^56
    long mylower, myupper;
    MPI_Status st;   // Estructura que respresenta el estatus de el mensaje recibido
    MPI_Request req; // Estructura que representa un handle con MPI_wait
    int flag;
    int ciphlen = strlen(cipher);

    MPI_Init(&argc, &argv);             // inicializacion de entorno de ejecucion MPI
    MPI_Comm_size(MPI_COMM_WORLD, &N);  // Se determina el tamanio del grupo de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &id); // Se determina el rank del calling process

    // Se determina el espacio de memoria de cada proceso

    int range_per_node = upper / N;
    mylower = range_per_node * id;
    myupper = range_per_node * (id + 1) - 1;
    if (id == N - 1)
    {
        // compensar residuo
        myupper = upper;
    }

    long found = 0;

    MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req); // Comienza un nonblocking recieve

    for (int i = mylower; i < myupper && found == 0; ++i) // for que recorre el espacio de memoria de cada proceso
    {
        if (tryKey(i, (char *)cipher, ciphlen)) // if que determina si la llave es la correcta
        {
            found = i;
            for (int node = 0; node < N; node++) // for que envia un blocking send a otros ranks
            {
                MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
            }
            break;
        }
    }
    if (id == 0) // proceso rank 0 que espera hasta que se haya encontrado la llave y despliega el resultado
    {
        MPI_Wait(&req, &st);
        decrypt(found, (char *)cipher, ciphlen);
        printf("%li %s\n", found, cipher);
    }
    MPI_Finalize(); // Liberamos entorno MPI
}
