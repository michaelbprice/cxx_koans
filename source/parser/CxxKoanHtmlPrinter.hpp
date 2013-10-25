#ifndef __CXXKOANHTMLPRINTER_HPP__
#define __CXXKOANHTMLPRINTER_HPP__

#include <ostream>
#include <string>

namespace cxxkoans {

void printKoanAsHtml (const std::string & koanFilePath,
                      std::ostream & out,
                      const std::string & koan_name);

} // namespace cxxkoans

#endif // __CXXKOANHTMLPRINTER_HPP__
