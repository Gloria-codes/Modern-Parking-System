#Modern Parking Management System

## 1. Project Overview

ParkSmart KE is a modern parking management system developed in C++ for a parking facility in Kenya.

The system automates the main parking operations. It allows drivers to see available parking bays, records vehicles when they enter, allocates parking bays, calculates parking duration, calculates the parking fee, records payment and allows a vehicle to exit only after payment has been confirmed.

The project was developed as part of a Data Structures and Algorithms task in the Computer Science course.

The system uses simple and practical data structures such as vectors, queues, hash maps and structures to solve the parking management problem.

---

## 2. Objectives

The main objectives of ParkSmart KE are:

- Display available and occupied parking bays.
- Record every vehicle when it arrives.
- Record the vehicle number plate and entry time.
- Automatically allocate an available parking bay.
- Calculate parking duration at exit.
- Automatically calculate the amount payable.
- Support M-Pesa, card and cash payment records.
- Open the exit barrier only after payment is confirmed.
- Allow management to change parking rates without changing the main program.
- Keep an audit record of important transactions.
- Produce management and payment reports.
- Provide a database design for future permanent storage.

---

## 3. Parking Fee Structure

The parking charges used by the system are:

| Parking Duration | Fee |
|---|---:|
| Up to 30 minutes | KSh 0 |
| More than 30 minutes up to 2 hours | KSh 50 |
| More than 2 hours up to 4 hours | KSh 100 |
| More than 4 hours up to 6 hours | KSh 300 |
| More than 6 hours | KSh 500 |

The rates are stored separately in:

`config/rates.txt`

This allows management to change parking rates without changing the main C++ source code.

---

## 4. Eight System Modules

### Module 1 – Slot Monitoring and Display

Displays the current status of every parking bay.

It shows:

- Bay number
- FREE or OCCUPIED status
- Vehicle number plate for occupied bays
- Total number of available bays

### Module 2 – Vehicle Arrival

Records a vehicle when it enters the parking facility.

It records:

- Vehicle number plate
- Ticket ID
- Entry time
- Allocated parking bay

The system also prevents the same vehicle from being registered twice while it is already inside.

### Module 3 – Slot Allocation

Searches for a free parking bay and assigns it to an arriving vehicle.

If all bays are occupied, the vehicle is placed in a waiting queue.

### Module 4 – Duration and Fee Calculation

Calculates how long a vehicle has stayed in the parking facility.

The system then uses the configured parking rates to calculate the amount payable.

### Module 5 – Payment Collection

The system supports:

1. M-Pesa
2. Card
3. Cash

The current version simulates payment confirmation for academic demonstration purposes.

### Module 6 – Exit and Barrier Control

The system checks the vehicle, calculates the fee and requests payment.

The barrier is opened only after payment has been confirmed.

After the vehicle leaves:

- The parking bay becomes free.
- The active vehicle record is removed.
- The exit is recorded.
- A waiting vehicle can be allocated to the newly available bay.

### Module 7 – Administration and Exception Handling

The administrator can:

- View parking rates.
- Change parking rates.
- View audit records.
- Handle invalid operations.

The demonstration administrator password is:

`admin123`

This password should be changed before any real deployment.

### Module 8 – Reports and Audit

This module provides management information such as:

- Total confirmed payments
- M-Pesa collections
- Card collections
- Cash collections
- Total amount collected
- Number of active vehicles
- Number of waiting vehicles
- Audit events

---

## 5. Data Structures Used

| Data Structure | Purpose | Reason |
|---|---|---|
| `vector` | Parking bays, rates, payments and audit records | Simple ordered storage |
| `unordered_map` | Active vehicles | Fast searching using a number plate |
| `queue` | Waiting vehicles | First-in-first-out processing |
| `struct` | Tickets, vehicles, payments and audit records | Groups related information together |

Example:

```cpp
unordered_map<string, ParkingTicket> activeTickets;
queue<string> waitingVehicles;
vector<ParkingSlot> slots;

Author
*Gloria Magoma

Course
*Computer Science

