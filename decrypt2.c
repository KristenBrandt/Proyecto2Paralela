// Archivo: decrypt2.c

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
    long found = 0;
    MPI_Status st;   // Estructura que respresenta el estatus de el mensaje recibido
    MPI_Request req; // Estructura que representa un handle con MPI_wait
    int flag;
    int ciphlen = strlen((char *)cipher);

    MPI_Init(&argc, &argv);             // inicializacion de entorno de ejecucion MPI
    MPI_Comm_size(MPI_COMM_WORLD, &N);  // Se determina el tamanio del grupo de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &id); // Se determina el rank del calling process
    MPI_Irecv((void *)&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
    int interCount = 0;

    for (long i = id; i < upper; i += N)
    {
        if (tryKey(i, (cahr *)cipher, ciphlen))
        {
            found = i;
            for (int node = 0; node < N; node++)
                MPI_Send((void *)&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
            break;
        }
        if (++interCount % 1000 == 0) // estatus de recive pendiente
        {
            MPI_Test(&req, &flag, &st);
            if (flag)
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
