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
};