#include <queue>
#include <iostream>

using namespace std;


int main(int argc, char *argv[])
{
    queue<int> stack;

    stack.push( 1 );
    stack.push( 2 );
    stack.push( 3 );
    stack.push( 4 );
    stack.push( 0 );
    stack.back() = 5;
    stack.push( 6 );

    while (!stack.empty())
	{
	    cout << stack.front() << " ";
	    stack.pop();
	}

    return 0;
}
