# ParkSmart KE – Modern Parking System

A C++17 implementation of the Data Structures and Algorithms parking-system task.

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

Source: `DSA_Complete_Task1_project 2.pdf`.

## Important note about the web requirement

The assignment asks for a web-based system, but it also requires C++ and does not specify a web framework. This repository therefore keeps the core system as a portable C++17 application with clean modules. The same classes can be connected to a C++ web framework such as Crow later without changing the parking rules.

## Project structure

```text
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
```

## Build with g++

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp src/ParkingSystem.cpp -o parking_system
./parking_system
```

On Windows:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic src/main.cpp src/ParkingSystem.cpp -o parking_system.exe
parking_system.exe
```

## Build with CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## GitHub

```bash
git init
git add .
git commit -m "Initial modern parking system"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/modern-parking-system.git
git push -u origin main
```

Replace `YOUR_USERNAME` with your GitHub username.

## Login

The demo admin password is:

```text
admin123
```

Change it before using the project in a real environment.

## Payment note

The program records M-Pesa, card, and cash as payment methods. It does **not** connect to real M-Pesa/card services. Real payment APIs require credentials, network access, security controls, and a provider integration.

## Data structures used

- `vector<ParkingSlot>`: stores parking bays and their status.
- `unordered_map<string, ParkingTicket>`: finds an active vehicle quickly by number plate.
- `queue<string>`: keeps vehicles waiting for a free bay in first-in-first-out order.
- `vector<PaymentRecord>`: stores the payment/audit history.
- `vector<RateBand>`: stores configurable parking rates.

## Eight modules

See `docs/DESIGN.md` for the algorithms and data-structure explanation for each module.
