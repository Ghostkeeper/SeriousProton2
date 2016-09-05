#ifndef SP2_IO_RESOURCEPROVIDER_H
#define SP2_IO_RESOURCEPROVIDER_H

#include <sp2/string.h>
#include <sp2/pointer.h>
#include <sp2/pointerVector.h>
#include <SFML/System/InputStream.hpp>
#include <memory>

namespace sp {
namespace io {

class ResourceStream : public sf::InputStream
{
public:
    virtual ~ResourceStream() {}
    
    string readLine();
};
typedef std::shared_ptr<ResourceStream> ResourceStreamPtr;

class ResourceProvider : public AutoPointerObject
{
public:
    ResourceProvider();
    
    virtual ResourceStreamPtr getStream(const string filename) = 0;
    virtual std::vector<string> findFilenames(const string search_pattern) = 0;

    static ResourceStreamPtr get(const string filename);
    static std::vector<string> find(const string search_pattern);
protected:
    bool searchMatch(const string name, const string search_pattern);

private:
    static PVector<ResourceProvider> providers;
};

};//!namespace io
};//!namespace sp

#endif//RESOURCELOADER_H
