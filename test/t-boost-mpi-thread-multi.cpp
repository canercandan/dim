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

mutex m_em, m_imm;

template <typename EOT>
class MyCommunicator : public Communicate
{
public:
    MyCommunicator(Pop<EOT>& pop) : _pop(pop) {}

    void operator()()
    {
	unique_lock<mutex> lk(*pt_m);
	communicator world;

	while (1)
	    {
		// pt_cv->wait(lk); // plus de stabilité entre les deux threads
		// cout << "x"; cout.flush();
		// std::this_thread::sleep_for(std::chrono::microseconds(1000));

		for (size_t i = 0; i < world.size(); ++i)
		    {
			// if (i == world.rank()) continue;

			m_em.lock();
			vector< Pop<EOT> > em;
			while (!vec_em[i].empty())
			    {
				em.push_back( vec_em[i].front() );
				vec_em[i].pop();
			    }
			m_em.unlock();

			world.send(i, 100, em.size());

			for (auto& newpop : em)
			    {
				// cout << "^"; cout.flush();
				world.send(i, 0, newpop);
			    }
		    }

		for (size_t i = 0; i < world.size(); ++i)
		    {
			// if (i == world.rank()) continue;

			size_t count = 0;
			world.recv(i, 100, count);

			m_imm.lock();
			for (size_t k = 0; k < count; ++k)
			    {
				// cout << "*"; cout.flush();
				vec_imm[i].push( Pop<EOT>() );
				world.recv(i, 0, vec_imm[i].front());
			    }
			m_imm.unlock();
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
	size_t g = 0;

	while (1)
	    {
		if ( g++ == 10000 )
		    {
			cout << "."; cout.flush();
			g = 0;
		    }

			cout << _pop.size() << " "; cout.flush();

		// Selection
		for (int i = 0; i < world.size(); ++i)
		    {
			// if (i == world.rank()) continue;

			int chunk = 2;
			int size = _pop.size() > chunk ? chunk : _pop.size();
			if (size)
			    {
				Pop<EOT> newpop;
				// copy_n(_pop.begin(), size, back_inserter(newpop));

				for (auto& ind : _pop)
				    {
					newpop.push_back(ind);
				    }

				_pop.erase(_pop.begin(), _pop.begin() + size);

				m_em.lock();
				vec_em[i].push( newpop );
				m_em.unlock();
			    }
		    }
		// !Selection

		for (int i = 0; i < world.size(); ++i)
		    {
			// if (i == world.rank()) continue;

			m_imm.lock();
			while (!vec_imm[i].empty())
			    {
				// cout << "="; cout.flush();
				auto& newpop = vec_imm[i].front();
				// copy(newpop.begin(), newpop.end(), back_inserter(_pop));
				for (auto& ind : newpop)
				    {
					_pop.push_back(ind);
				    }
				vec_imm[i].pop();
			    }
			m_imm.unlock();
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
