#include "ParkingSystem.h"
#include <iostream>

int main() {
    std::cout << "Starting ParkSmart KE...\n";

    // Change 20 to the number of parking bays needed by the client.
    ParkingSystem system(20);

    system.run();

    return 0;
}
