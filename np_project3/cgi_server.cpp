#include "cgi_server.h"

boost::asio::io_context *session::_io_context;
boost::asio::io_context g_io_context;

session::session(tcp::socket socket):socket_(std::move(socket))
{
    status_str = "HTTP/1.1 200 OK\n";
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
    // std::cout<<"ddddddddddddddddddddddddddddddddddddd"<<std::endl;
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length), [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::string tmp_str(data_);
                // std::cout<<tmp_str<<std::endl;
                std::istringstream tmp_iss(tmp_str);
                std::vector<std::string> tmp_v;
                boost::split(tmp_v,tmp_str,boost::is_any_of(" \r\n"));

                REQUEST_METHOD = tmp_v[0];
                REQUEST_URL = tmp_v[1];
                SERVER_PROTOCOL = tmp_v[2];
                HTTP_HOST = tmp_v[5];

                // std::cout<<"HTTP_HOST is "<<HTTP_HOST<<std::endl;
                SERVER_ADDR = socket_.local_endpoint().address().to_string();
                REMOTE_ADDR = socket_.remote_endpoint().address().to_string();

                SERVER_PORT = std::to_string(socket_.local_endpoint().port());
                REMOTE_PORT = std::to_string(socket_.remote_endpoint().port());

                tmp_iss = std::istringstream(REQUEST_URL);
                std::getline(tmp_iss, REQUEST_URL, '?');
                std::getline(tmp_iss, QUERY_STRING);    //

                // EXEC_FILE = boost::filesystem::current_path().string() + REQUEST_URL;

                Setenv();
                Printenv();
                
                // std::cout<<"oooooooooooooooooooooooooo"<<std::endl;
                // do_write(length);
                do_cgi();
            }
        });
}

void session::do_write(std::size_t length)
{
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                do_cgi();
            }
        });
}

void session::do_cgi()
{

    if ( REQUEST_URL == "/console.cgi" )
    {
        m_console.set_wb_sock(&socket_);
        m_console.set_query(QUERY_STRING);
        m_console.init_clients();
        m_console.connect_server();
        m_console.Run();

    }
    else if ( REQUEST_URL == "/panel.cgi" )
    {
        m_panel.Run(socket_);
    }
}

void session::Setenv()
{
    setenv("REQUEST_METHOD", REQUEST_METHOD.c_str(), 1);  // conceal
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
    REQUEST_METHOD = getenv("REQUEST_METHOD");      // conceal
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
    // std::cout<<"acacacacacacacacaccacaca"<<std::endl;
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                // std::cout<<"ssssssssssssssssssssssssss"<<std::endl;
                std::make_shared<session>(std::move(socket))->start();
            }

            do_accept();
        });
}

console::console()
{}

void console::set_wb_sock(boost::asio::ip::tcp::socket* socket)
{
    web_sockt = socket;
}

void console::set_query(std::string query_str)
{
    QUERY_STRING = query_str;
}

void console::init_clients()
{
    QUERY_STRING = getenv("QUERY_STRING");
    std::string buffer(QUERY_STRING);
    std::string addr, port, file;
    std::vector<std::string> vtr;
    int index = 0;

    if ( buffer.length() == 0 ) return;

    boost::split(vtr,buffer,boost::is_any_of("&"));

    for ( int i=0; i<5; i++ )    // h0=nplinux1.cs.nctu.edu.tw&p0=1234&f0=t1.txt&h1=nplinux2.cs.nctu.edu.tw&p1=5678&f1=t2.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=

    {
        if ( index < (int)vtr.size() && vtr[index].length() != 0 )
        {
            addr = vtr[index].substr(3);
        }
        index++;
        if ( index < (int)vtr.size() && vtr[index].length() != 0 )
        {
            port = vtr[index].substr(3);
        }
        index++;
        if ( index < (int)vtr.size() && vtr[index].length() != 0 )
        {
            file = vtr[index].substr(3);
        }
        index++;
        
        if ( addr.length() != 0 && port.length() != 0 && file.length() != 0 )
        {
            clients.push_back(client(g_io_context, addr, stoi(port), file));
        }
    }
    
}

void console::connect_server()
{
    for ( int i=0; i< (int)clients.size(); i++ )
    {
        boost::asio::ip::tcp::resolver resolver(g_io_context);
        boost::asio::ip::tcp::resolver::query query(clients[i].server_addr, std::to_string(clients[i].server_port));
        boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
        boost::asio::ip::tcp::endpoint endpoint = *it;
        clients[i].socket.connect(endpoint);
    }
}

void console::Run()
{
    init_html();

    for ( int i=0; i<(int)clients.size(); i++ )
    {
        std::vector<std::string> input = get_shell_input(clients[i].testfile);

        get_shell_output(i, input);
    }

    g_io_context.run();
}

void console::init_html()
{
    // std::cout<<"Content-type: text/html\r\n\r\n"<<std::flush;
    dowrite("HTTP/1.1 200 OK\r\n");
    dowrite("Content-type: text/html\r\n\r\n");
    std::string initial_html = "";

    initial_html += 
    "<!DOCTYPE html>"
    "<html lang=\"en\">"
    "    <head>"
    "    <meta charset=\"UTF-8\" />"
    "    <title>NP Project 3 Sample Console</title>"
    "    <link"
    "        rel=\"stylesheet\""
    "        href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\""
    "        integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\""
    "        crossorigin=\"anonymous\""
    "    />"
    "    <link"
    "        href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\""
    "        rel=\"stylesheet\""
    "    />"
    "    <link"
    "        rel=\"icon\""
    "        type=\"image/png\""
    "        href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\""
    "    />"
    "    <style>"
    "        * {"
    "            font-family: 'Source Code Pro', monospace;"
    "            font-size: 1rem !important;"
    "        }"
    "        body {"
    "            background-color: #212529;"
    "        }"
    "        pre {"
    "            color: #cccccc;"
    "        }"
    "        b {"
    "            color: #01b468;"
    "        }"
    "    </style>"
    "    </head>"
    "    <body>"
    "    <table class=\"table table-dark table-bordered\">"
    "        <thead>"
    "            <tr>";
    
    for ( int i=0; i<(int)clients.size(); i++ )  // <th scope="col">nplinux1.cs.nctu.edu.tw:1234</th>
    {
        initial_html += "<th scope=\"col\">" + clients[i].server_addr + ":" + std::to_string(clients[i].server_port) + "</th>";
    }
    
    initial_html += 
            "</tr>"
            "</thead>"
            "<tbody>"
            "    <tr>";
    
    for ( int i=0; i<(int)clients.size(); i++ )  // <th scope="col">nplinux1.cs.nctu.edu.tw:1234</th>
    {
        initial_html += "<td><pre id=\"s" + std::to_string(i) + "\" class=\"mb-0\"></pre></td>";
    }

    initial_html += 
                        "</tr>"
                    "</tbody>"
                "</table>"
            "</body>"
        "</html>";
    
    // std::cout<<initial_html<<std::flush;
    dowrite(initial_html);
}

void console::shell_input(int session, std::vector<std::string> input)
{
    if ( input.size() == 0) return;

    boost::asio::async_write(clients[session].socket, boost::asio::buffer(input[0], input[0].length()),
    [this, input, session](boost::system::error_code ec, std::size_t)
    {
        std::string prev[7] = { "&", "\"", "\'", "<", ">", "\r\n", "\n" };
        std::string aftr[7] = {"&amp;", "&quot", "&apos;", "&lt;", "&gt;", "\n", "<br>" };
        std::string content = input[0];

        for( int i=0; i<7; i++ )
        {
            boost::replace_all(content, prev[i], aftr[i]);
        }

        // shell_output(session, "% ");
        std::string tmp_str =  "<script>document.getElementById(\'s" + std::to_string(session) + "\').innerHTML += \'<b>" + content + "</b>\';</script>";

        dowrite(tmp_str);
        std::vector<std::string> tmp_v (input.begin()+1,input.end());

        get_shell_output(session, tmp_v);
    });
}

void console::shell_output(int session, std::string content) // print(f"<script>document.getElementById('{session}').innerHTML += '{content}';</script>")
{
    std::string prev[7] = { "&", "\"", "\'", "<", ">", "\r\n", "\n" };
    std::string aftr[7] = {"&amp;", "&quot", "&apos;", "&lt;", "&gt;", "\n", "<br>" };

    for( int i=0; i<7; i++ )
    {
        boost::replace_all(content, prev[i], aftr[i]);
    }
    
    std::string tmp_str = "<script>document.getElementById(\'s" + std::to_string(session) + "\').innerHTML += \'" + content + "\';</script>";
    dowrite(tmp_str);
}

void console::get_shell_output(int session, std::vector<std::string> input)  // stock
{
    clients[session].socket.async_read_some(boost::asio::buffer(clients[session].data, 10240),
    [this, input, session](boost::system::error_code ec, std::size_t length)
    {
        if (ec) return;
        std::string tmp(clients[session].data, length);

        memset(clients[session].data, 0, 10240);   // ??

        shell_output(session, tmp);

        if ( tmp.find("%") == std::string::npos)
        {
            get_shell_output(session, input);
        }
        else
        {
            shell_input(session, input);
        }
    }
    );
}

void console::dowrite(std::string content)
{
    if ( web_sockt == NULL )
    {
        std::cout << "web_socket error" << std::endl;
		return;
    }
    // std::cout << "wwwwwwwwwwwwwwwwwwwwwwwwwwwwwww" << std::endl;
    boost::asio::async_write(*web_sockt, boost::asio::buffer(content.c_str(), content.length()),
    [this](boost::system::error_code ec, std::size_t )
    {
        if (ec) return;
    });
}

std::vector<std::string> console::get_shell_input(std::string testFile)
{
    std::ifstream in ("test_case/" + testFile );
    char buffer[10240];
    std::vector<std::string> v_str;

    memset(buffer, 0, 10240);
    
    if ( in.is_open() )
    {
        while(in.getline(buffer, 10240) )
        {
            v_str.push_back(std::string(buffer) + "\n" );
            memset(buffer, 0, 10240);
        }
    }

    return v_str;
}

panel::panel()
{}

void panel::Run(boost::asio::ip::tcp::socket &socket)
{
    // std::cout<<"============== panel writing 1 =============="<<std::endl;
    std::string FORM_METHOD = "GET";
    std::string FORM_ACTION = "console.cgi";
    std::string TEST_CASE_DIR = "test_case";
    std::vector<std::string> testFile;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(TEST_CASE_DIR.c_str())) != NULL)
    {
        std::string temp;
        while((ent = readdir(dir)) != NULL)
        {
            temp = std::string(ent->d_name);
            if (temp != "." && temp != "..")
            {
                testFile.push_back(temp);
            }
        }
        closedir(dir);
    }
    else
    {
        perror("");
    }

    std::string TEST_CASE_MENU = "";
    sort(testFile.begin(), testFile.end());
    
    // std::cout<<"============== panel writing 2 =============="<<std::endl;

    for (int i = 0; i < (int)testFile.size(); i++ )
    {
        TEST_CASE_MENU += "<option value=\"" + testFile[i] + "\">" + testFile[i] + "</option>";
    }

    std::string domain = ".cs.nctu.edu.tw";
    std::vector<std::string> hosts;
    std::string host_menu = "";
    for (int i = 0; i < 12; i++ ) 
    {
        hosts.push_back("nplinux" + std::to_string(i + 1));
    }
    for (int i = 0; i < (int)hosts.size(); i++ )
    {
        host_menu += "<option value=\"" + hosts[i] + domain + "\">" + hosts[i] + "</option>";
    }

    dowrite(socket, "HTTP/1.1 200 OK\r\n");
    dowrite(socket, "Content-type: text/html\r\n\r\n");

    // std::cout<<"============== panel writing 3 =============="<<std::endl;

    std::string tmp_html = 
"<!DOCTYPE html>"
"    <html lang=\"en\">"
"        <head>"
"            <title>NP Project 3 Panel</title>"
"            <link"
"                rel=\"stylesheet\""
"                href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\""
"                integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\""
"                crossorigin=\"anonymous\""
"            />"
"            <link"
"                href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\""
"                rel=\"stylesheet\""
"            />"
"            <link"
"                rel=\"icon\""
"                type=\"image/png\""
"                href=\"https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png\""
"            />"
"            <style>"
"                * {"
"                    font-family: 'Source Code Pro', monospace;"
"                }"
"            </style>"
"        </head>"
"        <body class=\"bg-secondary pt-5\">";

    dowrite(socket, tmp_html);

    tmp_html = 
"<form action=\"" + FORM_ACTION + "\" method=\"" + FORM_METHOD + "\">"
"    <table class=\"table mx-auto bg-light\" style=\"width: inherit\">"
"        <thead class=\"thead-dark\">"
"            <tr>"
"                <th scope=\"col\">#</th>"
"                <th scope=\"col\">Host</th>"
"                <th scope=\"col\">Port</th>"
"                <th scope=\"col\">Input File</th>"
"            </tr>"
"        </thead>"
"        <tbody>";

    dowrite(socket, tmp_html);

    for ( int i=0; i<5; i++ )
    {
        tmp_html = 
        "<tr>"
        "    <th scope=\"row\" class=\"align-middle\">Session" + std::to_string(i + 1) + "</th>"
        "    <td>"
        "        <div class=\"input-group\">"
        "            <select name=\"h" + std::to_string(i) + "\" class=\"custom-select\">"
        "                 <option></option>" + host_menu  +   
    "               </select>"
    "                    <div class=\"input-group-append\">"
    "                        <span class=\"input-group-text\">.cs.nctu.edu.tw</span>"
    "                    </div>"
    "                </div>"
    "            </td>"
    "            <td>"
    "                <input name=\"p" + std::to_string(i) + "\" type=\"text\" class=\"form-control\" size=\"5\" />"
    "            </td>"
    "            <td>"
    "                <select name=\"f" + std::to_string(i) + "\" class=\"custom-select\">"
    "                    <option></option>" 
                       + TEST_CASE_MENU +
    "                </select>"
    "            </td>"
    "        </tr>";
       
        dowrite(socket, tmp_html);
    }
        tmp_html = 
                    "<tr>"
"                        <td colspan=\"3\"></td>"
"                        <td>"
"                            <button type=\"submit\" class=\"btn btn-info btn-block\">Run</button>"
"                        </td>"
"                    </tr>"
"                </tbody>"
"            </table>"
"        </form>"
"    </body>"
"</html>";

    dowrite(socket, tmp_html);
    // std::cout<<"============== panel writing 4 =============="<<std::endl;
}

void panel::dowrite(boost::asio::ip::tcp::socket &socket, std::string content)
{
    boost::asio::async_write(socket, boost::asio::buffer(content.c_str(), content.length()),
    [this](boost::system::error_code ec, std::size_t)
    {
        if (ec) return;
    });
}

client::client(boost::asio::io_context &io_context, std::string addr, int port, std::string file)
        : server_port(port), server_addr(addr), testfile(file), socket(io_context)
{}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        // std::cout<<"aaaaaaaaaaaaaaaaa"<<std::endl;
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