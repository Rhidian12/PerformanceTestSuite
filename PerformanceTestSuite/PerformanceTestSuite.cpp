#include "Timer.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

/*
command line format:
	PerformanceTestSuite.exe [--iterations N | -i N] --commands <cmd_1> <cmd_2> <cmd_N> --wdirectories <wdir_1> <wdir_2> <wdir_N> --args "<args_1>" "<args_2>" "<args_N>"

command line options:
	--iterations N      number of times to run commands
	--commands          list of commands to execute, these can be absolute paths to .exe files or cmd line commands to execute
	--wdirectories      list of working directories from which to execute commands. Working Directories indices are the same as Command Indices.
							Current Working directory can be selected as ".", there must be as many working directories as commands
	--args				command line arguments to pass to the respective commands. No command line arguments is selected as '.'
	-i N                number of times to run commands
*/

#ifdef _DEBUG
#define ASSERT(expr) if (!(expr)) std::cout << "Assertion triggered: " << #expr << " at line " << __LINE__ << " in file " << __FILE__ << "\n"; \
						__debugbreak();
#else
#define ASSERT(expr)
#endif

namespace
{
	static void PrintHelp()
	{
		std::cout << "Command Line Format:\n";
		std::cout << "PerformanceTestSuite.exe [--iterations N | -i N] --commands <cmd_1> <cmd_2> <cmd_N> --wdirectories <wdir_1> <wdir_2> <wdir_N> --args \"<args_1>\" \"<args_2>\" \"<args_N>\"\n\n";

		std::cout << "command line options :\n";
		std::cout << "--iterations [N]		number of times to run commands\n";
		std::cout << "--commands			list of commands to execute, these can be absolute paths to .exe files or cmd line commands to execute\n";
		std::cout << "--wdirectories		list of working directories from which to execute commands. Working Directories indices are the same as Command Indices. Current Working directory can be selected as '.', there must be as many working directories as commands\n";
		std::cout << "--args				command line arguments to pass to the respective commands. No command line arguments is selected as '.'\n";
		std::cout << "-i					number of times to run commands\n";

		std::cout << "Example: PerformanceTestSuite.exe -i 10 --commands D:\\Hello_World.exe findstr\n";
	}

	static bool IsArgDigit(char* Arg)
	{
		while (*Arg != '\0')
		{
			if (!std::isdigit(*Arg++)) return false;
		}

		return true;
	}

	static int ParseCmdLine(int Argc, char* Argv[], int& Iterations, std::vector<std::string>& Commands, std::vector<std::string>& WorkingDirectories, std::vector<std::string>& CommandArgs)
	{
		for (int i{ 1 }; i < Argc; ++i)
		{
			const std::string CurrentArg{ Argv[i] };

			if (CurrentArg == "--iterations" || CurrentArg == "-i")
			{
				if (!IsArgDigit(Argv[++i]))
				{
					return 1;
				}

				Iterations = std::stoi(Argv[i]);
			}
			else if (CurrentArg == "--commands")
			{
				std::string Arg{ Argv[++i] };
				while (!Arg.starts_with("-"))
				{
					Commands.push_back(Arg);

					if (i < Argc - 1)
					{
						Arg = Argv[++i];
					}
				}

				--i; // we've been manually iterating through the args, and only stop when we found the next cmd arg, so go back one to let the for-loop organically find it :)
			}
			else if (CurrentArg == "--wdirectories")
			{
				std::string Arg{ Argv[++i] };
				while (!Arg.starts_with("-"))
				{
					WorkingDirectories.push_back(Arg);

					if (i < Argc - 1)
					{
						Arg = Argv[++i];
					}
				}

				--i;
			}
			else if (CurrentArg == "--args")
			{
				std::string Arg{ Argv[++i] };
				while (Arg.starts_with("\""))
				{
					CommandArgs.push_back(Arg.substr(1, Arg.find_last_of('"')));

					if (i < Argc - 1)
					{
						Arg = Argv[++i];
					}
				}

				--i;
			}
		}

		return 0;
	}

	template<typename T>
	constexpr static T GetAverage(const std::vector<T>& v)
	{
		T average{};

		for (const T value : v)
		{
			average += value;
		}

		return average / static_cast<T>(v.size());
	}

	template<typename T>
	constexpr static T GetMedian(const std::vector<T>& v)
	{
		const T size{ static_cast<T>(v.size()) };
		const T medianIndex{ (size + 1) / 2 };

		if (size % 2 == 0)
		{
			return (v[medianIndex] + v[medianIndex + 1]) / 2;
		}
		else
		{
			return v[medianIndex];
		}
	}
}

int RunCmdLine(int Argc, char* Argv[]);

int main(int Argc, char* Argv[])
{
	return RunCmdLine(Argc, Argv);
}

int RunCmdLine(int Argc, char* Argv[])
{
	constexpr uint8_t MIN_NR_OF_ARGS{ 5 };

	const uint8_t ActualNrOfArgs{ static_cast<uint8_t>(Argc - 1) };
	if (ActualNrOfArgs < MIN_NR_OF_ARGS)
	{
		std::cout << "Not enough arguments\n";
		PrintHelp();
		return 1;
	}

	int Iterations{};
	std::vector<std::string> Commands{}, WorkingDirectories{}, CommandArgs{};
	if (ParseCmdLine(Argc, Argv, Iterations, Commands, WorkingDirectories, CommandArgs) != 0)
	{
		std::cout << "Wrong usage of arguments\n";
		PrintHelp();
		return 1;
	}

	ASSERT(Commands.size() == WorkingDirectories.size());
	using namespace Time;

	std::vector<std::vector<uint32_t>> Times{};
	for (size_t i{}; i < Commands.size(); ++i)
	{
		Times.push_back(std::vector<uint32_t>{});
	}

	for (int i{}; i < Iterations; ++i)
	{
		for (size_t j{}; j < Commands.size(); ++j)
		{
			std::string Command{ "cd " + WorkingDirectories[j] + " " + Commands[j] };
			if (j < CommandArgs.size() && CommandArgs[j] != ".")
			{
				Command += " " + CommandArgs[j];
			}

			Command += " > nul";

			const Timepoint t1{ Timer::GetInstance().Now() };
			system(Command.c_str());
			const Timepoint t2{ Timer::GetInstance().Now() };

			Times[j].push_back((t2 - t1).Count<TimeLength::MilliSeconds, uint32_t>());
		}
	}

	int nrOfElementsToRemove{ Iterations / 10 / 2 };
	if (nrOfElementsToRemove == 0)
	{
		nrOfElementsToRemove = 1;
	}

	for (std::vector<uint32_t>& v : Times)
	{
		std::sort(v.begin(), v.end());
		v.erase(v.begin(), v.begin() + nrOfElementsToRemove);
		v.erase(v.begin() + v.size() - nrOfElementsToRemove, v.end());
	}

	std::cout << "\n\nNr Of Iterations: " << Iterations << "\n";

	for (size_t i{}; i < Commands.size(); ++i)
	{
		std::cout << Commands[i] << " Times:\n\n";

		std::cout << "Average (ms): " << GetAverage(Times[i]) << "\n";
		std::cout << "Median (ms): " << GetMedian(Times[i]) << "\n";

		std::cout << "\n========================\n";
	}

	return 0;
}