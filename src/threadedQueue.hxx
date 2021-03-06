#ifndef THREADED_QUEUE__HXX
#define THREADED_QUEUE__HXX

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <utility>

namespace pcat
{
	template<typename T> struct threadedQueue_t final
	{
	private:
		std::queue<T> queue{};
		std::mutex queueMutex{};
		std::condition_variable haveData{};
		std::atomic<size_t> queueLength{0};

	public:
		template<typename... args_t> void emplace(args_t &&...args)
		{
			std::lock_guard<std::mutex> lock{queueMutex};
			queue.emplace(std::forward<args_t>(args)...);
			++queueLength;
			haveData.notify_one();
		}

		void push(T &&value)
		{
			std::lock_guard<std::mutex> lock{queueMutex};
			queue.push(std::move(value));
			++queueLength;
			haveData.notify_one();
		}

		[[nodiscard]] auto pop()
		{
			std::unique_lock<std::mutex> lock{queueMutex};
			if (!queueLength)
				haveData.wait(lock, [this]() noexcept -> bool { return queueLength; });
			--queueLength;
			const auto result{queue.front()};
			queue.pop();
			return result;
		}

		[[nodiscard]] auto empty() const noexcept { return !queueLength; }
		[[nodiscard]] std::size_t size() const noexcept { return queueLength; }
	};
} // namespace pcat

#endif /*THREADED_QUEUE__HXX*/
