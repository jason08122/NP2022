#include "socks_server.h"

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

    memset(data_, 0, max_length);

    socket_.async_read_some(boost::asio::buffer(data_, max_length),
    [this, self](boost::system::error_code ec, std::size_t length)
    {
        if (ec) return;

        parse_sock_reuest(length);

        do_reply();
    });
}

void session::parse_sock_reuest(int length)
{
    // unsigned char USER_ID[1024];
	unsigned char DOMAIN_NAME_TEMP[1024];

    // Dst_Port += std::to_string((int)(data_[2] < 0 ? (data_[2] + 256) * 256 : data_[2] * 256));
    // Dst_Port += std::to_string((int)(data_[3] < 0 ? data_[3] + 256 : data_[3]));
    Dst_Port = std::to_string((int)(data_[2] < 0 ? (data_[2] + 256) * 256 : data_[2] * 256) + (int)(data_[3] < 0 ? data_[3] + 256 : data_[3]));

    Dst_IP = "";

    for ( int i=4; i<8; i++ )
    {
        if ( i != 4 ) Dst_IP += ".";

        int tmp = (data_[i] < 0 ? (int)data_[i] + 256 : data_[i]);
        Dst_IP += std::to_string(tmp);
    }

    bool flag = false;
    int count = 0;
    for ( int i = 8; i<(int)length; i++ )
    {
        if ( !data_[i])
        {
            flag = true;
            count = 0;
        }
        else if (!flag)
        {
            // USER_ID[count] = data_[i];
            // USER_ID[count+1] = '\0';
            count++;
        }
        else
        {
            DOMAIN_NAME_TEMP[count] = data_[i];
            DOMAIN_NAME_TEMP[count+1] = '\0';
            count++;
        }
    }

    DOMAIN_NAME = std::string((char*)DOMAIN_NAME_TEMP);
}

void session::do_reply()
{
    std::string command;
    std::string reply;
    
    if ( data_[0] != 0x04 || !is_FireWall(data_[1]) )
    {
        reply = "Reject";
        reject_reply();
    }
    else if ( data_[1] == 0x01 )
    {
        command = "CONNECT";
        reply = "Accept";

        connect_reply();
    }
    else if ( data_[1] == 0x02 )
    {
        command = "BIND";
        reply = "Accept";

        bind_reply();
    }

    print_sock_info(socket_.remote_endpoint().address().to_string(), std::to_string(socket_.remote_endpoint().port()), Dst_IP, Dst_Port, command, reply);
}

void session::print_sock_info(std::string SourceIP, std::string SourcePort, std::string DstIP, std::string DstPort, std::string command, std::string reply)
{
    std::string ip;

    if ( Dst_IP[0] == '0' )
    {
        tcp::resolver resolver(*session::_io_context);
        tcp::resolver::query query(DOMAIN_NAME, DstPort);
        tcp::resolver::iterator it = resolver.resolve(query);
        tcp::endpoint endpoint = *it;
        ip = endpoint.address().to_string();
    }

    printf("==============================================\n");
	printf("<S_IP>: %s \n", SourceIP.c_str());
	printf("<S_PORT>: %s \n", SourcePort.c_str());
	printf("<D_IP>: %s\n", DstIP.c_str());
	if (ip != "") printf("<D_IP>: %s(parse)\n", ip.c_str());
	printf("<D_PORT>: %s\n", DstPort.c_str());
	printf("<Command>: %s\n", command.c_str());
	printf("<Reply>: %s\n", reply.c_str());
	printf("==============================================\n\n");
}

void session::reject_reply()
{
    message[0] = 0;
    message[1] = 0x58;

    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(message, 8),
    [this, self](boost::system::error_code ec, std::size_t)
    {
        if (ec) return;
    });
}

void session::connect_reply()
{
    // printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
    message[0] = 0;
    message[1] = 0x5A;

    for ( int i=2;i<8;i++)
    {
        message[i] = data_[i];
    }

    auto self(shared_from_this());
    // printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");
    boost::asio::async_write(socket_, boost::asio::buffer(message, 8),
    [this, self](boost::system::error_code ec, std::size_t)
    {
        if (ec) return;

        web_socket = new tcp::socket(*session::_io_context);

        tcp::endpoint endpoint;

        if(Dst_IP[0] == '0')
        {
            tcp::resolver resolver(*session::_io_context);
            tcp::resolver::query query(DOMAIN_NAME, Dst_Port);
            tcp::resolver::iterator it = resolver.resolve(query);
            endpoint = *it;
        }
        else
        {
            endpoint = tcp::endpoint(boost::asio::ip::address::from_string(Dst_IP), atoi(Dst_Port.c_str()));
        }

        (*web_socket).connect(endpoint);
        // printf("cccccccccccccccccccccccccccccccccccccc\n");
        web_read();
        client_read();
    });
}

void session::bind_reply()
{
    int tmp;

    message[0] = 0;
    message[1] = 0x5A;

    tcp::endpoint endpoint(boost::asio::ip::address::from_string("0.0.0.0"), 0);

    acceptor_ = new tcp::acceptor(*session::_io_context);

    (*acceptor_).open(tcp::v4());
    (*acceptor_).set_option(tcp::acceptor::reuse_address(true));
    (*acceptor_).bind(endpoint);
    (*acceptor_).listen(boost::asio::socket_base::max_connections);

    tmp = (*acceptor_).local_endpoint().port()/256;
    message[2] = tmp > 128 ? tmp-256 : tmp;
    tmp = (*acceptor_).local_endpoint().port()%256;
    message[3] = tmp > 128 ? tmp-256 : tmp;

    for ( int i=4; i<8; i++ )
    {
        message[i] = 0;
    }

    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer(message, 8),
    [this, self](boost::system::error_code ec, std::size_t)
    {
        if (ec) return;

        web_socket = new tcp::socket(*session::_io_context);
        (*acceptor_).accept(*web_socket);
        boost::asio::write(socket_, boost::asio::buffer(message, 8));

        web_read();
        client_read();

    });
}

void session::web_read()
{
    auto self(shared_from_this());

    // printf("web_read is\n%s\n", web_reply);
    memset(web_reply, '\0', max_length); 
    (*web_socket).async_read_some(boost::asio::buffer(web_reply),
    [this, self](boost::system::error_code ec, std::size_t length)
    {
        if (ec)
        {
            if ( ec == boost::asio::error::eof)
            {
                (*web_socket).close();
                (socket_).close();
            }

            return;
        }
        client_write(length);
    });
}

void session::client_write(int length)
{
    auto self(shared_from_this());
    // printf("client_write is\n%s\n", web_reply);
    boost::asio::async_write(socket_, boost::asio::buffer(web_reply, length),
    [this, self](boost::system::error_code ec, std::size_t len)
    {
        if (ec) return;

        web_read();
    });
}

void session::client_read()
{
    auto self(shared_from_this());
    // printf("client_read is\n%s\n", client_reply);
    memset(client_reply, 0, max_length);
    socket_.async_read_some(boost::asio::buffer(client_reply),
    [this, self](boost::system::error_code ec, std::size_t length)
    {
        if (ec)
        {
            if ( ec == boost::asio::error::eof)
            {
                (*web_socket).close();
                (socket_).close();
            }

            return;
        }

        web_request(length);
    });
}

void session::web_request(int length)
{
    auto self(shared_from_this());
    // printf("web_request is\n%s\n", client_reply);
    boost::asio::async_write((*web_socket), boost::asio::buffer(client_reply, length),
    [this, self](boost::system::error_code ec, std::size_t len)
    {
        if (ec) return;

        client_read();
    });
}

bool session::is_FireWall(char command)
{
    std::ifstream fp;

    fp.open("socks.conf");

    if ( !fp)
    {
        std::cout<<"Can't find fire wall file"<<std::endl;
        return true;
    }

    std::string rule;

    while(std::getline(fp, rule))
    {
        std::stringstream tmp_string(rule);
        std::string event, type, ip;

        tmp_string >> event >> type >> ip;

        std::string tmp_ip;

        if ( Dst_IP[0] == '0')
        {
            tcp::resolver resolver(*session::_io_context);
            tcp::resolver::query query(DOMAIN_NAME, Dst_Port);
            tcp::resolver::iterator it = resolver.resolve(query);
            tcp::endpoint endpoint = *it;
            tmp_ip = endpoint.address().to_string();
        }
        else
        {
            tmp_ip = Dst_IP;
        }

        if ( event == "permit" && ((type == "c" && command == 0x01) || (type == "b" && command == 0x02)))
        {
            std::string rex_string;
            for ( int i=0; i<(int)ip.length(); i++)
            {
                if (ip[i] == '*' )
                {
                    rex_string += "[0-9]+";
                }
                else
                {
                    rex_string += ip[i];
                }
            }

            std::regex reg(rex_string);
            
            if ( regex_match(tmp_ip, reg))
            {
                fp.close();
                return true;
            }
        }
    }

    fp.close();
    return false;
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
        if (ec) return;
        (*session::_io_context).notify_fork(boost::asio::io_context::fork_prepare);
        pid_t pid = fork();

        if(pid != 0)
        {
            (*session::_io_context).notify_fork(boost::asio::io_context::fork_parent);
            socket.close();

            pid_pool.push_back(pid);

            int waitPID, status;

            for (int i=0; i<(int)pid_pool.size(); i++ )
            {
                waitPID = waitpid(pid_pool[i], &status, WNOHANG);
                if ( waitPID == pid_pool[i] ) pid_pool.erase(pid_pool.begin()+i, pid_pool.begin()+i+1);
            }

            do_accept();
        }
        else
        {
            (*session::_io_context).notify_fork(boost::asio::io_context::fork_child);
            std::make_shared<session>(std::move(socket))->start();
        }
        
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