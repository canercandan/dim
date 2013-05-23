// http://stackoverflow.com/questions/11407577/how-to-use-boost-barrier

#include <boost/thread/barrier.hpp>
#include <chrono>
#include <thread>

boost::mutex io_mutex;

// using namespace std;

void thread_fun(boost::barrier& cur_barier, unsigned& time)
{
    while(1)
	{
	    std::cerr << "Waiting... " << time << std::endl; std::cerr.flush();
	    std::this_thread::sleep_for(std::chrono::seconds(time));
	    cur_barier.wait();
	    // boost::lock_guard<boost::mutex> locker(io_mutex);
	}
}

int main()
{
    boost::barrier bar(3);
    unsigned time1 = 1;
    std::thread thr1(std::bind(&thread_fun, std::ref(bar), std::ref(time1)));
    unsigned time2 = 2;
    std::thread thr2(std::bind(&thread_fun, std::ref(bar), std::ref(time2)));
    unsigned time3 = 3;
    std::thread thr3(std::bind(&thread_fun, std::ref(bar), std::ref(time3)));
    thr1.join();
    thr2.join();
    thr3.join();
}
