#ifndef LMS_INHERITANCE_H
#define LMS_INHERITANCE_H
#include <memory>
namespace lms{
class Inheritance{
public:
    virtual bool isSubType(size_t hashcode);
    virtual float getHashCode();
};
}//lms


#endif //LMS_INHERITANCE_H
