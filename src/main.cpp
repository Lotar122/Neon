#include "Classes/Logger/Logger.hpp"

#include <functional>
#include <array>
#include <queue>
#include <mutex>
#include <utility>

struct Task
{
	std::function<void(int)> calle;
	int param;
};

class Queue
{
public:
	std::deque<Task> recordingQueue;
	std::deque<Task> executionQueue;

	std::mutex recordingMtx;
	std::mutex executionMtx;

	void appendTask(Task task)
	{
		std::unique_lock<std::mutex> lock(recordingMtx);
		recordingQueue.push_back(task);
	}

	void execute()
	{
		std::unique_lock<std::mutex> recordingLock(recordingMtx);
		std::unique_lock<std::mutex> executionLock(executionMtx);

		std::swap(recordingQueue, executionQueue);

		recordingLock.unlock();

		while (!executionQueue.empty())
		{
			executionQueue.front().calle(executionQueue.front().param);
			executionQueue.pop_front();
		}
	}
};

void logNum(int a)
{
	Carbo::Logger::Log("Num: {}", a);
}

int main()
{
	Queue queue;

	std::atomic<int> counter;
	counter.store(0);

	std::thread t1([&queue, &counter]() {
		while (true)
		{
			int counterValue = counter.fetch_add(1);
			queue.appendTask(Task{ &logNum, counterValue });
		}
	});

	std::thread t2([&queue, &counter]() {
		while (true)
		{
			int counterValue = counter.fetch_add(1);
			queue.appendTask(Task{ &logNum, counterValue });
		}
	});

	std::thread t3([&queue, &counter]() {
		while (true)
		{
			int counterValue = counter.fetch_add(1);
			queue.appendTask(Task{ &logNum, counterValue });
		}
	});

	while (true)
	{
		queue.execute();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}