#ifndef __CXXKOANSOLVER_HPP__
#define __CXXKOANSOLVER_HPP__

#include <ostream>
#include <string>

namespace cxxkoans {

std::string getKoanExerciseAnswer (const std::string & koanFilePath,
                                   const std::string & exerciseNumber,
                                   const std::string & inputNumber);

} // namespace cxxkoans

#endif // __CXXKOANSOLVER_HPP__
