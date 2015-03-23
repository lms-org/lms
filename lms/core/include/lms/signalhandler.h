#ifndef LMS_SIGNAL_HANDLER_H
#define LMS_SIGNAL_HANDLER_H

#include <vector>
#include <map>

namespace lms {

/**
 * @brief Object-oriented wrapper for signal.h
 *
 * Catches the the given signals from the operating
 * system and broadcasts them to all listeners.
 */
class SignalHandler {
public:
    /**
     * @brief Return the sigleton instance or create one and return it.
     * @return the singleton instance
     */
    static SignalHandler& getInstance();

    /**
     * @brief Listener interface with a single method
     */
    class Listener {
    public:
        /**
         * @brief This method will be called when a signal
         * is broadcasted.
         * @param signal signal code, e.g. SIGINT or SIGSEGV
         * @return should return false if the signal
         */
        virtual void signal(int signal) = 0;
    };

    SignalHandler& addListener(int signal, Listener *listener);
    SignalHandler& removeListener(int signal, Listener *listener);
private:
    static SignalHandler instance;

    /**
     * @brief Private constructor
     */
    SignalHandler() {}

    std::map<int, std::vector<Listener*>> listeners;

    /**
     * @brief Argument for sigaction() in signal.h
     *
     * Must be static!
     */
    static void handlerForAllSignals(int signal);

    /**
     * @brief Private copy constructor
     *
     * We will not allow any copies of our singleton.
     */
    SignalHandler(const SignalHandler&);

    /**
     * @brief Platform-dependant signal registration function
     * @param signal signal code, e.g. SIGINT
     */
    void platform_registerSignal(int signalCode);

    /**
     * @brief Platform-dependant signal unregistration function
     * @param signal signal code, e.g. SIGINT
     */
    void platform_unregisterSignal(int signalCode);
};

}  // namespace lms

#endif /* LMS_SIGNAL_HANDLER_H */
