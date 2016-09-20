#ifndef LMS_SERIALIZABLE_H
#define LMS_SERIALIZABLE_H

#include <iostream>
#include <lms/deprecated.h>
#include <lms/exception.h>

namespace lms {

/**
 * @brief Abstract super class for all data channels
 * that should be serializable.
 *
 * Provides methods to serialize and deserialize the
 * data channel.
 */
class Serializable {
public:
    enum class Type{
        BINARY,XML,JSON,DEFAULT
    };
    static std::string getStringFromType(const Type &t){
        switch (t) {
        case Type::BINARY:
            return "BINARY";
            break;
        case Type::XML:
            return "XML";
            break;
        case Type::JSON:
            return "JSON";
            break;
        case Type::DEFAULT:
            return "DEFAULT";
            break;
        default:
            break;
        }
        throw lms::LmsException("INVALID Serializable Type: "+(int)t);
    }
    static Type getTypeFromString(const std::string s){
        if(s == "BINARY"){
            return Type::BINARY;
        }else if(s == "JSON"){
            return Type::JSON;
        }else if(s == "XML"){
            return Type::XML;
        }else if(s == "DEFAULT"){
            return Type::DEFAULT;
        }
        throw lms::LmsException("INVALID Serializable Type: "+s);
    }

    virtual ~Serializable() {}

    /**
     * @brief Serialize the object into the given output stream.
     * @param os output stream to write in
     */
    virtual void lmsSerialize(std::ostream &os)const =0;

    /**
     * @brief Serialize the object into the given output stream.
     * @param os output stream to write in
     * @param type serialization type
     */
    virtual void lmsSerialize(std::ostream &os,Type type){// const = 0;
        lmsSerialize(os); //TODO
    }

    /**
     * @brief Deserialize the object from the given input stream.
     * @param is input stream to read from
     */
    virtual void lmsDeserialize(std::istream &is) = 0;

    /**
     * @brief Deserialize the object from the given input stream.
     * @param is input stream to read from
     * @param type serialization type
     */
    virtual void lmsDeserialize(std::istream &is,Type type){
        lmsDeserialize(is); //TODO
    }
};

} // namespace lms

#endif /* LMS_SERIALIZABLE_H */
