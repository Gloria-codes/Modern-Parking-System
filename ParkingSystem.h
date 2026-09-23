#ifndef PARKING_SYSTEM_H
#define PARKING_SYSTEM_H

#include <chrono>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

// One parking bay.
struct ParkingSlot {
    int id;
    bool occupied = false;
    std::string plate;
};

// One parking ticket for a vehicle inside the car park.
struct ParkingTicket {
    int ticketId;
    std::string plate;
    int slotId;
    std::chrono::system_clock::time_point entryTime;
};

// One parking price band.
struct RateBand {
    int maxMinutes; // -1 means there is no upper limit.
    double amount;
};

// One successful payment.
struct PaymentRecord {
    int paymentId;
    int ticketId;
    std::string plate;
    double amount;
    std::string method;
    std::chrono::system_clock::time_point paymentTime;
};

// One audit event.
struct AuditRecord {
    std::chrono::system_clock::time_point time;
    std::string eventType;
    std::string description;
    double amount = 0.0;
};

class ParkingSystem {
private:
    std::vector<ParkingSlot> slots;
    std::unordered_map<std::string, ParkingTicket> activeTickets;
    std::queue<std::string> waitingVehicles;
    std::vector<RateBand> rates;
    std::vector<PaymentRecord> payments;
    std::vector<AuditRecord> auditLog;

    int nextTicketId = 1;
    int nextPaymentId = 1;

    std::string normalizePlate(const std::string& plate) const;
    std::string timeToString(
        const std::chrono::system_clock::time_point& time) const;

    int findFreeSlot() const;
    void freeSlot(int slotId);
    void saveRates() const;
    void loadRates();
    void addAudit(const std::string& type,
                  const std::string& description,
                  double amount = 0.0);

    bool adminLogin() const;
    bool processPayment(const ParkingTicket& ticket, double amount);
    void allocateWaitingVehicle();

public:
    explicit ParkingSystem(int numberOfSlots = 20);

    // Module 1
    void showSlots() const;

    // Module 2 + 3
    void vehicleArrival();

    // Module 4 + 5 + 6
    void vehicleExit();

    // Module 7
    void adminMenu();

    // Module 8
    void showReport() const;

    void showWaitingList() const;
    void run();
};

#endif
