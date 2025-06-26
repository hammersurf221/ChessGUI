#include <Python.h>
#include <iostream>

int main() {
    Py_Initialize();
    if (Py_IsInitialized()) {
        std::cout << "Python initialized successfully" << std::endl;
        Py_Finalize();
        return 0;
    } else {
        std::cout << "Failed to initialize Python" << std::endl;
        return 1;
    }
} 