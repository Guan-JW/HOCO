#include "Factory.hpp"
#include "schemes/include_list.txt"

int main () {
    cout << "Registered classes:" << endl;
    REGISTER(BaseHC, RLE_HC, string, size_t, string, string);

    FACTORY(BaseHC, string, size_t, string, string).printRegisteredClasses();
    cout << "---" << std::endl; 
    
    unique_ptr<BaseHC<string, size_t, string, string>> rle(FACTORY(BaseHC, string, size_t, string, string).create("RLE_HC"));
  
    rle->printName();

    return 0;
}