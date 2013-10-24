#ifndef __CXXKOANSERVER_HPP__
#define __CXXKOANSERVER_HPP__

#include "cpprest/http_listener.h"

#include <string>

namespace cxxkoans {

class Server final
{
 public:

    Server (const std::string & listenerUrl,
            const std::string & staticResourceDirectory,
            const std::string & koanDirectory);

    ~Server ();

    void start ();
    void stop ();

 private:

    const std::string m_staticResourceDirectory;
    const std::string m_koanDirectory;

    web::http::experimental::listener::http_listener m_listener;

    Server (const Server &) = delete;
    Server (Server &&) = delete;
    Server& operator= (const Server &) = delete;
    Server& operator= (Server &&) = delete; 

};


} // namespace cxxkoans

#endif // __CXXKOANSERVER_HPP__

