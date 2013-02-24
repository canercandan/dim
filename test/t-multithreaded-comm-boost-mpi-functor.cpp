#include <boost/mpi.hpp>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <cassert>

using namespace std;
using namespace boost;
using namespace boost::mpi;

class Sender// : public Thread< int& >
{
public:
    Sender(size_t to) : _to(to) {}
    Sender(Sender&& other) : _to(other._to) {}
    Sender& operator=(const Sender& other) = default;

    void operator()()
    {
	communicator world;

	while (1)
	    {
		string s("hello");
		vector<string> v(1000, s);
		world.send(_to, _to * world.size() + world.rank(), v);
	    }
    }

private:
    size_t _to;
};

class Receiver// : public Thread< int& >
{
public:
    Receiver(size_t from) : _from(from) {}
    Receiver(Receiver&& other) : _from(other._from) {}
    Receiver& operator=(const Receiver& other) = default;

    void operator()()
    {
	communicator world;

	while (1)
	    {
		vector<string> v;
		world.recv(_from, world.rank() * world.size() + _from, v);
		// cout << v[0] << " "; cout.flush();
	    }
    }

private:
    size_t _from;
};

int main(int argc, char *argv[])
{
    environment env(argc, argv, MPI_THREAD_MULTIPLE, true);
    communicator world;

    vector<Sender*> senders;
    vector<Receiver*> receivers;
    vector<thread> ths;

    for (int i = 0; i < world.size(); ++i)
    	{
    	    if (i == world.rank()) { continue; }

	    senders.push_back( new Sender(i) );
	    receivers.push_back( new Receiver(i) );

    	    ths.emplace_back( std::ref( *(senders.back()) ) );
    	    ths.emplace_back( std::ref( *(receivers.back()) ) );
    	}

    for (auto& t : ths) { t.join(); }

    return 0;
}
