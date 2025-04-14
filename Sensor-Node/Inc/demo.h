#ifndef DEMO_H
#define DEMO_H

#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes for C++ functions that will be called from C
void demo_init(void);
const char* demo_get_message(void); // Function to get a message from C++

#ifdef __cplusplus
}
#endif

#endif // DEMO_H