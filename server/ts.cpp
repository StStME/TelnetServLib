#include "telnetservlib.h"
#include "iostream"
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
void myConnected(SP_TelnetSession session)
{
    std::cout << "myConnected got called\n";
    session->sendLine("Welcome to the Telnet Server.");
}

void myNewLine(SP_TelnetSession session, std::string line)
{
    std::cout << "myNewLine got called with line: " << line << "\n";
    session->sendLine("Copy that.");
}

int main()
{

    // Do unit tests
    TelnetSession::UNIT_TEST();

    sock_s sock = {
        .shutdown = shutdown,
        .socket = socket,
        .getaddrinfo = getaddrinfo,
        .freeaddrinfo = freeaddrinfo,
        .accept = accept,
        .listen = listen,
        .select = select,
        .bind = bind,
        .close = close,
        .recv = recv,
        .send = send
    };
    // Create a terminal server which
    auto ts = std::make_shared < TelnetServer >();
    
    ts->initialise(27015);
    ts->connectedCallback(myConnected);
    ts->newLineCallback(myNewLine);

    // Our loop
    do 
    {
        ts->update();
        sleep(16);
    } 
    while (true);

    ts->shutdown();
    return 0;
}