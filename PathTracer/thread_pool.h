#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class thread_pool {
	public:
		void Start() {
			do_terminate = false;

			const uint32_t num_threads = std::thread::hardware_concurrency();
			threads.resize(num_threads);

			// Initialize the maximum number of concurrent threads
			for (uint32_t i = 0; i < num_threads; ++i) {
				threads.at(i) = std::thread([this] { ThreadLoop(); });
			}
		}

		void Start(int _print_condition, int _jobs_total) {
			// Set conditions for printing
			do_print = true;
			print_condition = _print_condition;
			jobs_total = _jobs_total;
			jobs_completed = 0;

			Start();
		}

		void Stop() {
			{
				std::unique_lock<std::mutex> lock(jobs_mutex);
				do_terminate = true;
			}

			// Notify all threads now that they know to stop
			wait_condition.notify_all();
			for (std::thread& thread : threads) {
				thread.join();
			}
			threads.clear();
		}

		void QueueJob(const std::function<void()>& job) {
			// Add a new job and alert a thread
			{
				std::unique_lock<std::mutex> lock(jobs_mutex);
				jobs.push(job);
			}
			wait_condition.notify_one();
		}

		bool IsBusy() {
			std::unique_lock<std::mutex> lock(jobs_mutex);
			return !jobs.empty();
		}

	private:
		void ThreadLoop() {
			while (true) {
				std::function<void()> job;

				{
					// Wait for a job to be available or for a terminating call
					std::unique_lock<std::mutex> lock(jobs_mutex);
					wait_condition.wait(lock, [this] {
						return !jobs.empty() || do_terminate;
					});

					if (do_terminate) return;

					// Choose and run the next job
					job = jobs.front();
					jobs.pop();
				}

				job();

				if (do_print) {
					std::unique_lock<std::mutex> lock(completed_mutex);
					++jobs_completed;

					if (jobs_completed % print_condition == 0) {
						printf("%f%%\n", 100 * float(jobs_completed) / jobs_total);
					}
				}
			}
		}

	private:
		std::vector<std::thread> threads;

		std::queue<std::function<void()>> jobs;
		std::mutex jobs_mutex;

		std::condition_variable wait_condition;

		uint32_t jobs_total = 0;
		uint32_t jobs_completed = 0;
		std::mutex completed_mutex;

		bool do_terminate = false;
		bool do_print = false;

		uint32_t print_condition;	// Print the completion percent every {print_condition} tasks
};