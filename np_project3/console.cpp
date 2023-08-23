#include "console.h"

boost::asio::io_context g_io_context;

console::console()
{}

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
    std::cout<<"Content-type: text/html\r\n\r\n"<<std::flush;
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
    
    std::cout<<initial_html<<std::flush;
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
    std::cout<<content<<std::flush;
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

client::client(boost::asio::io_context &io_context, std::string addr, int port, std::string file)
        : server_port(port), server_addr(addr), testfile(file), socket(io_context)
{}

int main(int argc, char* argv[])
{
    console m_console;
    m_console.init_clients();
    m_console.connect_server();
    m_console.Run();
    

    return 0;
}