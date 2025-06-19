#include "utils/print_tuple.h"
#include <string>
#include <tuple>

int main() {
    auto data = std::make_tuple(17, 3.14159, 'B', std::string("Universe"), std::make_pair(1, 2));
    
    std::cout << "Tuple contents: ";
    utils::tuple::print_tuple(std::cout, data); 
    std::cout << "\n" << std::endl;
    
    return 0;
}
