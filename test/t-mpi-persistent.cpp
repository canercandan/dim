#include "mpi.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    MPI_Request r;
    MPI_Status s;
    // int flag;
    int buf[10];
    int rbuf[10];
    int tag = 27;
    int dest = 0;
    int rank, size;

    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    /* Create a persistent send request */
    // tout le monde prépare l'envoi à 0
    MPI_Send_init( buf, 10, MPI_INT, dest, tag, MPI_COMM_WORLD, &r );

    /* Use that request */
    if (rank == 0) {
	// on alloue un tableau de size request pour les irecv
	MPI_Request *rr = (MPI_Request *)malloc(size * sizeof(MPI_Request));
	for (int i=0; i<size; i++) {
	    // 0 va recevoir de tout le monde
	    MPI_Irecv( rbuf, 10, MPI_INT, i, tag, MPI_COMM_WORLD, &rr[i] );
	}
	// 0 va envoyer à 0
	MPI_Start( &r );
	// 0 envoi à 0
	MPI_Wait( &r, &s );
	// 0 recoit de tout le monde
	MPI_Waitall( size, rr, MPI_STATUSES_IGNORE );
	free(rr);
    }
    else {
	// non-0 va envoyer à 0
	MPI_Start( &r );
	// non-0 envoi à 0
	MPI_Wait( &r, &s );
    }

    MPI_Request_free( &r );


    // if (rank == 0)
    // 	{
    // 	    MPI_Request sr;
    // 	    /* Create a persistent receive request */
    // 	    // 0 prépare la récéption de tout le monde
    // 	    MPI_Recv_init( rbuf, 10, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &r );
    // 	    // 0 va envoyer à 0
    // 	    MPI_Isend( buf, 10, MPI_INT, 0, tag, MPI_COMM_WORLD, &sr );
    // 	    for (int i=0; i<size; i++) {
    // 		// 0 va recevoir de tout le monde
    // 		MPI_Start( &r );
    // 		// 0 recoit de tout le monde
    // 		MPI_Wait( &r, &s );
    // 	    }
    // 	    // 0 envoi à 0
    // 	    MPI_Wait( &sr, &s );
    // 	    MPI_Request_free( &r );
    // 	}
    // else {
    // 	// non-0 envoi à 0
    // 	MPI_Send( buf, 10, MPI_INT, 0, tag, MPI_COMM_WORLD );
    // }

    MPI_Finalize();
    return 0;
}
