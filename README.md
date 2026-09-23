#Modern Parking System

A C++ implementation of the Data Structures and Algorithms parking-system task.

## What the system does

The system supports:

1. Live parking-slot display before entry.
2. Vehicle arrival registration.
3. Automatic slot allocation.
4. Automatic parking-duration calculation.
5. Automatic fee calculation using the task rates.
6. Payment by M-Pesa, card, or cash.
7. Exit/barrier control after confirmed payment.
8. Exception handling, configurable rates, and audit/reporting.

The task source specifies these parking charges:

- Up to 30 minutes: Free
- More than 30 minutes up to 2 hours: KSh 50
- More than 2 hours up to 4 hours: KSh 100
- More than 4 hours up to 6 hours: KSh 300
- More than 6 hours: KSh 500

## Project structure

ModernParkingSystem/
├── README.md
├── CMakeLists.txt
├── .gitignore
├── config/
│   └── rates.txt
├── data/
│   └── parking.db.sql
├── docs/
│   └── DESIGN.md
└── src/
    ├── main.cpp
    ├── ParkingSystem.h
    └── ParkingSystem.cpp


## Build with g++


g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp src/ParkingSystem.cpp -o parking_system
./parking_system


On Windows:


g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp src/ParkingSystem.cpp -o parking_system.exe
parking_system.exe


## Build with CMake


mkdir build
cd build
cmake ..
cmake --build .


## GitHub


git init
git add .
git commit -m "Initial modern parking system"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/modern-parking-system.git
git push -u origin main



## Payment note

The program records M-Pesa, card, and cash as payment methods. It does not connect to real M-Pesa/card services. Real payment APIs require credentials, network access, security controls, and a provider integration.

## Data structures used

- vector<ParkingSlot>: stores parking bays and their status.
- unordered_map<string, ParkingTicket>: finds an active vehicle quickly by number plate.
- queue<string>: keeps vehicles waiting for a free bay in first-in-first-out order.
- vector<PaymentRecord>: stores the payment/audit history.
- vector<RateBand>: stores flexible parking rates.

