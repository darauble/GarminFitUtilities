#include "binary-scanner.hpp"

#include <iostream>

namespace darauble {

BinaryScanner::BinaryScanner(BinaryMapper& _mapper) :
    mapper {_mapper}
{}

void BinaryScanner::scan() {
    reset();
    
    if (!mapper.isParsed()) {
        mapper.parse();

        if (!mapper.isParsed()) {
            throw std::runtime_error("BinaryScanner:: failed to map file");
        }
    }

    std::cout << "BinaryScanner::scan: " << mapper.dataMessages().size() << " data messages" << std::endl;

    stopFlag = false;

    for (auto& m : mapper.dataMessages()) {
        record(mapper.definitions()[m.definitionIndex], m);
        
        if (stopFlag) {
            break;
        }
    }
}

void BinaryScanner::stop() {
    stopFlag = true;
}

void BinaryScanner::record(const FitDefinitionMessage& d, const FitDataMessage& m) {
    std::cout << "BinaryScanner::record: Global #"
        << d.globalMessageNumber
        << ", local #" << +m.localMessageType
        << ", data offset: " << m.offset
        << std::endl;
}

} // namespace darauble
