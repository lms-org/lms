/**
  *Soll später ein handler werden, welcher die geparsten argumente speichert
  */
#ifndef argumenthandler_H
#define argumenthandler_H
#include <string>
class ArgumentHandler{
public:
    ArgumentHandler();
    void parse_arguments(int argc, char* const*argv);
    ~ArgumentHandler();

    std::string load_configuration;

};


#endif
