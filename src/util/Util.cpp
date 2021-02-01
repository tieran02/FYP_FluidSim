#include "Util.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::string Util::ReadFile(const std::string& path)
{
	std::ifstream file;
	file.exceptions (std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		file.open(path);
		std::stringstream stream;
		stream << file.rdbuf();
		return stream.str();
	}
	catch(std::ifstream::failure& e)
	{
		std::cout << e.code() << "	" << e.what() << std::endl;
		throw;
	}

}
