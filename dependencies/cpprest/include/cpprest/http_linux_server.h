/*
* Copyright (c) Microsoft Corporation. All rights reserved. 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <set>
#include "pplx/threadpool.h"
#include "http_server.h"

using boost::asio::ip::tcp;
using namespace boost::asio;

#define FAILED(x) ((x) != 0)

namespace web
{

namespace http
{

namespace experimental
{

namespace listener
{

class http_linux_server;

namespace details
{


struct linux_request_context : web::http::details::_http_server_context
{
    linux_request_context(){}

    pplx::task_completion_event<void> m_response_completed;

private:
    linux_request_context(const linux_request_context&);
    linux_request_context& operator=(const linux_request_context&);
};

class hostport_listener;

class connection
{
private:
    std::unique_ptr<tcp::socket> m_socket;
    boost::asio::streambuf m_request_buf;
    boost::asio::streambuf m_response_buf;
    http_linux_server* m_p_server;
    hostport_listener* m_p_parent;
    http_request m_request;
    size_t m_read, m_write;
    size_t m_read_size, m_write_size;
    bool m_close;
    bool m_chunked;
    std::atomic<int> m_refs; // track how many threads are still referring to this
    
public:
    connection(std::unique_ptr<tcp::socket> socket, http_linux_server* server, hostport_listener* parent)
    : m_socket(std::move(socket))
    , m_request_buf()
    , m_response_buf()
    , m_p_server(server)
    , m_p_parent(parent)
    , m_refs(1)
    {
        start_request_response();
    }

    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    void close();

private:
    void start_request_response();
    void handle_http_line(const boost::system::error_code& ec);
    void handle_headers();
    void handle_body(const boost::system::error_code& ec);
    void handle_chunked_header(const boost::system::error_code& ec);
    void handle_chunked_body(const boost::system::error_code& ec, int toWrite);
    void dispatch_request_to_listener();
    void request_data_avail(size_t size);
    void do_response(bool bad_request=false);
    template <typename ReadHandler>
    void async_read_until_buffersize(size_t size, ReadHandler handler);
    void async_process_response(http_response response);
    void cancel_sending_response_with_error(http_response response, std::exception_ptr);
    void handle_headers_written(http_response response, const boost::system::error_code& ec);
    void handle_write_large_response(http_response response, const boost::system::error_code& ec);
    void handle_write_chunked_response(http_response response, const boost::system::error_code& ec);
    void handle_response_written(http_response response, const boost::system::error_code& ec);
    void finish_request_response();
};

class hostport_listener
{
private:
    friend class connection;

    std::unique_ptr<tcp::acceptor> m_acceptor;
    std::map<std::string, http_listener* > m_listeners;
    pplx::extensibility::reader_writer_lock_t m_listeners_lock;

    pplx::extensibility::recursive_lock_t m_connections_lock;
    pplx::extensibility::event_t m_all_connections_complete;
    std::set<connection*> m_connections;

    http_linux_server* m_p_server;

    std::string m_host;
    std::string m_port;
    
public:
     hostport_listener(http_linux_server* server, const std::string& hostport)
    : m_acceptor()
    , m_listeners()
    , m_listeners_lock()
    , m_connections_lock()
    , m_connections()
    , m_p_server(server)
    {
        m_all_connections_complete.set();

        std::istringstream hostport_in(hostport);
        
        std::getline(hostport_in, m_host, ':');
        std::getline(hostport_in, m_port);
    }

    ~hostport_listener()
    {
        stop();
    }

    void start();
    void stop();

    void add_listener(const std::string& path, http_listener* listener);
    void remove_listener(const std::string& path, http_listener* listener);

private:
    void on_accept(ip::tcp::socket* socket, const boost::system::error_code& ec);
    
};


}

using namespace http::experimental::listener::details;

struct iequal_to
{
    bool operator()(const std::string& left, const std::string& right) const
    {
        return boost::ilexicographical_compare(left, right);
    }
};

class http_linux_server : public web::http::experimental::details::http_server
{
private:
    friend class http::experimental::listener::details::connection;

    pplx::extensibility::reader_writer_lock_t m_listeners_lock;
    std::map<std::string, std::unique_ptr<hostport_listener>, iequal_to> m_listeners;
    std::unordered_map<http_listener*, std::unique_ptr<pplx::extensibility::reader_writer_lock_t>> m_registered_listeners;
    bool m_started;

public:
    http_linux_server()
    : m_listeners_lock()
    , m_listeners()
    , m_started(false)
    {}

    ~http_linux_server()
    {
        stop();
    }

    virtual pplx::task<void> start();
    virtual pplx::task<void> stop();

    virtual pplx::task<void> register_listener(http_listener* listener);
    virtual pplx::task<void> unregister_listener(http_listener* listener);

    pplx::task<void> respond(http::http_response response);
};

}
}
}
}
