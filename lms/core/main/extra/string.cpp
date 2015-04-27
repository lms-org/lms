#include <lms/extra/string.h>
#include <string>
#include <vector>
#include <iostream>

namespace lms {
namespace extra {

std::string trim(const std::string& str, const std::string& delims) {
    std::string result(str);
    result.erase(0, result.find_first_not_of(delims));
    result.erase(result.find_last_not_of(delims) + 1);
    return result;
}
std::vector<std::string> split(char *string, int strLength, const char splitter){
    std::vector<std::string> result;
    std::cout <<"CALLED"<<std::endl;
    char *resultBuff = string;
    int oldFound = 0;
    for(int i = 0; i < strLength; i++){
        std::cout<<"char: "<< resultBuff[i] <<std::endl;
        if(resultBuff[i] == splitter){
            //found new part
            std::string name(&resultBuff[oldFound],i-oldFound);
            oldFound = i+1;
            result.push_back(name);
            std::cout <<"part found: " << name << std::endl;
        }
    }
    //add last element if the string doesn't end with the splitter
    if(oldFound < strLength){
        std::string name(&resultBuff[oldFound],strLength-oldFound);
        result.push_back(name);

    }
    return result;
}

} // namespace extra
} // namespace lms
