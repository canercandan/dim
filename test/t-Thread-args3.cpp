#include <boost/mpi.hpp>
#include <thread>
#include <iostream>
#include <vector>

using namespace std;
using namespace boost::mpi;
using namespace boost;

template <class... Args>
class ThreadsRunner;

template <class... Args>
class Thread
{
public:
    virtual ~Thread() {}

    // virtual void operator()(Args...) = 0;
    virtual void operator()(Args...) {}

private:
    friend class ThreadsRunner<Args...>;

    void build(Args... args)
    {
	t = std::thread( std::ref(*this), std::ref(args)... );
	// t = std::thread( [&](Args... args){ (*this)(std::ref(args)...); }, std::ref(args)... );
    }

    inline void join() { t.join(); }

protected:
    std::thread t;
};

template <class... Args>
class ThreadsHandler
{
public:
    virtual ~ThreadsHandler() {}

    virtual void addTo(ThreadsRunner<Args...>&) = 0;
};

template <class... Args>
class ThreadsRunner
{
public:
    void operator()(Args... args)
    {
	for (auto& t : _vt) { t->build(args...); }
	for (auto& t : _vt) { t->join(); }
    }

    ThreadsRunner& add(Thread<Args...>& t)
    {
	_vt.push_back(&t);
	return *this;
    }

    ThreadsRunner& add(Thread<Args...>* t)
    {
	_vt.push_back(t);
	return *this;
    }

    ThreadsRunner& addHandler(ThreadsHandler<Args...>& t)
    {
	t.addTo(*this);
	return *this;
    }

private:
    std::vector< Thread<Args...>* > _vt;
};

class Main : public ThreadsHandler< int& >
{
public:
    ~Main()
    {
	for (auto& sender : _senders) { delete sender; }
	for (auto& receiver : _receivers) { delete receiver; }
    }

    class Sender : public Thread< int& >
    {
    public:
	Sender(size_t to) : _to(to) {}
	Sender(Sender&& other) : _to(other._to) {}
	Sender& operator=(const Sender& other) = default;

	void operator()(int&)
	{
	    while (1)
		{
		    // std::cout << "fbs "; std::cout.flush();
		}
	}

    private:
	size_t _to;
    };

    class Receiver : public Thread< int& >
    {
    public:
	Receiver(size_t from) : _from(from) {}
	Receiver(Receiver&& other) : _from(other._from) {}
	Receiver& operator=(const Receiver& other) = default;

	void operator()(int&)
	{
	    while (1)
		{
		    // std::cout << "fbr "; std::cout.flush();
		}
	}

    private:
	size_t _from;
    };

    virtual void addTo(ThreadsRunner< int& >& tr)
    {
	communicator world;

	for (int i = 0; i < world.size(); ++i)
	    {
		if (i == world.rank()) { continue; }

		_senders.push_back( new Sender(i) );
		_receivers.push_back( new Receiver(i) );

		tr.add( _senders.back() );
		tr.add( _receivers.back() );
	    }
    }

private:
    std::vector<Sender*> _senders;
    std::vector<Receiver*> _receivers;
};

int main(int argc, char** argv)
{
    environment env(argc, argv, MPI_THREAD_MULTIPLE, true);
    communicator world;

    ThreadsRunner< int& > tr;

    Main handler;
    tr.addHandler(handler);

    int x = 1;

    tr(x);

    return 0;
}
