#include <boost/mpi.hpp>
#include <dim/dim>
#include <queue>
#include <algorithm>

using namespace boost::mpi;
using namespace dim::core;
using namespace std;

using EOT = Bit<double>;

vector< queue< Pop<EOT> > > vec_em, vec_imm;
vector< queue< Pop<EOT> > > vec_fbs, vec_fbr;

template <typename EOT>
class MyCommunicator : public Communicate
{
public:
    MyCommunicator(Pop<EOT>& pop) : _pop(pop) {}

    void operator()()
    {
	std::unique_lock<std::mutex> lk(*pt_m);
	communicator world;

	while (1)
	    {
		// pt_cv->wait(lk);
		// cout << "x"; cout.flush();
		// std::this_thread::sleep_for(std::chrono::microseconds(1000));

		size_t dst = (world.rank()+1) % world.size();

		if ( !world.rank() )
		    {
			bool state = !vec_em[dst].empty();
			world.send(dst, 100, state);

			if (state)
			    {
				cout << "^"; cout.flush();
				Pop<EOT>& newpop = vec_em[dst].front();
				world.send(dst, 0, newpop);
				vec_em[dst].pop();
			    }
		    }
		else
		    {
			bool state = false;
			world.recv(dst, 100, state);

			if (state)
			    {
				cout << "*"; cout.flush();
				Pop<EOT> newpop;
				world.recv(dst, 0, newpop);
				vec_imm[dst].push( newpop );
			    }
		    }
	    }
    }

private:
    Pop<EOT>& _pop;
};

template <typename EOT>
class MyAlgo : public Compute
{
public:
    MyAlgo(Pop<EOT>& pop) : _pop(pop) {}

    void operator()()
    {
	communicator world;

	while (1)
	    {
		// cout << "."; cout.flush();

		size_t dst = (world.rank()+1) % world.size();

		if ( !world.rank() )
		    {
			// cout << "+"; cout.flush();
			// Selection
			Pop<EOT> newpop;
			int chunk = 2;
			int size = _pop.size() > chunk ? chunk : _pop.size();
			if (size)
			    {
				copy_n(_pop.begin(), size, back_inserter(newpop));
				_pop.erase(_pop.begin(), _pop.begin() + size);
				vec_em[dst].push( newpop );
				cout << _pop.size() << " "; cout.flush();
			    }
			// !Selection
		    }
		else
		    {
			while (!vec_imm[dst].empty())
			    {
				cout << "="; cout.flush();
				Pop<EOT>& newpop = vec_imm[dst].front();
				for (auto& ind : newpop)
				    {
					_pop.push_back( ind );
				    }
				vec_imm[dst].pop();
			    }
		    }

		// std::this_thread::sleep_for(std::chrono::microseconds(100000));
		pt_cv->notify_one();
	    }
    }

private:
    Pop<EOT>& _pop;
};

int main(int argc, char *argv[])
{
    environment env(argc, argv);
    communicator world;

    vec_em.resize(world.size());
    vec_imm.resize(world.size());
    vec_fbr.resize(world.size());
    vec_fbs.resize(world.size());

    Pop<EOT> pop;

    for (int i = 0; i < 100; ++i)
	{
	    EOT sol(1000, true);
	    sol.fitness(42);
	    pop.push_back(sol);
	}

    MyCommunicator<EOT> cmt(pop);
    MyAlgo<EOT> cpt(pop);

    ThreadsRunner tr;
    tr.add(cmt).add(cpt);

    tr();

    return 0;
}
