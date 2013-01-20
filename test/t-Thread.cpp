#include <dim/dim>

using namespace dim::core;

int main()
{
    Communicate cmt;
    Compute cpt;

    ThreadsRunner tr;
    tr.add(cmt).add(cpt);

    tr();

    return 0;
}
