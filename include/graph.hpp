#ifndef VKR_COURSE_GRAPH
#define VKR_COURSE_GRAPH

#include<stdlib.h>
#include <map>
#include <string>

class INodeKey{
    virtual bool operator<(const INodeKey& other);
};

template <typename data_type> 
class Parameter{
    private:
        data_type data;
    public:
        void set_data(const data_type& _data) {
            data = _data;
        }
    virtual data_type  get_data();
};

class INode {
public:
    const INodeKey key;

    std::map<std::string, Parameter<char[]>> parameters;
};

#endif