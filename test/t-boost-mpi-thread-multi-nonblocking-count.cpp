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
    MyCommunicator(IslandData<EOT>& data) : _data(data) {}

    void operator()()
    {
	unique_lock<mutex> lk(*pt_m);
	communicator world;
	ostringstream ss;
	ss << "trace." << world.rank() << ".comm.txt";
	ofstream of(ss.str().c_str());

	while (1)
	    {
		// pt_cv->wait(lk); // plus de stabilité entre les deux threads

		// of << "x"; of.flush();

		// of << vec_em[(world.rank()+1) % world.size()].size() << " "; of.flush();
		// of << vec_imm[(world.rank()+1) % world.size()].size() << " "; of.flush();

		// std::this_thread::sleep_for(std::chrono::microseconds(1000));

		for (size_t i = 0; i < world.size(); ++i)
		    {
			if (i == world.rank()) continue;

			vector<request> reqs;

			m_em.lock();
			vector< Pop<EOT> > em;
			while (!vec_em[i].empty())
			    {
				em.push_back( vec_em[i].front() );
				vec_em[i].pop();
			    }
			m_em.unlock();

			of << "e" << em.size() << " "; of.flush();
			world.send(i, 100, em.size());

			if (em.size())
			    {
				reqs.push_back( world.isend(i, 0, em) );
			    }

			size_t count = 0;
			world.recv(i, 100, count);
			of << "i" << count << " "; of.flush();

			vector< Pop<EOT> > imm;
			if (count)
			    {
				reqs.push_back( world.irecv(i, 0, imm) );
			    }

			wait_all(reqs.begin(), reqs.end());

			if (!imm.empty())
			    {
				m_imm.lock();
				for (size_t k = 0; k < imm.size(); ++k)
				    {
					vec_imm[i].push( imm[k] );
				    }
				m_imm.unlock();
			    }
		    }
	    }
    }
private:
    IslandData<EOT>& _data;
};

template <typename EOT>
class MyAlgo : public Compute
{
public:
    MyAlgo(Pop<EOT>& pop, IslandData<EOT>& data) : _pop(pop), _data(data) {}

    void operator()()
    {
	communicator world;
	ostringstream ss;
	ss << "trace." << world.rank() << ".algo.txt";
	ofstream of(ss.str().c_str());

	while (1)
	    {
		// of << "."; of.flush();

		of << _pop.size() << " "; of.flush();

		vector< Pop<EOT> > pops( world.size() );

		for (auto &indi : _pop)
		    {
			double s = 0;
			int r = rng.rand() % 1000 + 1;

			size_t j;
			for ( j = 0; j < world.size() && r > s; ++j )
			    {
				s += _data.proba[j];
			    }
			--j;

			pops[j].push_back(indi);
		    }

		_pop.clear();

		// of << _pop.size() << " "; of.flush();

		m_em.lock();
		for (int i = 0; i < world.size(); ++i)
		    {
			if (i == world.rank()) continue;
			vec_em[i].push( pops[i] );
		    }
		m_em.unlock();

		for (auto& indi : pops[world.rank()])
		    {
			_pop.push_back( indi );
		    }

		// of << _pop.size() << " "; of.flush();

		for (int i = 0; i < world.size(); ++i)
		    {
			if (i == world.rank()) continue;

			m_imm.lock();
			while (!vec_imm[i].empty())
			    {
				// of << "="; of.flush();
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

		std::this_thread::sleep_for(std::chrono::microseconds(4 * 100000));
		pt_cv->notify_one();
	    }
    }

private:
    Pop<EOT>& _pop;
    IslandData<EOT>& _data;
};

int main(int argc, char *argv[])
{
    environment env(argc, argv);
    communicator world;

    // cout << "tag max: " << env.max_tag() << endl;

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

    IslandData<EOT> data;

    MigrationMatrix<EOT> probabilities( world.size() );
    // InitMatrix<EOT> initmatrix(false, 0);
    // InitMatrix<EOT> initmatrix(false, 100);
    InitMatrix<EOT> initmatrix(false, 50);

    if ( 0 == world.rank() )
	{
	    initmatrix( probabilities );
	    std::cout << probabilities;
	    data.proba = probabilities(world.rank());
	    for (size_t j = 0; j < world.size(); ++j)
		{
		    data.probaret[j] = probabilities(j, world.rank());
		}

	    for (size_t i = 1; i < world.size(); ++i)
		{
		    world.send( i, 500, probabilities(i) );
		    vector< typename EOT::Fitness > probaRetIsl( world.size() );
		    for (size_t j = 0; j < world.size(); ++j)
			{
			    probaRetIsl[j] = probabilities(j,i);
			}
		    world.send( i, 501, probaRetIsl );
		}
	}
    else
	{
	    world.recv( 0, 500, data.proba );
	    world.recv( 0, 501, data.probaret );
	}

    apply<EOT>(eval, pop);

    MyCommunicator<EOT> cmt(data);
    MyAlgo<EOT> cpt(pop, data);

    ThreadsRunner tr;
    tr.add(cmt).add(cpt);

    tr();

    return 0;
}
