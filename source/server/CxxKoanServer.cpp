#include "CxxKoanServer.hpp"
#include "CxxKoanHtmlPrinter.hpp"
#include "CxxKoanSolver.hpp"

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
const char kSolvePrefix[] = "/solve";
const char kEvaluatePrefix[] = "/evaluate";

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

    m_listener.support(methods::POST, [this] (http_request request)
    {
        auto resource = request.request_uri().resource().to_string();

        if (resource.compare(0, 6, kSolvePrefix) == 0) // TODO: Avoid hardcoded string length
        {
            json::value jsonData = request.extract_json().get();

            string koan_name = resource.substr(6);

            http_response response(status_codes::OK);

            auto koan_path = filesystem::path{m_koanDirectory} / koan_name;

            string answer = getKoanExerciseAnswer(koan_path.string(), jsonData["exercise"].as_string(), jsonData["input"].as_string());

            response.headers().set_content_type("text/plain");
            response.set_body(answer);
            request.reply(response);
        }
        else if (resource.compare(0, 9, kEvaluatePrefix) == 0) // TODO: Avoid hardcoded string length
        {
            json::value jsonData = request.extract_json().get();

            string koan_name = resource.substr(9);

            auto koan_path = filesystem::path{m_koanDirectory} / koan_name;

            int i = 1;
            for (auto value : jsonData["inputs"])
            {
                string answer = getKoanExerciseAnswer(koan_path.string(), jsonData["exercise"].as_string(), to_string(i));

                if (answer.compare(value.second.as_string()) != 0)
                {
                    request.reply(status_codes::NotFound, "NotFound");
                    return;
                }

                ++i;
            }

            request.reply(status_codes::OK, "OK");
        }
    });

    m_listener.support(methods::GET, [this] (http_request request)
    {
        auto resource = request.request_uri().resource().to_string();

        if (resource.compare(0, 6, kKoansPrefix) == 0) // TODO: Avoid hardcoded string length
        {
            string koan_name = resource.substr(6);

            http_response response(status_codes::OK);

            auto koan_path = filesystem::path{m_koanDirectory} / koan_name;

            ostringstream koan_contents_stream;
            printKoanAsHtml(koan_path.string(), koan_contents_stream);

            response.headers().set_content_type("text/html");
            response.set_body(koan_contents_stream.str());
            request.reply(response);
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

