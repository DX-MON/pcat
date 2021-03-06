#ifndef AFFINITY__HXX
#define AFFINITY__HXX

#include <vector>
#include <thread>
#include <algorithm>
#include <system_error>
#include <stdexcept>
#include <sched.h>
#include "args.hxx"
#include "indexSequence.hxx"

namespace pcat
{
	struct affinity_t final
	{
	private:
		std::vector<uint32_t> processors{};

		void pinTo(const pthread_t thread, const std::size_t index) const
		{
			if (index >= processors.size())
				throw std::out_of_range{"index into thread affinity object too large"};
			cpu_set_t affinity{};
			CPU_SET(processors[index], &affinity);
			pthread_setaffinity_np(thread, sizeof(cpu_set_t), &affinity);
		}

	public:
		affinity_t()
		{
			const auto *const pinning{dynamic_cast<args::argPinning_t *>(::args->find(argType_t::pinning))};
			const auto *const threadCount{dynamic_cast<args::argThreads_t *>(::args->find(argType_t::threads))};
			cpu_set_t affinity{};
			if (sched_getaffinity(0, sizeof(cpu_set_t), &affinity) != 0)
				throw std::system_error{errno, std::system_category()};
			for (uint32_t i{0}; i < CPU_SETSIZE; ++i)
			{
				if (CPU_ISSET(i, &affinity))
				{
					if ((!threadCount || processors.size() < threadCount->threads()) &&
						(!pinning || std::find(pinning->begin(), pinning->end(), i) != pinning->end()))
						processors.push_back(i);
					CPU_CLR(i, &affinity);
				}
				else if (!CPU_COUNT(&affinity))
					break;
			}
		}

		[[nodiscard]] auto numProcessors() const noexcept { return processors.size(); }
		[[nodiscard]] auto begin() const noexcept { return processors.begin(); }
		[[nodiscard]] auto end() const noexcept { return processors.end(); }
		[[nodiscard]] auto indexSequence() const noexcept { return indexSequence_t{0, numProcessors()}; }

		void pinThreadTo(std::thread &thread, const std::size_t index) const
			{ pinTo(thread.native_handle(), index); }
		void pinThreadTo(const std::size_t index) const { pinTo(pthread_self(), index); }
	};
} // namespace pcat

#endif /*AFFINITY__HXX*/
