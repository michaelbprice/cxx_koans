#include "CxxKoanHtmlPrinter.hpp"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <fstream>
#include <ostream>
#include <string>


using namespace boost;
using namespace std;


namespace { // private

const char kHtmlDocType[] = "<!DOCTYPE html>";

const char kHtmlTagOpen[] = "<html>";
const char kHtmlTagClose[] = "</html>";

const char kHtmlBodyTagOpen[] = "<body>";
const char kHtmlBodyTagClose[] = "</body>";

const char kHtmlDivKoan[] = R"(<div class="koans">)";
const char kHtmlDivText[] = R"(<div class="text">)";
const char kHtmlDivTagClose[] = "</div>";

const char kHtmlHeader[] = R"(
  <head>
    <link rel="stylesheet" type="text/css" href="/resources/koans.css" />
    <script type="text/javascript" src="/resources/koans.js"></script>
  </head>)";

const char kHtmlBreakLine[] = "<br/>";

const regex exerciseBegin{R"(\$(\d))"};
const char exerciseBeginReplacement[] = R"(<div class="failing" id="$1" />)";
const regex exerciseEnd{R"(\$)"};

const regex input{R"(@(\d+),([^@]*)@)"};
const char inputReplacement[] = R"(<input type="text" name="koan_?_$1" onchange="evaluateExercise(this.parentNode);" onkeyup="solveInput(this, event);" />)";

const regex output{"@!"};

} // namespace private


namespace cxxkoans {

void printKoanAsHtml (const std::string & koanFilePath,
                      std::ostream & out,
                      const std::string & koan_name)
{
    ifstream koan_file_stream{koanFilePath};

    if (!koan_file_stream.is_open())
    {
        // TODO: do something to indicate failure
    }

    out << kHtmlDocType << endl;
    out << kHtmlTagOpen << endl;
    out << kHtmlHeader << endl;
    out << kHtmlBodyTagOpen << endl;
    out << "<h2>" << koanFilePath << "</h2>" << endl;
    out << R"(<div class="koans" id=")" << koan_name << R"(">)" << endl;;

    string exerciseNumber;
    string inputNumber;

    for (std::string line; std::getline(koan_file_stream, line); /*Purposefully empty*/)
    {
        smatch match_results;

        if (regex_match(line, match_results, exerciseBegin))
        {
            exerciseNumber = match_results[1];
            out << regex_replace(line, exerciseBegin, exerciseBeginReplacement) << endl;
        }
        else if (regex_match(line, match_results, exerciseEnd))
        {
            exerciseNumber.clear();
            out << regex_replace(line, exerciseEnd, kHtmlDivTagClose) << endl;
        }
        else if (regex_search(line, match_results, input))
        {
            inputNumber = match_results[1]; 
            out << match_results.prefix();
            out << R"(<input type="text" name="koan_)" << exerciseNumber << "_" << match_results[1] << R"(" onchange="evaluateExercise(this.parentNode);" onkeyup="solveInput(this, event);" />)";
            out << match_results.suffix();
        }
/*
        else if (regex_search(line, match_results, output))
        {
            out << R"(<input type="text" name="output_)" << exerciseNumber << "_" << inputNumber << R"(" value="Output will appear here" readonly />)" << kHtmlBreakLine << endl;
        }
*/
        else
        {
            out << line << kHtmlBreakLine << endl;
        }
    }

    out << kHtmlDivTagClose << endl;
    out << kHtmlBodyTagClose << endl;
    out << kHtmlTagClose << endl;
}

} // namespace cxxkoans

