#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include <core/datamanager.h>
#include <core/signalhandler.h>
#include <core/argumentparser.h>
#include <core/executionmanager.h>

class Framework : public SignalHandler::Listener {
public:
    Framework(const ArgumentHandler& arguments);
    ~Framework();
protected:
    /**
     * @brief initManagers override this method to set custom Managers!
     */
    virtual void initManagers();
private:
    bool running;

    void signal(int s);

    DataManager *dataManager;
    ExecutionManager *executionManager;
    ArgumentHandler argumentHandler;
};

#endif /* FRAMEWORK_H */
