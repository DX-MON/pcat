#ifndef ALGORITHM_CHUNK_SPANS_CHUNK_STATE__HXX
#define ALGORITHM_CHUNK_SPANS_CHUNK_STATE__HXX

#include <cassert>
#include "mappingOffset.hxx"

namespace pcat::algorithm::chunkSpans
{
	struct chunkState_t final
	{
	private:
		inputFilesIterator_t file_{};
		off_t inputLength_{};
		mappingOffset_t inputOffset_{};
		mappingOffset_t outputOffset_{};

		constexpr void nextInputBlock() noexcept
		{
			inputOffset_ += inputOffset_.length();
			if (inputOffset_.offset() == inputLength_)
			{
				assert(file_ != inputFiles.end()); // NOLINT
				++file_;
				inputLength_ = file_ == inputFiles.end() ? 0 : file_->length();
				inputOffset_ = {};
			}
		};

	public:
		chunkState_t() noexcept = default;
		constexpr chunkState_t(const inputFilesIterator_t &file, const off_t inputLength,
			const mappingOffset_t &inputOffset, const mappingOffset_t &outputOffset) noexcept :
			file_{file}, inputLength_{inputLength}, inputOffset_{inputOffset}, outputOffset_{outputOffset} { }
		constexpr chunkState_t(const chunkState_t &) noexcept = default;
		constexpr chunkState_t(chunkState_t &&) noexcept = default;
		chunkState_t &operator =(const chunkState_t &) noexcept = default;
		chunkState_t &operator =(chunkState_t &&) noexcept = default;
		~chunkState_t() noexcept = default;
		[[nodiscard]] constexpr const inputFilesIterator_t &file() const noexcept { return file_; }
		[[nodiscard]] constexpr off_t inputLength() const noexcept { return inputLength_; }
		[[nodiscard]] const fd_t &inputFile() const noexcept { return *file_; }
		[[nodiscard]] constexpr const mappingOffset_t &inputOffset() const noexcept { return inputOffset_; }
		[[nodiscard]] constexpr const mappingOffset_t &outputOffset() const noexcept { return outputOffset_; }

		[[nodiscard]] constexpr chunkState_t end() const noexcept
		{
			chunkState_t state{*this};
			while (!state.atEnd())
				++state;
			return state;
		}

		[[nodiscard]] constexpr bool atEnd() const noexcept
			{ return !outputOffset_.length(); }

		constexpr void operator ++() noexcept
		{
			if (atEnd())
				return;
			const off_t remainder = outputOffset_.length() - inputOffset_.length();
			outputOffset_ += inputOffset_.length();
			outputOffset_.length(remainder);
			nextInputBlock();
			inputOffset_.length(std::min(transferBlockSize, std::min(remainder, inputLength_)));
		}

		bool operator ==(const chunkState_t &other) const noexcept
		{
			return file_ == other.file_ &&
				inputOffset_ == other.inputOffset_ &&
				outputOffset_ == other.outputOffset_;
		}
		bool operator !=(const chunkState_t &other) const noexcept { return !(*this == other); }
	};
} // namespace pcat::algorithm::blockLinear

#endif /*ALGORITHM_CHUNK_SPANS_CHUNK_STATE__HXX*/
