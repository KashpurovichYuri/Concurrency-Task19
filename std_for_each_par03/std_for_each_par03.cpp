#include <algorithm>
#include <future>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

template < typename T >
void func(T& elem)
{
	std::cout << elem << " ";
}

template < typename Iterator, typename Function >
void parallel_for_each(Iterator first, Iterator last, Function function)
{
	const std::size_t length = std::distance(first, last);

	const std::size_t max_size = 25;

	if (length <= max_size)
	{
		std::for_each(first, last, function);
	}
	else
	{
		Iterator middle = first;
		std::advance(middle, length / 2);

		std::future < void > first_half_result =
			std::async(parallel_for_each < Iterator, Function >, first, middle, function);

		parallel_for_each(middle, last, function);
	}
}

int main()
{
	std::vector < int > v(100);
	std::iota(std::begin(v), std::end(v), 1);
	parallel_for_each(std::begin(v), std::end(v), func < int >);

	system("pause");

	return EXIT_SUCCESS;
}