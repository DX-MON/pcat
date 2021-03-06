#include <cassert>
#include <string_view>
#include <array>
#include <random>
#include <utility>
#include <substrate/utility>
#include <crunch++.h>
#include <chunking.hxx>
#include <args.hxx>

using namespace std::literals::string_view_literals;
constexpr static std::size_t operator ""_uz(const unsigned long long value) noexcept { return value; }

std::vector<substrate::fd_t> pcat::inputFiles{};
substrate::fd_t pcat::outputFile{};
std::atomic<bool> pcat::sync{true};

using random_t = typename std::random_device::result_type;
using substrate::fd_t;
using substrate::normalMode;
using pcat::pageSize;
using pcat::transferBlockSize;
using pcat::inputFiles;
using pcat::outputFile;
using pcat::algorithm::chunkSpans::chunkedCopy;

constexpr auto totalHugeSize{std::size_t(transferBlockSize * 34U)};
constexpr auto hugefileSize{std::size_t(transferBlockSize * 32U) - 2048_uz};
constexpr static auto chunkFiles{substrate::make_array<std::pair<std::string_view, std::size_t>>(
{
	{"chunk1.test"sv, 1024_uz},
	{"chunk2.test"sv, 2048_uz},
	{"chunk3.test"sv, 3072_uz},
	{"chunk4.test"sv, std::size_t(transferBlockSize) - 4096_uz},
	{"chunk5.test"sv, std::size_t(transferBlockSize)},
	{"chunk6.test"sv, hugefileSize}
})};

class testChunking final : public testsuite
{
private:
	fd_t resultFile{"chunks.test", O_RDWR | O_CREAT | O_NOCTTY, normalMode};
	std::vector<fd_t> files{};

	void checkCopyResult()
	{
		std::array<char, pageSize> inputBlock{};
		std::array<char, pageSize> outputBlock{};
		const auto outputLength{outputFile.length()};
		off_t outputOffset{};
		assertTrue(outputFile.head());
		for (const auto &file : inputFiles)
		{
			assertTrue(file.head());
			const auto inputLength{file.length()};
			off_t inputOffset{};
			assertTrue(inputLength <= (outputLength - outputOffset));
			while (inputOffset < inputLength)
			{
				const auto amount{std::min(pageSize, inputLength - inputOffset)};
				assertTrue(file.read(inputBlock.data(), amount));
				assertTrue(outputFile.read(outputBlock.data(), amount));
				assertEqual(outputBlock.data(), inputBlock.data(), amount);
				inputOffset += amount;
				outputOffset += amount;
			}
		}
	}

	void testCopyNone()
	{
		inputFiles.clear();
		outputFile = resultFile.dup();
		assertEqual(outputFile.length(), 0);
		assertTrue(inputFiles.begin() == inputFiles.end());
		assertEqual(chunkedCopy(), 0);
	}

	void testCopySingle()
	{
		inputFiles.clear();
		inputFiles.emplace_back(files[5].dup());
		if (!resultFile.resize(hugefileSize))
			fail("Failed to resize the output test file");
		outputFile = resultFile.dup();
		assertEqual(outputFile.length(), hugefileSize);
		assertFalse(inputFiles.begin() == inputFiles.end());
		assertEqual(inputFiles.size(), 1);
		assertEqual(inputFiles[0].length(), hugefileSize);
		assertEqual(chunkedCopy(), 0);
		checkCopyResult();
	}

	void testCopyAll()
	{
		inputFiles.clear();
		inputFiles.emplace_back(files[0].dup());
		inputFiles.emplace_back(files[1].dup());
		inputFiles.emplace_back(files[2].dup());
		inputFiles.emplace_back(files[4].dup());
		inputFiles.emplace_back(files[3].dup());
		inputFiles.emplace_back(files[5].dup());
		if (!resultFile.resize(totalHugeSize))
			fail("Failed to resize the output test file");
		outputFile = resultFile.dup();
		assertEqual(outputFile.length(), totalHugeSize);
		assertFalse(inputFiles.begin() == inputFiles.end());
		assertEqual(inputFiles.size(), 6);
		assertEqual(inputFiles[0].length(), 1024);
		assertEqual(inputFiles[1].length(), 2048);
		assertEqual(inputFiles[2].length(), 3072);
		assertEqual(inputFiles[3].length(), transferBlockSize);
		assertEqual(inputFiles[4].length(), transferBlockSize - 4096);
		assertEqual(inputFiles[5].length(), hugefileSize);
		assertEqual(chunkedCopy(), 0);
		checkCopyResult();
	}

	void makeFile(const std::string_view fileName, const std::size_t size, const random_t seed) noexcept
	{
		const auto &file = files.emplace_back(fileName.data(), O_RDWR | O_CREAT | O_NOCTTY, normalMode);
		std::minstd_rand engine{seed};
		std::uniform_int_distribution<uint32_t> genRandom{};
		for (size_t i{}; i < size; i += sizeof(uint32_t))
			file.write(genRandom(engine));
		assert(file.head()); // NOLINT
	}

public:
	testChunking()
	{
		args = substrate::make_unique<pcat::args::argsTree_t>();
		std::random_device seed{};
		if (!resultFile.valid())
			throw std::logic_error{"Failed to create the output test file"};
		else if (!resultFile.resize(0))
			throw std::runtime_error{"Unable to reset output file size to 0"};
		for (const auto &file : chunkFiles)
			makeFile(file.first, file.second, seed());
	}

	testChunking(const testChunking &) = delete;
	testChunking(testChunking &&) = delete;
	testChunking &operator =(const testChunking &) = delete;
	testChunking &operator =(testChunking &&) = delete;

	~testChunking() final
	{
		pcat::inputFiles.clear();
		files.clear();
		for (const auto &file : chunkFiles)
			unlink(file.first.data());
		unlink("chunks.test");
	}

	void registerTests() final
	{
		CRUNCHpp_TEST(testCopyNone)
		CRUNCHpp_TEST(testCopySingle)
		CRUNCHpp_TEST(testCopyAll)
	}
};

CRUNCHpp_TESTS(testChunking)
