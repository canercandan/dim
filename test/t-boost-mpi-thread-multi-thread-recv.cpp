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
	// unique_lock<mutex> lk(*pt_m);
	// communicator world;

	// while (1)
	//     {
	// 	// pt_cv->wait(lk); // plus de stabilité entre les deux threads
	// 	// cout << "x"; cout.flush();
	// 	// std::this_thread::sleep_for(std::chrono::microseconds(1000));

	// 	vector<request> reqs;

	// 	for (size_t i = 0; i < world.size(); ++i)
	// 	    {
	// 		// if (i == world.rank()) continue;

	// 		bool state = false;
	// 		world.recv(i, 100, state);

	// 		if (state)
	// 		    {
	// 			m_imm.lock();
	// 			// cout << "*"; cout.flush();
	// 			vec_imm[i].push( Pop<EOT>() );
	// 			// world.recv(i, 0, vec_imm[i].front());
	// 			reqs.push_back( world.irecv(i, 0, vec_imm[i].front()) );
	// 			m_imm.unlock();
	// 		    }
	// 	    }

	// 	wait_all(reqs.begin(), reqs.end());
	//     }
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
		// if ( g++ == 10000 )
		//     {
		cout << "."; cout.flush();
		cout << _pop.size() << " "; cout.flush();
		g = 0;
		// }

		vector<request> reqs;

		vector< Pop<EOT> > newpops_send(world.size());

		// Selection
		for (int i = 0; i < world.size(); ++i)
		    {
			// if (i == world.rank()) continue;

			int chunk = 2;
			int size = _pop.size() > chunk ? chunk : _pop.size();
			// world.send(i, 100, size > 0);
			if (size)
			    {
				// Pop<EOT> newpop;
				// copy_n(_pop.begin(), size, back_inserter(newpop));

				for (auto& ind : _pop)
				    {
					newpops_send[i].push_back(ind);
				    }

				_pop.erase(_pop.begin(), _pop.begin() + size);

				// world.send(i, 0, newpop);
				reqs.push_back( world.isend(i, 0, newpops_send[i]) );
			    }
			else
			    {
				reqs.push_back( world.isend(i, 0, Pop<EOT>()) );
			    }
		    }
		// !Selection

		// wait_all(reqs.begin(), reqs.end());
		// reqs.clear();

		// for (int i = 0; i < world.size(); ++i)
		//     {
		// 	// if (i == world.rank()) continue;

		// 	m_imm.lock();
		// 	while (!vec_imm[i].empty())
		// 	    {
		// 		// cout << "="; cout.flush();
		// 		auto& newpop = vec_imm[i].front();
		// 		// copy(newpop.begin(), newpop.end(), back_inserter(_pop));
		// 		for (auto& ind : newpop)
		// 		    {
		// 			_pop.push_back(ind);
		// 		    }
		// 		vec_imm[i].pop();
		// 	    }
		// 	m_imm.unlock();
		//     }


		// vector<request> reqs;

		vector< Pop<EOT> > newpops(world.size());

		for (size_t i = 0; i < world.size(); ++i)
		    {
			// if (i == world.rank()) continue;

			// bool state = false;
			// world.recv(i, 100, state);

			// if (state)
			    {
				// m_imm.lock();
				// cout << "*"; cout.flush();
				// vec_imm[i].push( Pop<EOT>() );
				// world.recv(i, 0, vec_imm[i].front());
				reqs.push_back( world.irecv(i, 0, newpops[i]) );
				// m_imm.unlock();
			    }
		    }

		wait_all(reqs.begin(), reqs.end());

		for (auto& newpop : newpops)
		    {
			for (auto& ind : newpop)
			    {
				_pop.push_back(ind);
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
    tr/*.add(cmt)*/.add(cpt);

    tr();

    return 0;
}
