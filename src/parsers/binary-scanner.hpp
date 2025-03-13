#pragma once
#include "binary-mapper.hpp"

namespace darauble {

class BinaryScanner {
private:
    bool stopFlag;
protected:
    BinaryMapper& mapper;
public:
    BinaryScanner(BinaryMapper& _mapper);

    virtual void scan() final;
    virtual void stop() final;
    
    virtual void reset() {};
    virtual void record(const FitDefinitionMessage& d, const FitDataMessage& m);
    virtual void end() {};
};

} // namespace darauble
