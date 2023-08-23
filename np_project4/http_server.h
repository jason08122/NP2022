#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <dirent.h>
#include<unistd.h>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include<sys/wait.h>

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
    public:
        session(tcp::socket socket);
        void start();
        static void set_io_context(boost::asio::io_context* _io_context);
    
    private:
        void do_read();
        void do_write(std::size_t length);
        void do_cgi();
        void Setenv();
        void Printenv();
        void exe_cgi();

        tcp::socket socket_;
        enum { max_length = 1024 };
        char data_[max_length];
        static boost::asio::io_context* _io_context;

        std::string status_str;
        std::string REQUEST_METHOD;
        std::string REQUEST_URL;
        std::string QUERY_STRING;
        std::string SERVER_PROTOCOL;
        std::string trash;
        std::string HTTP_HOST;
        std::string SERVER_ADDR;
        std::string SERVER_PORT;
        std::string REMOTE_ADDR;
        std::string REMOTE_PORT;
        std::string EXEC_FILE;

};

class server
{
    public:
        server(boost::asio::io_context& io_context, short port);

    private:
        void do_accept();

        tcp::acceptor acceptor_;
};