#include "CxxKoanServer.hpp"

#include "cpprest/http_listener.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

using web::http::experimental::listener::http_listener;
using namespace web::http;
using namespace web;

using namespace boost;
using namespace std;

namespace { // private

const char kKoansPrefix[] = "/koans";

unordered_map<string, string> extensionsToContentTypeMap = {{"html", "text/html"},
                                                            {"js", "application/javascript"},
                                                            {"css", "text/css"}};

} // namespace private


namespace cxxkoans {

Server::Server (const std::string & listenerUrl,
                const std::string & staticResourceDirectory,
                const std::string & koanDirectory)
  : m_staticResourceDirectory{staticResourceDirectory}
  , m_koanDirectory{koanDirectory}
  , m_listener{listenerUrl}
{
    m_listener.support(methods::GET, [this] (http_request request)
    {
        auto resource = request.request_uri().resource().to_string();

        if (resource.compare(0, 6, kKoansPrefix) == 0) // TODO: Avoid hardcoded string length
        {
            cout << "No koans yet: " << resource << endl;
            request.reply(status_codes::NotFound, U("No Koans Yet"));
        }
        else
        {
            auto resource_path = filesystem::path{m_staticResourceDirectory} / resource;

            ifstream resource_stream{resource_path.string()};
            ostringstream resource_contents_stream;

            copy(istreambuf_iterator<char>(resource_stream), istreambuf_iterator<char>(),
                 ostreambuf_iterator<char>(resource_contents_stream));

            http_response response(status_codes::OK);

            auto extension_iter = resource.begin() + resource.rfind(".");
            auto extension = string{&(*++extension_iter)};

            string & contentType = extensionsToContentTypeMap[extension];

            contentType.empty() ? response.headers().set_content_type("text/plain")
                                : response.headers().set_content_type(contentType);

            response.set_body(resource_contents_stream.str());
            request.reply(response);
        }

    });

}

Server::~Server ()
{ }

void Server::start ()
{ 
    m_listener.open().wait();
}

void Server::stop ()
{
    m_listener.close().wait();
}


} // namespace cxxkoans

