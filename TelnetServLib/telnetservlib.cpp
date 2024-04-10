#include "telnetservlib.h"
#include "iostream"
#include <errno.h>
#include <assert.h>
#include <array>
#include <iterator>
#include <netdb.h>
#include <sys/socket.h>

/* ------------------ Telnet Server -------------------*/
bool TelnetServer::initialise(u_long listenPort, std::string promptString)
{
    if (m_initialised)
    {
        std::cout << "This Telnet Server instance has already been initialised. Please shut it down before reinitialising it.";
        return false;
    }

    m_listenPort = listenPort;
    m_promptString = promptString;

    std::cout << "Starting Telnet Server on port " << std::to_string(m_listenPort) << "\n";

    int iResult;

    m_listenSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints = {    
        .ai_flags = AI_PASSIVE,
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = 0, /* allow any protocol */
    };

    // Initialize Winsock


    // Resolve the server address and port
    iResult = (*m_sock_s->getaddrinfo)(NULL, std::to_string(m_listenPort).c_str(), &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %i\n", iResult);
        return false;
    }

    // Create a SOCKET for connecting to server
    m_listenSocket = (*m_sock_s->socket)(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %i\n", iResult);
        (*m_sock_s->freeaddrinfo)(result);
        return false;
    }

    // Setup the TCP listening socket
    iResult = (*m_sock_s->bind)(m_listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult < 0) {
        printf("bind failed with error: %i\n", iResult);
        (*m_sock_s->freeaddrinfo)(result);
        (*m_sock_s->shutdown)(m_listenSocket, 2); // and stop reception and transmission
        return false;
    }

    (*m_sock_s->freeaddrinfo)(result);

    iResult = (*m_sock_s->listen)(m_listenSocket, SOMAXCONN);
    if (iResult < 0) {
        printf("listen failed with error: %i\n", iResult);
        (*m_sock_s->shutdown)(m_listenSocket, 2); // and stop reception and transmission        return false;
    }

    m_initialised = true;
    return true;
}

void TelnetServer::acceptConnection()
{
    socket_t ClientSocket = INVALID_SOCKET;
    ClientSocket = (*m_sock_s->accept)(m_listenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %i\n", ClientSocket);
        (*m_sock_s->shutdown)(m_listenSocket, 2); // and stop reception and transmission        return;
    }
    else
    {
        SP_TelnetSession s = std::make_shared < TelnetSession >(ClientSocket, shared_from_this());
        m_sessions.push_back(s);
        s->initialise();        
    }
}

void TelnetServer::update()
{
    // See if connection pending on the listening socket
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(m_listenSocket, &readSet);
    timeval timeout;
    timeout.tv_sec = 0;  // Zero timeout (poll)
    timeout.tv_usec = 0;

    if ((*m_sock_s->select)(m_listenSocket, &readSet, NULL, NULL, &timeout) == 1)
    {
        // There is a connection pending, so accept it.
        acceptConnection();
    }

    // Update all the telnet Sessions that are currently in flight.
    for (SP_TelnetSession ts : m_sessions)
    {
        ts->update();
    }
}

void TelnetServer::shutdown()
{
    // Attempt to cleanly close every telnet session in flight.
    for (SP_TelnetSession ts : m_sessions)
    {
        ts->closeClient();
    }
    m_sessions.clear();

    // No longer need server socket so close it.
    (*m_sock_s->shutdown)(m_listenSocket, 2); // and stop reception and transmission
    m_initialised = false;
}