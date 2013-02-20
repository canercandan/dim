#include <mpi.h>
// #include <boost/mpi.hpp>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <cassert>

using namespace std;
// using namespace boost;
// using namespace boost::mpi;
using namespace MPI;

void send(int dst)
{
    // communicator world;
    int rank = COMM_WORLD.Get_rank();
    int size = COMM_WORLD.Get_size();

    while (1)
	{
	    // world.send(dst, dst*world.size()+world.rank(), s);
	    int v = 42;
	    COMM_WORLD.Send(&v, 1, INT, dst, dst*size+rank);
	}
}

void recv(int src)
{
    // communicator world;
    int rank = COMM_WORLD.Get_rank();
    int size = COMM_WORLD.Get_size();

    while (1)
	{
	    int v;
	    // world.recv(src, world.rank()*world.size()+src, s);
	    COMM_WORLD.Recv((void*)&v, 1, INT, src, rank*size+src);
	    // cout << v << " "; cout.flush();
	}
}

// void sendrecv(int i)
// {
//     communicator world;
//     vector<request> reqs;
//     string ss("hello");
//     reqs.push_back( world.isend(i, i*world.size()+world.rank(), ss) );
//     string sr;
//     reqs.push_back( world.irecv(i, world.rank()*world.size()+i, sr) );
//     wait_all(reqs.begin(), reqs.end());
// }

int main(int argc, char *argv[])
{
    // environment env(argc, argv);
    // communicator world;

    // Init(argc, argv);
    int provided = Init_thread(argc, argv, MPI_THREAD_MULTIPLE);

    assert(provided == MPI_THREAD_MULTIPLE);

    int rank = COMM_WORLD.Get_rank();
    int size = COMM_WORLD.Get_size();

    vector<thread> ths;

    // for (int i = 0; i < world.size(); ++i)
    // 	{
    // 	    if (i == world.rank()) { continue; }

    // 	    // ths.emplace_back( send, i );
    // 	    // ths.emplace_back( recv, i );

    // 	    ths.emplace_back( sendrecv, i );
    // 	}

    for (int i = 0; i < size; ++i)
    	{
    	    if (i == rank) { continue; }

    	    ths.emplace_back( send, i );
    	    ths.emplace_back( recv, i );
    	}

    for (auto& t : ths) { t.join(); }

    Finalize();

    return 0;
}
