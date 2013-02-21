#include <thread>
#include <iostream>

using namespace std;

template<class... Args>
class Thread
{
public:
    virtual void operator()(Args...) = 0;

    void run(Args... args)
    {
	std::thread t(ref(*this), ref(args)...);
	t.join();
    }
};

class Functor : public Thread<int&>
{
public:
    void operator()(int&)
    {
    	while (1)
    	    {
    		cout << "42 "; cout.flush();
    	    }
    }
};

int main()
{
    int a = 12;
    Functor f;
    f.run(ref(a));

    return 0;
}
