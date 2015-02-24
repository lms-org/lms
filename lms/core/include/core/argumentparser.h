/**
  *Soll später ein handler werden, welcher die geparsten argumente speichert
  */
#ifndef argumenthandler_H
#define argumenthandler_H

#include <string>

class ArgumentHandler{
public:
    /**
     * @brief Parse the given command line arguments and store the values in the members.
     * @param argc Number of arguments
     * @param argv List of argument strings
     */
    void parseArguments(int argc, char* const*argv);

    /**
     * @brief Return the "load configuration" setting. This can
     * be set by "-c arg".
     */
    std::string loadConfiguration() const;

private:
    std::string m_load_configuration;
};


#endif /* argumenthandler_H */
