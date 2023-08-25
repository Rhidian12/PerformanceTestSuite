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

	return 0;
}