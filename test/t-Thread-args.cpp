#include <dim/dim>
#include <iostream>

using namespace dim::core;
using namespace std;

template <typename... Args>
using T = Thread<Args...>;
template <typename... Args>
using TH = ThreadsHandler<Args...>;
template <typename... Args>
using TR = ThreadsRunner<Args...>;

class MyThread : public T<int&, int&>
{
public:
    void operator()(int& a, int& b)
    {
	while (1) { cout << a+10 << " " << b+10 << " "; cout.flush(); }
    }
};

class MyThread2 : public T<int&, int&>
{
public:
    void operator()(int& a, int& b)
    {
	while (1) { cout << a+20 << " " << b+20 << " "; cout.flush(); }
    }
};

class MyThread3 : public TH<int&, int&>
{
public:
    void addTo(TR<int&, int&>&)
    {

    }
};

int main()
{
    TR<int&, int&> tr;

    MyThread mt;
    MyThread2 mt2;
    MyThread3 mt3;
    tr.add(mt).add(mt2).addHandler(mt3);

    int a = 1, b = 1;

    tr(a, b);

    return 0;
}
