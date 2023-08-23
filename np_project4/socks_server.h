#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include<unistd.h>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include<sys/wait.h>
#include <regex>

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
    public:
        session(tcp::socket socket);
        void start();
        static void set_io_context(boost::asio::io_context* _io_context);
        static boost::asio::io_context* _io_context;
    
    private:
        void do_read();
        void do_reply();
        void reject_reply();
        void connect_reply();
        void bind_reply();
        void parse_sock_reuest(int length);
        void print_sock_info(std::string SourceIP, std::string SourcePort, std::string DstIP, std::string DstPort, std::string command, std::string reply);
        void client_read();
        void client_write(int length);
        void web_read();
        void web_request(int length);
        


        bool is_FireWall(char command);

        tcp::socket socket_;
        tcp::socket* web_socket;
        tcp::socket* bind_socket;
        tcp::acceptor* acceptor_;

        enum { max_length = 10240 };
        char data_[max_length];
        char message[8];
        char web_reply[max_length];
        char client_reply[max_length];
        // static boost::asio::io_context* _io_context;

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

        std::string DOMAIN_NAME;
        std::string Dst_IP;
        std::string Dst_Port;

};

class server
{
    public:
        server(boost::asio::io_context& io_context, short port);

    private:
        void do_accept();
        std::vector<pid_t> pid_pool;
        tcp::acceptor acceptor_;
};