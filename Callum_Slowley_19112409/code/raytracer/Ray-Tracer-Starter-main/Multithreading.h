/* source from Ryan Westwood starts here: */
#pragma once
#include <condition_variable>
#include <functional>
#include <vector>
#include <thread>
#include <queue>

class ThreadPool
{
public:
	using Task = std::function<void()>;
	ThreadPool(short threadCount) {
		Start(threadCount);
	}
	~ThreadPool() {
		Stop();
	}

	void Enqueue(Task task) {
		{
			std::unique_lock<std::mutex>lock(mMutex);
			mTask.emplace(std::move(task));
		}
		mCondition.notify_one();
	}
private:
	std::vector<std::thread>mThreads;
	std::condition_variable mCondition;
	std::mutex mMutex;
	std::queue<Task> mTask;

	bool mRunning = false;

	void Start(short threadCount) {
		for (short i = 0; i < threadCount; i++) {
			mThreads.emplace_back([=] {
				while (true)
				{
					Task task;
					{
						std::unique_lock<std::mutex>lock(mMutex);

						mCondition.wait(lock, [=] {return mRunning || !mTask.empty(); });

						if (mRunning && mTask.empty())
							break;
						task = std::move(mTask.front());
						mTask.pop();
					}
					task();
				}
				});
		}
	}
	void Stop() {
		{
			std::unique_lock<std::mutex>lock(mMutex);
			mRunning = true;
		}

		mCondition.notify_all();

		for (auto& thread : mThreads)
			thread.join();
	}
};
/*source from Ryan Westwood ends here*/
