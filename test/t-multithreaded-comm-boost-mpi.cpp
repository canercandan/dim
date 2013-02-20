#include <boost/mpi.hpp>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <cassert>

using namespace std;
using namespace boost;
using namespace boost::mpi;

void send(int dst)
{
    communicator world;

    while (1)
	{
	    string s("hello");
	    world.send(dst, dst*world.size()+world.rank(), s);
	}
}

void recv(int src)
{
    communicator world;

    while (1)
	{
	    string s;
	    world.recv(src, world.rank()*world.size()+src, s);
	    cout << s << " "; cout.flush();
	}
}

int main(int argc, char *argv[])
{
    environment env(argc, argv, MPI_THREAD_MULTIPLE, true);
    communicator world;

    vector<thread> ths;

    for (int i = 0; i < world.size(); ++i)
    	{
    	    if (i == world.rank()) { continue; }

    	    ths.emplace_back( send, i );
    	    ths.emplace_back( recv, i );

    	}

    for (auto& t : ths) { t.join(); }

    return 0;
}
