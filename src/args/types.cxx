#include <substrate/conversions>
#include "../args.hxx"

using namespace std::literals::string_view_literals;
using substrate::toInt_t;

namespace pcat::args
{
	argNode_t *argsTree_t::find(const argType_t nodeType) const noexcept
	{
		for (const auto &node : children_)
		{
			if (node->type() == nodeType)
				return node.get();
		}
		return nullptr;
	}

	bool argsTree_t::add(std::unique_ptr<argNode_t> &&node) noexcept try
		{ return bool{children_.emplace_back(std::move(node))}; }
	catch (std::bad_alloc &)
		{ return false; }

	argThreads_t::argThreads_t(const std::string_view threads) noexcept : argNode_t{argType_t::threads}
		{ threads_ = toInt_t<size_t>{threads.data(), threads.size()}.fromDec(); }

	argPinning_t::argPinning_t(const std::string_view threads) noexcept : argNode_t{argType_t::pinning}, cores_{}
	{
		try
		{
			for (size_t begin{}; begin < threads.length();)
			{
				const auto end{threads.find(',', begin)};
				const auto core{threads.substr(begin, end - begin)};
				if (end != std::string_view::npos && (end + 1) == threads.length())
					throw std::exception{};
				toInt_t<size_t> converter{core.data(), core.size()};
				if (!converter.isDec())
					throw std::exception{};
				cores_.emplace_back(converter.fromDec());
				begin = end == std::string_view::npos ? threads.length() : end + 1;
			}
		}
		catch (const std::exception &)
			{ cores_.clear(); }
	}

	argAlgorithm_t::argAlgorithm_t(const std::string_view algorithm) noexcept : argNode_t{argType_t::algorithm}
	{
		if (algorithm == "blockLinear"sv)
			algorithm_ = algorithm_t::blockLinear;
		else if (algorithm == "chunkSpans"sv)
			algorithm_ = algorithm_t::chunkSpans;
		else
			algorithm_ = algorithm_t::invalid;
	}
} // namespace pcat::args
