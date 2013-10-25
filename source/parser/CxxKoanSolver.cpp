#include "CxxKoanHtmlPrinter.hpp"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <fstream>
#include <string>


using namespace boost;
using namespace std;


namespace { // private

const regex lessThan{"<"};
const char lessThanReplacement[] = "&lt;";

const regex greaterThan{">"};
const char greaterThanReplacement[] = "&gt;";

const regex exerciseEnd{R"(\$)"};

} // namespace private


namespace cxxkoans {

std::string getKoanExerciseAnswer (const std::string & koanFilePath,
                                   const std::string & exerciseNumber,
                                   const std::string & inputNumber)
{
    ifstream koan_file_stream{koanFilePath};

    if (!koan_file_stream.is_open())
    {
        // TODO: do something to indicate failure
    }

    for (std::string line; std::getline(koan_file_stream, line); /*Purposefully empty*/)
    {
        smatch match_results;

        auto exerciseRegex = regex{string(R"(\$)") + exerciseNumber};
        auto inputRegex = regex{string("@") + inputNumber + ",([^@]*)@"};

        if (regex_match(line, match_results, exerciseRegex))
        {
            while (!regex_match(line, match_results, exerciseEnd))
            {
                if (regex_search(line, match_results, inputRegex))
                {
                    return match_results[1];
                }

                std::getline(koan_file_stream, line);
            }
            return "";
        }
    }
    return "";
}

} // namespace cxxkoans

