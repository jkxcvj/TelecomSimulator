#include <iostream>
#include <limits>

int main()
{
    int maxValue = std::numeric_limits<int>::max();
    long long result = static_cast<long long>(maxValue) + 1;
    std::cout << result << '\n';

    return 0;
}