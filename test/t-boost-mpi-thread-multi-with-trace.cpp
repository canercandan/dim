#include <boost/mpi.hpp>
#include <dim/dim>
#include <queue>
#include <algorithm>
#include <eo>
#include <fstream>

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
    MyCommunicator() {}

    void operator()()
    {
	unique_lock<mutex> lk(*pt_m);
	communicator world;
	ostringstream ss;
	ss << "trace." << world.rank() << ".comm.txt";
	ofstream of(ss.str().c_str());

	for (int g = 0; g >= 0 /* < 10*/; ++g)
	    {
		// pt_cv->wait(lk); // plus de stabilité entre les deux threads

		// of << "x"; of.flush();

		of << vec_em[(world.rank()+1) % world.size()].size() << " "; of.flush();
		of << vec_imm[(world.rank()+1) % world.size()].size() << " "; of.flush();

		// std::this_thread::sleep_for(std::chrono::microseconds(1000));

		vector< request > reqs;

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

			of << "em:" << em.size() << " "; of.flush();
			world.send(i, 100, em.size());

			for (auto& newpop : em)
			    {
				of << "^"; of.flush();
				// world.send(i, 0, newpop);
				reqs.push_back( world.isend(i, 0, newpop) );
				break;
			    }
		    }

		// vector< vector< Pop<EOT> > > pops( world.size() );
		vector< Pop<EOT> > pops( world.size(), Pop<EOT>() );

		for (size_t i = 0; i < world.size(); ++i)
		    {
			// if (i == world.rank()) continue;

			size_t count = 0;
			world.recv(i, 100, count);
			of << "imm:" << count << " "; of.flush();

			if (count)
			    {
				// pops[i] = vector< Pop<EOT> >(count, Pop<EOT>());

				// m_imm.lock();
				// for (size_t k = 0; k < count; ++k)
				    {
					of << "*"; of.flush();
					// vec_imm[i].push( Pop<EOT>() );
					// world.recv(i, 0, vec_imm[i].back());
					// reqs.push_back( world.irecv(i, 0, vec_imm[i].back()) );
					reqs.push_back( world.irecv(i, 0, pops[i]) );
				    }
				// m_imm.unlock();
			    }
		    }

		wait_all( reqs.begin(), reqs.end() );

		m_imm.lock();
		for (size_t i = 0; i < pops.size(); ++i)
		    {
			// for (int k = 0; i < pops[i].size(); ++k)
			    {
				// cout << pops[i][k].size() << " ";
				// vec_imm[i].push( pops[i][k] );
				cout << pops[i].size() << " ";
				if (!pops[i].empty())
				    {
					vec_imm[i].push( pops[i] );
				    }
			    }
		    }
		m_imm.unlock();
	    }
    }
};

template <typename EOT>
class MyAlgo : public Compute
{
public:
    MyAlgo(Pop<EOT>& pop) : _pop(pop) {}

    void operator()()
    {
	communicator world;
	ostringstream ss;
	ss << "trace." << world.rank() << ".algo.txt";
	ofstream of(ss.str().c_str());

	for (int g = 0; g >= 0/* < 10*/; ++g)
	    {
		// of << "."; of.flush();

		// of << _pop.size() << " "; of.flush();

		// Selection
		for (int i = 0; i < world.size(); ++i)
		    {
			if (i == world.rank()) continue;

			// int chunk = 2;
			// int size = _pop.size() > chunk ? chunk : _pop.size();
			int size = _pop.size();
			if (size)
			    {
				Pop<EOT> newpop;
				// copy_n(_pop.begin(), size, back_inserter(newpop));

				for (auto& ind : _pop)
				    {
					newpop.push_back(ind);
				    }

				of << _pop.size() << " "; of.flush();
				// _pop.erase(_pop.begin(), _pop.begin() + size);
				_pop.clear();
				of << _pop.size() << " "; of.flush();

				m_em.lock();
				vec_em[i].push( newpop );
				m_em.unlock();
			    }
		    }
		// !Selection

		for (int i = 0; i < world.size(); ++i)
		    {
			if (i == world.rank()) continue;

			m_imm.lock();
			while (!vec_imm[i].empty())
			    {
				of << "="; of.flush();
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

    eoParser parser(argc, argv);
    eoState state;    // keeps all things allocated

    unsigned chromSize = parser.getORcreateParam(unsigned(1500), "chromSize", "The length of the bitstrings", 'n',"Problem").value();
    eoInit<EOT>& init = dim::do_make::genotype(parser, state, EOT(), 0);

    dim::evaluation::OneMax<EOT> mainEval;
    eoEvalFuncCounter<EOT> eval(mainEval);

    /*unsigned popSize = */parser.getORcreateParam(unsigned(100), "popSize", "Population Size", 'P', "Evolution Engine")/*.value()*/;
    dim::core::Pop<EOT>& pop = dim::do_make::detail::pop(parser, state, init);

    make_parallel(parser);
    make_verbose(parser);
    make_help(parser);

    ostringstream ss;
    ss << "trace." << world.rank() << ".txt";
    eo::log << eo::file(ss.str()) << "Start writing for rank: " << world.rank() << endl;

    vec_em.resize(world.size());
    vec_imm.resize(world.size());
    vec_fbr.resize(world.size());
    vec_fbs.resize(world.size());

    apply<EOT>(eval, pop);

    MyCommunicator<EOT> cmt;
    MyAlgo<EOT> cpt(pop);

    ThreadsRunner tr;
    tr.add(cmt).add(cpt);

    tr();

    return 0;
}
