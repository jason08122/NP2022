#include "http_server.h"

boost::asio::io_context *session::_io_context;
boost::asio::io_context g_io_context;

session::session(tcp::socket socket):socket_(std::move(socket))
{
    // status_str = "HTTP/1.1 200 OK\n";
}

void session::start()
{
    do_read();
}
    
void session::set_io_context(boost::asio::io_context* io_context)
{
    _io_context = io_context;
}

void session::do_read()
{
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length), [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::string tmp_str(data_);
                std::istringstream tmp_iss(tmp_str);
                std::vector<std::string> tmp_v;
                boost::split(tmp_v,tmp_str,boost::is_any_of(" \r\n"));

                REQUEST_METHOD = tmp_v[0];
                REQUEST_URL = tmp_v[1];
                SERVER_PROTOCOL = tmp_v[2];
                HTTP_HOST = tmp_v[5];

                SERVER_ADDR = socket_.local_endpoint().address().to_string();
                REMOTE_ADDR = socket_.remote_endpoint().address().to_string();

                SERVER_PORT = std::to_string(socket_.local_endpoint().port());
                REMOTE_PORT = std::to_string(socket_.remote_endpoint().port());

                tmp_iss = std::istringstream(REQUEST_URL);
                std::getline(tmp_iss, REQUEST_URL, '?');
                std::getline(tmp_iss, QUERY_STRING);    //

                EXEC_FILE = boost::filesystem::current_path().string() + REQUEST_URL;

                Setenv();
                Printenv();

                exe_cgi();
            }
        });
}

void session::exe_cgi()
{
    pid_t pid = fork();

    if ( pid == 0 )
    {
        int sock = socket_.native_handle();
        
        dup2(sock, STDIN_FILENO);
        dup2(sock, STDOUT_FILENO);
        dup2(sock, STDERR_FILENO);
        close(sock);
        
        std::cout<<"HTTP/1.1 200 OK\r\n";
        std::cout<<"Content-Type: text/html\r\n";
        
        if ( execlp(EXEC_FILE.c_str(), EXEC_FILE.c_str(), NULL) < 0 )
        {
            std::cout<<"Content-type:text/html\r\n\r\n<h1>fail</h1>"<<std::endl;
        }

    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        socket_.close();
    }
}

void session::Setenv()
{
    setenv("REQUEST_METHOD", REQUEST_METHOD.c_str(), 1);
    setenv("REQUEST_URL", REQUEST_URL.c_str(), 1);
    setenv("QUERY_STRING", QUERY_STRING.c_str(), 1);
    setenv("SERVER_PROTOCOL", SERVER_PROTOCOL.c_str(), 1);
    setenv("HTTP_HOST", HTTP_HOST.c_str(), 1);
    setenv("SERVER_ADDR", SERVER_ADDR.c_str(), 1);
    setenv("SERVER_PORT", SERVER_PORT.c_str(), 1);
    setenv("REMOTE_ADDR", REMOTE_ADDR.c_str(), 1);
    setenv("REMOTE_PORT", REMOTE_PORT.c_str(), 1);
    setenv("EXEC_FILE", EXEC_FILE.c_str(), 1);
}

void session::Printenv()
{
    REQUEST_METHOD = getenv("REQUEST_METHOD");
    REQUEST_URL = getenv("REQUEST_URL");
    QUERY_STRING = getenv("QUERY_STRING");
    SERVER_PROTOCOL = getenv("SERVER_PROTOCOL");
    HTTP_HOST = getenv("HTTP_HOST");
    SERVER_ADDR = getenv("SERVER_ADDR");
    SERVER_PORT = getenv("SERVER_PORT");
    REMOTE_ADDR = getenv("REMOTE_ADDR");
    REMOTE_PORT = getenv("REMOTE_PORT");
    EXEC_FILE = getenv("EXEC_FILE");
    
    std::cout<<"=================================================\n"<<std::endl;
    std::cout<<"REQUEST_METHOD: "<<REQUEST_METHOD<<std::endl; 
    std::cout<<"REQUEST_URL: "<<REQUEST_URL<<std::endl; 
    std::cout<<"QUERY_STRING: "<<QUERY_STRING<<std::endl;
    std::cout<<"SERVER_PROTOCOL: "<<SERVER_PROTOCOL<<std::endl;
    std::cout<<"HTTP_HOST: "<<HTTP_HOST<<std::endl;
    std::cout<<"SERVER_ADDR: "<<SERVER_ADDR <<std::endl;
    std::cout<<"SERVER_PORT: "<<SERVER_PORT<<std::endl;
    std::cout<<"REMOTE_ADDR: "<<REMOTE_ADDR <<std::endl;
    std::cout<<"REMOTE_PORT: "<<REMOTE_PORT <<std::endl;
    std::cout<<"EXEC_FILE: "<<EXEC_FILE <<std::endl;
    std::cout<<"\n=================================================\n"<<std::endl;
}

server::server(boost::asio::io_context& io_context, short port):acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
{
    session::set_io_context(&io_context);

    do_accept();
}

void server::do_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::make_shared<session>(std::move(socket))->start();
            }

            do_accept();
        });
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        server s(io_context, std::atoi(argv[1]));

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}