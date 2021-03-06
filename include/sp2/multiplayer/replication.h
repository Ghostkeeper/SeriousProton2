#ifndef SP2_MULTIPLAYER_REPLICATION_H
#define SP2_MULTIPLAYER_REPLICATION_H

#include <sp2/multiplayer/nodeRegistry.h>
#include <sp2/io/dataBuffer.h>

namespace sp {
namespace multiplayer {

class ReplicationLinkBase : NonCopyable
{
public:
    virtual ~ReplicationLinkBase() {}
    virtual bool isChanged() = 0;
    virtual void initialSend(NodeRegistry& registry, io::DataBuffer& packet) { send(registry, packet); }
    virtual void send(NodeRegistry& registry, io::DataBuffer& packet) = 0;
    virtual void receive(NodeRegistry& registry, io::DataBuffer& packet) = 0;
};

template<typename T> class ReplicationLink : public ReplicationLinkBase
{
public:
    ReplicationLink(T& value)
    : value(value), previous_value(value)
    {
    }

    virtual bool isChanged() override
    {
        if (value == previous_value)
            return false;
        previous_value = value;
        return true;
    }
    
    virtual void send(io::DataBuffer& packet) override
    {
        packet.write(value);
    }
    
    virtual void receive(io::DataBuffer& packet) override
    {
        packet.read(value);
    }
private:
    T& value;
    T previous_value;
};

template<class T> class ReplicationLink<P<T>> : public ReplicationLinkBase
{
public:
    ReplicationLink(P<T>& object)
    : object(object), previous_id(object ? object.multiplayer.getId() : 0)
    {
    }

    virtual bool isChanged() override
    {
        uint64_t id = object ? object.multiplayer.getId() : 0;
        if (id == previous_id)
            return false;
        previous_id = id;
        return true;
    }
    
    virtual void send(NodeRegistry& registry, io::DataBuffer& packet) override
    {
        uint64_t id = object ? object.multiplayer.getId() : 0;
        packet.write(id);
    }
    
    virtual void receive(NodeRegistry& registry, io::DataBuffer& packet) override
    {
        uint64_t id;
        packet.read(id);
        object = registry.getNode(id);
    }
private:
    P<T>& object;
    uint64_t previous_id;
};

};//namespace multiplayer
};//namespace sp

#endif//SP2_MULTIPLAYER_REPLICATION_H
