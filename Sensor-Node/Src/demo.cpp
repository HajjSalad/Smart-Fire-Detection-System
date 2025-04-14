#include "demo.h"
#include <string>

// A simple C++ class to demonstrate C++ functionality
class Demo {
public:
    std::string getMessage() {
        return "Hello from C++!"; // Return a string from C++
    }
};

// Global instance of the Demo class
static Demo demoInstance;

// C++ function that initializes the demo
void demo_init(void) {
    // Initialization code (if needed)
}

// C++ function that returns a message
const char* demo_get_message(void) {
    static std::string message = demoInstance.getMessage();
    return message.c_str(); // Return a C-style string
}
