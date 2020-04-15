#include <cstring>
#include <array>
#include <string_view>
#include <substrate/fd>
#include <substrate/mmap>
#include <substrate/utility>
#include <substrate/units>
#include <substrate/console>
#include <version.hxx>
#include "args.hxx"

using std::literals::string_view_literals::operator ""sv;
using substrate::console;

namespace pcat
{
	using substrate::mmap_t;
	using substrate::operator ""_KiB;

	constexpr static auto options{substrate::make_array<args::option_t>(
	{
		{"--version"sv, argType_t::version},
		{"--help"sv, argType_t::help},
		{"--output"sv, argType_t::outputFile},
		{"-o"sv, argType_t::outputFile}
	})};

	constexpr static size_t pageSize = 4_KiB;
}

int main(int argCount, char **argList)
{
	console = {stdout, stderr};
	if (!parseArguments(argCount, argList, pcat::options))
	{
		console.error("Failed to parse arguments"sv); // NOLINT(readability-magic-numbers)
		return 1;
	}
	if (args->find(argType_t::version))
		return pcat::versionInfo::printVersion();
	dumpAST();
	return 0;
}
