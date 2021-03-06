#include <sp2/multiplayer/client.h>
#include <sp2/multiplayer/registry.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/engine.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <private/multiplayer/packetIDs.h>

namespace sp {
namespace multiplayer {

Client::Client(string hostname, int port_nr)
{
    socket.setBlocking(false);
    socket.connect(io::network::Address(hostname), port_nr);
    
    state = State::Connecting;
}

Client::~Client()
{
}
    
void Client::onUpdate(float delta)
{
    io::DataBuffer packet;
    
    while(socket.receive(packet))
    {
        uint8_t command_id;
        packet.read(command_id);
        switch(command_id)
        {
        case PacketIDs::request_authentication:{
            send(io::DataBuffer(PacketIDs::request_authentication, PacketIDs::magic_sp2_value));
            }break;
        case PacketIDs::set_client_id:{
            packet.read(client_id);
            }break;
            
        case PacketIDs::change_game_speed:{
            float new_gamespeed;
            packet.read(new_gamespeed);
            sp::Engine::getInstance()->setGameSpeed(new_gamespeed);
            }break;
    
        case PacketIDs::create_object:{
            uint64_t id;
            string class_name;
            uint64_t parent;
            packet.read(id, class_name, parent);
            auto it = multiplayer::ClassEntry::name_to_create_mapping.find(class_name);
            if (it == multiplayer::ClassEntry::name_to_create_mapping.end())
            {
                LOG(Error, "Got class", class_name, "but no way to create it.");
            }
            else
            {
                P<Node> parent_node = getNode(parent);
                if (!parent_node)
                {
                    LOG(Error, "Cannot find parent to create multiplayer object for", class_name, parent);
                }
                else
                {
                    P<Node> new_node = it->second(parent_node);
                    new_node->multiplayer.id = id;
                    addNode(new_node);
                }
            }
            }break;
        case PacketIDs::update_object:{
            uint64_t id;
            packet.read(id);
            P<Node> node = getNode(id);
            if (node)
            {
                uint16_t idx;
                while(packet.available() >= sizeof(idx))
                {
                    packet.read(idx);
                    if (idx < node->multiplayer.replication_links.size())
                        node->multiplayer.replication_links[idx]->receive(*this, packet);
                }
            }
            }break;
        case PacketIDs::delete_object:{
            uint64_t id;
            packet.read(id);
            P<Node> node = getNode(id);
            node.destroy();
            }break;

        case PacketIDs::setup_scene:{
            uint64_t id;
            string scene_name;
            packet.read(id, scene_name);
            sp::P<Scene> scene = Scene::get(scene_name);
            if (scene)
            {
                scene->getRoot()->multiplayer.id = id;
                addNode(scene->getRoot());
            }
            else
            {
                LOG(Error, "Server send data for scene", scene_name, "but could not find the scene, this most likely will result in missing parents as well.");
            }
            }break;

        case PacketIDs::alive:
            break;
        default:
            LOG(Warning, "Received unknown packet:", command_id);
        }
    }
    if (!socket.isConnected())
        state = State::Disconnected;

    cleanDeletedNodes();
}

void Client::send(const io::DataBuffer& packet)
{
    socket.send(packet);
}


};//namespace multiplayer
};//namespace sp
