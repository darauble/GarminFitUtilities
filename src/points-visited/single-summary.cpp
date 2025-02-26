#include "single-summary.hpp"

namespace darauble {

std::ostream& operator << (std::ostream& os, const SingleSummary& s) {
    os << "===== Parsing Summary =====" << std::endl;
    os << "Total files:    " << s.getTotalFiles() << std::endl;
    os << "Parsed files:   " << s.getParsedFiles() << std::endl;
    os << "Filtered files: " << s.getFilteredFiles() << std::endl;
    os << "Total visits:   " << s.getTotalVisits() << std::endl;
    os << "===========================" << std::endl;
    return os;
}

} // namespace darauble