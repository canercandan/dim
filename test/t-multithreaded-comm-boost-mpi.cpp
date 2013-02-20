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
	    vector<string> v(1000, s);
	    world.send(dst, dst*world.size()+world.rank(), v);
	}
}

void recv(int src)
{
    communicator world;

    while (1)
	{
	    vector<string> v;
	    world.recv(src, world.rank()*world.size()+src, v);
	    cout << v[0] << " "; cout.flush();
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
