#include <sp2/io/network/tcpListener.h>
#include <sp2/io/network/tcpSocket.h>

#ifdef __WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
static constexpr int flags = 0;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
static constexpr int flags = MSG_NOSIGNAL;
#endif


namespace sp {
namespace io {
namespace network {


TcpListener::TcpListener()
{
}

TcpListener::~TcpListener()
{
    close();
}

bool TcpListener::listen(int port)
{
    if (isListening())
        close();
    
    handle = ::socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    int optval = 1;
    ::setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(int));
    optval = 0;
    ::setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&optval, sizeof(int));
    
    struct sockaddr_in6 server_addr;
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(port);

    if (::bind(handle, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        close();
        return false;
    }
    if (::listen(handle, 8) < 0)
	{
		close();
		return false;
	}
    return true;
}

void TcpListener::close()
{
    if (isListening())
    {
#ifdef __WIN32
        closesocket(handle);
#else
        ::close(handle);
#endif
        handle = -1;
    }
}

bool TcpListener::isListening()
{
    return handle != -1;
}

bool TcpListener::accept(TcpSocket& socket)
{
    if (!isListening())
        return false;
    
    int result = ::accept(handle, nullptr, nullptr);
    if (result < 0)
    {
        if (isLastErrorNonBlocking())
            close();
        return false;
    }
    if (socket.isConnected())
        socket.close();
    socket.handle = result;
    socket.setBlocking(socket.blocking);
    return false;
}

void TcpListener::setBlocking(bool blocking)
{
    this->blocking = blocking;
    if (!isListening())
        return;

#ifdef __WIN32
   unsigned long mode = blocking ? 0 : 1;
   ::ioctlsocket(handle, FIONBIO, &mode);
#else
    int flags = ::fcntl(handle, F_GETFL, 0);
    if (blocking)
        flags &=~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    ::fcntl(handle, F_SETFL, flags);
#endif
}

bool TcpListener::isLastErrorNonBlocking()
{
#ifdef __WIN32
    int error = WSAGetLastError();
    if (error == WSAEWOULDBLOCK || error == WSAEALREADY)
        return true;
#else
    if (errno == EAGAIN || errno == EINPROGRESS || errno == EWOULDBLOCK)
        return true;
#endif
    return false;
}

};//namespace network
};//namespace io
};//namespace sp