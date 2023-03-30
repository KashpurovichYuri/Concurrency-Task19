#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <execution>
#include <thread>
#include <future>

#include "../timer/timer.cpp"

/*
auto estimate_pi_seq_integral(const size_t N)
{
	std::vector < double > vector(N);

	std::random_device seed;
	std::uniform_real_distribution < double > distribution(-1, 1);
	std::mt19937 engine(seed());

	std::for_each(
		std::begin(vector),
		std::end(vector),	
		[ &engine, &distribution ](auto& i)
		{			
			i = std::acos(distribution(engine));
		});

	double sum = std::accumulate(std::begin(vector), std::end(vector), 0);
	double pi = 2 * sum / vector.size();

	std::cout << sum << " " << pi;
}
*/

double estimate_pi_seq_geom(const size_t N)
{
	Timer time {"starting sequent calculations...", std::cout};
	size_t Nin = 0;
	size_t Nout = 0;

	std::random_device seed;
	std::uniform_real_distribution < double > distribution(-1, 1);
	std::mt19937 engine(seed());

	for (auto i = 0; i < N; ++i)
	{
		double x = distribution(engine);
		double y = distribution(engine);
		if (x * x + y * y < 1)
			++Nin;
		++Nout;
	}

	double pi = 4.0 * Nin / Nout;

	time.stop();
	auto t = time.get_time();
	std::cout << "Time of sequent calculations: " << t << "\n\n";

	return pi;
}

class Threads_Guard
{
public:

	explicit Threads_Guard(std::vector < std::thread > & threads) :
		m_threads(threads)
	{}

	Threads_Guard			(Threads_Guard const&) = delete;

	Threads_Guard& operator=(Threads_Guard const&) = delete;

	~Threads_Guard() noexcept
	{
		try
		{
			for (std::size_t i = 0; i < m_threads.size(); ++i)
			{
				if (m_threads[i].joinable())
				{
					m_threads[i].join();
				}
			}
		}
		catch (...)
		{
			// std::abort();
		}
	}

private:

	std::vector < std::thread > & m_threads;
};

double estimate_pi_par(const size_t N)
{
	Timer time {"starting parallel calculations...", std::cout};

	const std::size_t min_per_thread = 25;
	const std::size_t max_threads =
		(N + min_per_thread - 1) / min_per_thread;

	const std::size_t hardware_threads =
		std::thread::hardware_concurrency();

	const std::size_t num_threads =
		std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

	const std::size_t block_size = N / num_threads;

	std::vector < std::future < double > > futures(num_threads - 1);
	std::vector < std::thread >            threads(num_threads - 1);

	Threads_Guard guard(threads);

	double block_start = 0.0;

	for (std::size_t i = 0; i < (num_threads - 1); ++i)
	{
		block_start += block_size;

		std::packaged_task < double (const size_t) > task{ estimate_pi_seq_geom };

		futures[i] = task.get_future();
		threads[i] = std::thread(std::move(task), block_start);
	}

	double last_result = estimate_pi_seq_geom(block_start);

	double result = 0.0;

	for (std::size_t i = 0; i < (num_threads - 1); ++i)
	{
		result += futures[i].get();
	}

	result += last_result;
	result /= num_threads;

	time.stop();
	auto t = time.get_time();
	std::cout << "Time of parallel calculations: " << t << "\n\n";

	return result;
}

int main()
{
	const size_t N = 100000000;
	auto pi_seq = estimate_pi_seq_geom(N);
	auto pi_par = estimate_pi_par(N);
	std::cout << pi_seq << " " << pi_par << std::endl;
}

/*

RESULTS (std::thread::hardware_concurrency() == 2), without averaging:

N         | t_seq, c | t_par, c | pi_seq   | pi_par   |
10000     | 0.007588 | 0.020966 | 3.130400 | 3.124000 |
100000    | 0.025758 | 0.020468 | 3.134960 | 3.151200 |
1000000   | 0.223238 | 0.126706 | 3.140096 | 3.141040 |
10000000  | 2.15663  | 1.130630 | 3.141770 | 3.141403 |
100000000 | 21.5657  | 11.88688 | 3.141528 | 3.141508 |

*/