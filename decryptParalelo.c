// Archivo decryptParalelo.c
#include <rpc/des_crypt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <omp.h>

const int BLOCK = 1000000;

int main(int argc, char **argv)
{
    int N, id;
    long upper = (1L << 56) long found = -1;
    MPI_Status st;
    MPI_Request req;
    int flag = 0;
    int ciphlen = strlen((char *)cipher);

    MPI_Init(&argc, &argv);
    double start = MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &N);

    MPI_Irecv((void *)&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &req);
    int interCount = 0;

    long idx = 0;
    while (idx < upper && found < 0)
    {
#pragma omp parallel for default(none) shared(cipher, ciphlen, found, idx, id, N)
        for (long i = idx + id; i < idx + N * BLOCK; i += N)
        {
            if (tryKey(i, (char *)cipher, ciphlen))
            {
#pragma omp critical
                found = i;
            }
        }
        // terminacion de comunicacion (senial)
        if (found >= 0)
        {
            for (int node = 0; node < N; node++)
                MPI_Send((void *)&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
        }
        idx += N * BLOCK;
    }
    // print resultados node 0
    if (id == 0)
    {
        MPI_Wait(&req, &st);
        decrypt(found, (char *)cipher, ciphlen);
        printf("%i nodos en %lf sec : %li %s\n", N, MPI_Wtime() - start, found, cipher);
    }
    MPI_Finalize(); // Liberamos entorno MPI
}