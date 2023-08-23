#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using boost::asio::ip::tcp;

class client
{
    public:
        client(boost::asio::io_context &io_context, std::string addr, int port, std::string file);

        int server_port;
        std::string server_addr;
        std::string testfile;
        boost::asio::ip::tcp::socket socket;
        char data[10240];
};

class console
{
    public:
        console();

        void set_wb_sock(boost::asio::ip::tcp::socket* socket);
        void set_query(std::string query_str);
        void init_clients();
        void connect_server();
        void Run();

    private:
        void init_html();
        void shell_input(int session, std::vector<std::string> input); // to do
        void shell_output(int session, std::string content); // to do
        void get_shell_output(int session, std::vector<std::string> input); // to do
        void dowrite(std::string content); 
        std::vector<std::string> get_shell_input(std::string testFile); 

        std::string QUERY_STRING;
        std::vector<client> clients;
        boost::asio::ip::tcp::socket* web_sockt;
};

class panel
{
    public:
        panel();
        void Run(boost::asio::ip::tcp::socket &socket);

    private:
        void dowrite(boost::asio::ip::tcp::socket &socket, std::string content);
};

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

        tcp::socket socket_;
        enum { max_length = 1024 };
        char data_[max_length];
        static boost::asio::io_context* _io_context;

        std::string status_str;
        std::string REQUEST_METHOD;
        std::string REQUEST_URL;
        std::string QUERY_STRING;
        std::string SERVER_PROTOCOL;
        std::string HTTP_HOST;
        std::string SERVER_ADDR;
        std::string SERVER_PORT;
        std::string REMOTE_ADDR;
        std::string REMOTE_PORT;
        std::string EXEC_FILE;

        console m_console;
        panel m_panel;


};

class server
{
    public:
        server(boost::asio::io_context& io_context, short port);

    private:
        void do_accept();

        tcp::acceptor acceptor_;
};