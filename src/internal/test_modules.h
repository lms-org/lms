#include "lms/module.h"

namespace lms {
namespace internal {

/**
 * @brief Test module that writes an increasing
 * integer into a channel named OUT
 *
 * Starting number and step with can be configured.
 */
class NumberGenerator : public lms::Module {
public:
    ~NumberGenerator();
    bool init() override;
    bool cycle() override;
    void destroy() override;
private:
    lms::WriteDataChannel<int> out;
    int currentNumber;
};

class Multiply : public lms::Module {
public:
    ~Multiply();
    bool init() override;
    bool cycle() override;
    void destroy() override;
private:
    lms::ReadDataChannel<int> factor1;
    lms::ReadDataChannel<int> factor2;
    lms::WriteDataChannel<int> product;
};

class Display : public lms::Module {
public:
    ~Display();
    bool init() override;
    bool cycle() override;
    void destroy() override;
private:
    lms::ReadDataChannel<int> in;
};

class ExitAfter : public lms::Module {
    bool init() override;
    bool cycle() override;
    void destroy() override;
};

}  // namespace internal
}  // namespace lms
