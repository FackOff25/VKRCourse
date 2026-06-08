#ifndef VKR_CONFIG
#define VKR_CONFIG

#include "fennel_params.hpp"

enum StreamingType {
    SIMPLE,
    FENNEL
};

enum RequestWeightType {
    SCHISM,
    WAWPART
};

enum OptimizerType {
    NONE,
    KL
};

struct Config{
public:
    const int storage_num;
    const StreamingType streaming_type;
    const RequestWeightType request_weigh_type;
    const FennelParameters fennel_params;
    const OptimizerType optimzier_type;
};


#endif // VKR\_CONFIG