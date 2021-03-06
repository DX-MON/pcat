#ifndef ALGORITHM_BLOCK_LINEAR_FILE_CHUNKER__HXX
#define ALGORITHM_BLOCK_LINEAR_FILE_CHUNKER__HXX

#include "chunkState.hxx"

namespace pcat::algorithm::blockLinear
{
	struct chunking_t final
	{
	private:
		inputFilesIterator_t file{inputFiles.begin()};
		off_t inputLength{file == inputFiles.end() ? 0 : file->length()};
		mappingOffset_t inputOffset{0, blockLength(inputLength)};
		const off_t outputLength{outputFile.length()};
		mappingOffset_t outputOffset{};

		constexpr void nextInputBlock() noexcept
		{
			inputOffset += inputOffset.length();
			if (inputOffset.offset() == inputLength)
			{
				assert(file != inputFiles.end()); // NOLINT
				++file;
				inputLength = file == inputFiles.end() ? 0 : file->length();
				inputOffset = {};
			}
			inputOffset.length(blockLength(inputLength - inputOffset));
		};

	public:
		chunking_t() noexcept { outputOffset.length(blockLength(outputLength - outputOffset)); }
		chunking_t(const inputFilesIterator_t file_) noexcept : file{file_}, outputOffset{outputLength} { }
		[[nodiscard]] chunkState_t subchunkState() const noexcept
			{ return {file, inputLength, inputOffset, outputOffset}; }
		chunkState_t operator *() const noexcept { return subchunkState(); }

		void operator ++() noexcept
		{
			if (outputOffset == outputLength)
				return;
			else if (outputOffset.length() != inputOffset.length())
			{
				const auto state{subchunkState().end()};
				file = state.file();
				inputLength = state.inputLength();
				inputOffset = state.inputOffset();
			}
			nextInputBlock();
			outputOffset += outputOffset.length();
			outputOffset.length(blockLength(outputLength - outputOffset));
		}

		bool operator ==(const chunking_t &other) const noexcept
		{
			return file == other.file &&
				inputOffset == other.inputOffset &&
				outputOffset == other.outputOffset;
		}
		bool operator !=(const chunking_t &other) const noexcept { return !(*this == other); }
	};

	// begin() and end() are non-static to fufill the requirements of for-each looping
	struct fileChunker_t final
	{
		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[nodiscard]] chunking_t begin() const noexcept { return {}; }
		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[nodiscard]] chunking_t end() const noexcept { return {inputFiles.end()}; }
	};
} // namespace pcat::algorithm::blockLinear

#endif /*ALGORITHM_BLOCK_LINEAR_FILE_CHUNKER__HXX*/
