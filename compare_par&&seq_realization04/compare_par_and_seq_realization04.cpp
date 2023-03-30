#include <algorithm>
#include <future>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>
#include <cmath>
#include <execution>
#include <fstream>

#include "../timer/timer.cpp"

auto measure_for_each(auto policy, const size_t vsize, double & elem)
{
	std::vector < double > v(vsize);
	std::iota(std::begin(v), std::end(v), 1);

	Timer time {"starting algorithm std::for_each...", std::cout};
	std::for_each(
		policy,
		std::begin(v),
		std::end(v),
		[](double& i)
		{
			i = std::sin(i);
	});
	time.stop();
	elem = time.get_time();
}

auto trig_operator1(const double a1, const double a2)
{
	return a1 + std::sin(a2);
}

auto measure_partial_sum(const size_t vsize, double & elem)
{
	std::vector < double > v(vsize);
	std::iota(std::begin(v), std::end(v), 1);

	Timer time {"starting algorithm std::partial_sum...", std::cout};
	std::partial_sum(std::cbegin(v), std::cend(v), std::begin(v), trig_operator1);
	time.stop();
	elem = time.get_time();
}

auto measure_inclusive_scan(const size_t vsize, double & elem)
{
	std::vector < double > v(vsize);
	std::iota(std::begin(v), std::end(v), 1);

	Timer time {"starting algorithm std::inclusive_scan...", std::cout};
	std::inclusive_scan(std::execution::par, std::cbegin(v), std::cend(v), std::begin(v), trig_operator1);
	time.stop();
	elem = time.get_time();
}

auto trig_operator2(const double a1, const double a2)
{
	return std::sin(a1) + std::sin(a2);
}

auto measure_inner_product(const size_t vsize, double & elem)
{
	std::vector < double > v(vsize);
	std::iota(std::begin(v), std::end(v), 1);

	Timer time {"starting algorithm std::inner_product...", std::cout};
	std::inner_product(std::cbegin(v), std::cend(v), std::begin(v), 0, std::plus < > (), trig_operator2);
	time.stop();
	elem = time.get_time();
}

auto measure_transform_reduce(const size_t vsize, double & elem)
{
	std::vector < double > v(vsize);
	std::iota(std::begin(v), std::end(v), 1);

	Timer time {"starting algorithm std::transform_reduce...", std::cout};
	std::transform_reduce(std::execution::par, std::cbegin(v), std::cend(v), std::begin(v), 0, std::plus < > (), trig_operator2);
	time.stop();
	elem = time.get_time();
}

template < typename Function >
auto experiment(
	Function function,
	std::vector < double > & seria,
	std::vector < double > & results,
	const size_t num_elems,
	std::ofstream & output)
{
	size_t j = 0;
	for (auto i = 100000; i < num_elems; i += 10000)
	{
		for (auto& elem : seria)
			function(i, elem);
		results[j++] = std::accumulate(std::begin(seria), std::end(seria), 0.0) / seria.size();
	}

	for (const auto& result : results)
		output << result << " ";
	output << "\n\n";
}

int main()
{	
	const size_t num_elems = 1000000;
	std::vector < double > seria(20);
	std::vector < double > results(num_elems / 10);
	std::ofstream output("output.txt");

	// std::for_each
	size_t j = 0;
	for (auto i = 100000; i < num_elems; i += 10000)
	{
		for (auto& elem : seria)
			measure_for_each(std::execution::seq, i, elem);
		results[j++] = std::accumulate(std::begin(seria), std::end(seria), 0.0) / seria.size();
	}
	output << "Sequent std::for_each:\n";  
	for (const auto& result : results)
		output << result << " ";
	output << "\n\n";

	j = 0;
	for (auto i = 100000; i < num_elems; i += 10000)
	{
		for (auto& elem : seria)
			measure_for_each(std::execution::par, i, elem);
		results[j++] = std::accumulate(std::begin(seria), std::end(seria), 0.0) / seria.size();
	}
	output << "Parallel std::for_each:\n";  
	for (const auto& result : results)
		output << result << " ";
	output << "\n\n";

	// std::partial_sum && std::inclusive_scan
	output << "Sequent std::partial_sum:\n"; 
	experiment(measure_partial_sum, seria, results, num_elems, output);

	output << "Parallel std::inclusive_scan:\n";
	experiment(measure_inclusive_scan, seria, results, num_elems, output);

	// std::inner_product && Parallel std::transform_reduce
	output << "Sequent std::inner_product:\n";
	experiment(measure_inner_product, seria, results, num_elems, output);

	output << "Parallel std::transform_reduce:\n";
	experiment(measure_transform_reduce, seria, results, num_elems, output);

	system("pause");

	return EXIT_SUCCESS;
}
