#ifndef SP2_IO_NETWORK_ADDRESS_H
#define SP2_IO_NETWORK_ADDRESS_H

#include <sp2/string.h>
#include <sp2/nonCopyable.h>
#include <list>


namespace sp {
namespace io {
namespace network {


class Address
{
public:
    Address(string hostname);
    
    static Address getLocalAddress();
private:
    class AddrInfo
    {
    public:
        AddrInfo(int family, const string& human_readable, const void* addr, size_t addrlen);
        ~AddrInfo();
    
        int family; //One of the AF_* macros, currently only AF_INET or AF_INET6
        string human_readable;
        std::string addr;    //We abuse the std::string as a data buffer, it allows for easy memory management.
    };

    Address(std::list<AddrInfo>&& addr_info);
    
    std::list<AddrInfo> addr_info;
    
    friend class TcpSocket;
    friend class UdpSocket;
};

};//namespace network
};//namespace io
};//namespace sp

#endif//SP2_IO_NETWORK_TCP_SOCKET_H
