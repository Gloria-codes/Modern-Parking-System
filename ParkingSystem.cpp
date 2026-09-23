#include "ParkingSystem.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

ParkingSystem::ParkingSystem(int numberOfSlots) {
    if (numberOfSlots < 1) {
        numberOfSlots = 1;
    }

    for (int i = 1; i <= numberOfSlots; ++i) {
        slots.push_back({i, false, ""});
    }

    loadRates();
    addAudit("SYSTEM", "Parking system started");
}

// Change a plate to one standard form.
string ParkingSystem::normalizePlate(const string& plate) const {
    string result;

    for (char c : plate) {
        if (!isspace(static_cast<unsigned char>(c))) {
            result += static_cast<char>(
                toupper(static_cast<unsigned char>(c)));
        }
    }

    return result;
}

string ParkingSystem::timeToString(
    const chrono::system_clock::time_point& time) const {

    time_t rawTime = chrono::system_clock::to_time_t(time);
    tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &rawTime);
#else
    localtime_r(&rawTime, &localTime);
#endif

    ostringstream out;
    out << put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

// Find the first free slot.
int ParkingSystem::findFreeSlot() const {
    for (const auto& slot : slots) {
        if (!slot.occupied) {
            return slot.id;
        }
    }

    return -1;
}

void ParkingSystem::freeSlot(int slotId) {
    for (auto& slot : slots) {
        if (slot.id == slotId) {
            slot.occupied = false;
            slot.plate.clear();
            return;
        }
    }
}

// Module 1: show a simple visual slot board.
void ParkingSystem::showSlots() const {
    cout << "\n========== LIVE PARKING BOARD ==========\n";

    int freeCount = 0;

    for (const auto& slot : slots) {
        cout << "[Bay " << setw(2) << slot.id << ": ";

        if (slot.occupied) {
            cout << "OCCUPIED - " << slot.plate;
        } else {
            cout << "FREE";
            ++freeCount;
        }

        cout << "]\n";
    }

    cout << "----------------------------------------\n";
    cout << "Free bays: " << freeCount << " / " << slots.size() << '\n';
    cout << "========================================\n";
}

// Load parking rates from a text file.
void ParkingSystem::loadRates() {
    rates.clear();

    ifstream file("config/rates.txt");

    if (!file) {
        // Safe default rates from the assignment.
        rates = {
            {30, 0},
            {120, 50},
            {240, 100},
            {360, 300},
            {-1, 500}
        };
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        istringstream input(line);
        RateBand band{};

        if (input >> band.maxMinutes >> band.amount) {
            rates.push_back(band);
        }
    }

    if (rates.empty()) {
        rates = {
            {30, 0},
            {120, 50},
            {240, 100},
            {360, 300},
            {-1, 500}
        };
    }

    sort(rates.begin(), rates.end(),
         [](const RateBand& a, const RateBand& b) {
             if (a.maxMinutes == -1) return false;
             if (b.maxMinutes == -1) return true;
             return a.maxMinutes < b.maxMinutes;
         });
}

// Save rates so management changes remain after restarting.
void ParkingSystem::saveRates() const {
    ofstream file("config/rates.txt");

    if (!file) {
        cerr << "Warning: could not save rates.\n";
        return;
    }

    file << "# Maximum minutes followed by price in KSh.\n";

    for (const auto& rate : rates) {
        file << rate.maxMinutes << ' ' << rate.amount << '\n';
    }
}

void ParkingSystem::addAudit(const string& type,
                             const string& description,
                             double amount) {
    auditLog.push_back({
        chrono::system_clock::now(),
        type,
        description,
        amount
    });
}

// Module 2 + 3: register a vehicle and allocate a bay.
void ParkingSystem::vehicleArrival() {
    string plate;

    cout << "\nEnter vehicle number plate: ";
    getline(cin >> ws, plate);

    plate = normalizePlate(plate);

    if (plate.empty()) {
        cout << "Invalid number plate.\n";
        return;
    }

    // Do not allow one vehicle to have two active tickets.
    if (activeTickets.find(plate) != activeTickets.end()) {
        cout << "This vehicle is already inside the car park.\n";
        addAudit("ERROR", "Duplicate vehicle entry: " + plate);
        return;
    }

    int freeSlotId = findFreeSlot();

    if (freeSlotId == -1) {
        waitingVehicles.push(plate);
        cout << "Parking is full.\n";
        cout << "Vehicle was added to the waiting queue.\n";
        addAudit("QUEUE", "Vehicle added to waiting queue: " + plate);
        return;
    }

    ParkingTicket ticket{
        nextTicketId++,
        plate,
        freeSlotId,
        chrono::system_clock::now()
    };

    activeTickets[plate] = ticket;

    for (auto& slot : slots) {
        if (slot.id == freeSlotId) {
            slot.occupied = true;
            slot.plate = plate;
            break;
        }
    }

    cout << "\n========== ENTRY TICKET ==========\n";
    cout << "Ticket ID: " << ticket.ticketId << '\n';
    cout << "Plate:     " << ticket.plate << '\n';
    cout << "Bay:       " << ticket.slotId << '\n';
    cout << "Entry:     " << timeToString(ticket.entryTime) << '\n';
    cout << "==================================\n";

    addAudit("ENTRY", "Vehicle entered: " + plate);
}

// Module 4: calculate a fee using the configurable rate bands.
double calculateFee(const vector<RateBand>& rates, long long minutes) {
    for (const auto& rate : rates) {
        if (rate.maxMinutes == -1 || minutes <= rate.maxMinutes) {
            return rate.amount;
        }
    }

    // This should not happen when the final rate has -1.
    return 0.0;
}

// Module 5: collect and confirm payment.
bool ParkingSystem::processPayment(const ParkingTicket& ticket,
                                   double amount) {
    cout << "\nAmount to pay: KSh " << fixed << setprecision(2)
         << amount << '\n';

    cout << "Payment method:\n";
    cout << "1. M-Pesa\n";
    cout << "2. Card\n";
    cout << "3. Cash\n";
    cout << "Choose method: ";

    int choice;
    cin >> choice;

    string method;

    if (choice == 1) {
        method = "M-Pesa";
    } else if (choice == 2) {
        method = "Card";
    } else if (choice == 3) {
        method = "Cash";
    } else {
        cout << "Invalid payment method. Payment was not confirmed.\n";
        addAudit("PAYMENT_FAILED",
                 "Invalid payment method for ticket " +
                 to_string(ticket.ticketId));
        return false;
    }

    // This is a safe simulation of payment confirmation.
    cout << method << " payment confirmed for this demo.\n";

    PaymentRecord payment{
        nextPaymentId++,
        ticket.ticketId,
        ticket.plate,
        amount,
        method,
        chrono::system_clock::now()
    };

    payments.push_back(payment);

    addAudit("PAYMENT",
             "Payment received using " + method +
             " for " + ticket.plate,
             amount);

    return true;
}

// Module 6: exit control and barrier logic.
void ParkingSystem::vehicleExit() {
    string plate;

    cout << "\nEnter vehicle number plate: ";
    getline(cin >> ws, plate);

    plate = normalizePlate(plate);

    auto found = activeTickets.find(plate);

    if (found == activeTickets.end()) {
        cout << "Vehicle not found among active vehicles.\n";
        addAudit("ERROR", "Exit requested for unknown vehicle: " + plate);
        return;
    }

    ParkingTicket ticket = found->second;
    auto exitTime = chrono::system_clock::now();

    auto secondsStayed =
        chrono::duration_cast<chrono::seconds>(
            exitTime - ticket.entryTime).count();

    if (secondsStayed < 0) {
        cout << "Invalid system time. Exit cancelled.\n";
        return;
    }

    // Round up partial minutes so that 30 minutes and a few seconds
    // are treated as the next full minute.
    long long minutes = (secondsStayed + 59) / 60;

    double fee = calculateFee(rates, minutes);

    cout << "\n========== EXIT BILL ==========\n";
    cout << "Ticket ID: " << ticket.ticketId << '\n';
    cout << "Plate:     " << ticket.plate << '\n';
    cout << "Bay:       " << ticket.slotId << '\n';
    cout << "Entry:     " << timeToString(ticket.entryTime) << '\n';
    cout << "Exit:      " << timeToString(exitTime) << '\n';
    cout << "Duration:  " << minutes << " minute(s)\n";
    cout << "Amount:    KSh " << fixed << setprecision(2) << fee << '\n';
    cout << "===============================\n";

    if (!processPayment(ticket, fee)) {
        cout << "Payment was not confirmed.\n";
        cout << "Barrier remains CLOSED.\n";
        return;
    }

    // Only after payment do we open the barrier.
    cout << "\nPAYMENT CONFIRMED.\n";
    cout << "Barrier: OPEN\n";

    freeSlot(ticket.slotId);
    activeTickets.erase(found);

    addAudit("EXIT", "Vehicle exited: " + ticket.plate, fee);

    // Give the freed bay to the first waiting vehicle.
    allocateWaitingVehicle();

    cout << "Barrier: CLOSED\n";
}

// Module 6: move the first waiting vehicle into a newly free bay.
void ParkingSystem::allocateWaitingVehicle() {
    if (waitingVehicles.empty()) {
        return;
    }

    int freeSlotId = findFreeSlot();

    if (freeSlotId == -1) {
        return;
    }

    string plate = waitingVehicles.front();
    waitingVehicles.pop();

    ParkingTicket ticket{
        nextTicketId++,
        plate,
        freeSlotId,
        chrono::system_clock::now()
    };

    activeTickets[plate] = ticket;

    for (auto& slot : slots) {
        if (slot.id == freeSlotId) {
            slot.occupied = true;
            slot.plate = plate;
            break;
        }
    }

    cout << "Waiting vehicle " << plate
         << " was allocated bay " << freeSlotId << ".\n";

    addAudit("QUEUE_EXIT",
             "Waiting vehicle allocated: " + plate);
}

void ParkingSystem::showWaitingList() const {
    cout << "\n========== WAITING QUEUE ==========\n";

    if (waitingVehicles.empty()) {
        cout << "No vehicles are waiting.\n";
    } else {
        // Copy the queue because reading it should not change it.
        queue<string> copy = waitingVehicles;
        int position = 1;

        while (!copy.empty()) {
            cout << position++ << ". " << copy.front() << '\n';
            copy.pop();
        }
    }

    cout << "===================================\n";
}

// Module 7: administrator functions.
bool ParkingSystem::adminLogin() const {
    string password;

    cout << "Admin password: ";
    cin >> password;

    return password == "admin123";
}

void ParkingSystem::adminMenu() {
    if (!adminLogin()) {
        cout << "Wrong password.\n";
        addAudit("ADMIN_FAILED", "Failed administrator login");
        return;
    }

    int choice = 0;

    while (choice != 4) {
        cout << "\n========== ADMIN MENU ==========\n";
        cout << "1. View current rates\n";
        cout << "2. Change a rate\n";
        cout << "3. View audit log\n";
        cout << "4. Back\n";
        cout << "Choose: ";
        cin >> choice;

        if (choice == 1) {
            cout << "\nCurrent rates:\n";

            for (const auto& rate : rates) {
                if (rate.maxMinutes == -1) {
                    cout << "Over 360 minutes: KSh "
                         << rate.amount << '\n';
                } else {
                    cout << "Up to " << rate.maxMinutes
                         << " minutes: KSh "
                         << rate.amount << '\n';
                }
            }
        } else if (choice == 2) {
            int band;
            double amount;

            cout << "\nChoose rate band:\n";
            for (size_t i = 0; i < rates.size(); ++i) {
                cout << i + 1 << ". ";

                if (rates[i].maxMinutes == -1) {
                    cout << "Over 360 minutes";
                } else {
                    cout << "Up to " << rates[i].maxMinutes
                         << " minutes";
                }

                cout << " - KSh " << rates[i].amount << '\n';
            }

            cout << "Band number: ";
            cin >> band;

            if (band < 1 || band > static_cast<int>(rates.size())) {
                cout << "Invalid band.\n";
                continue;
            }

            cout << "New amount in KSh: ";
            cin >> amount;

            if (amount < 0) {
                cout << "Amount cannot be negative.\n";
                continue;
            }

            rates[band - 1].amount = amount;
            saveRates();

            addAudit("RATE_CHANGE",
                     "Rate band " + to_string(band) + " changed",
                     amount);

            cout << "Rate saved. No recompilation is needed.\n";
        } else if (choice == 3) {
            cout << "\n========== AUDIT LOG ==========\n";

            for (const auto& record : auditLog) {
                cout << timeToString(record.time)
                     << " | " << record.eventType
                     << " | " << record.description;

                if (record.amount != 0.0) {
                    cout << " | KSh "
                         << fixed << setprecision(2)
                         << record.amount;
                }

                cout << '\n';
            }

            cout << "================================\n";
        } else if (choice != 4) {
            cout << "Invalid choice.\n";
        }
    }
}

// Module 8: report and reconciliation.
void ParkingSystem::showReport() const {
    double total = 0.0;
    double mpesa = 0.0;
    double card = 0.0;
    double cash = 0.0;

    for (const auto& payment : payments) {
        total += payment.amount;

        if (payment.method == "M-Pesa") {
            mpesa += payment.amount;
        } else if (payment.method == "Card") {
            card += payment.amount;
        } else if (payment.method == "Cash") {
            cash += payment.amount;
        }
    }

    cout << "\n========== MANAGEMENT REPORT ==========\n";
    cout << "Confirmed payments: " << payments.size() << '\n';
    cout << "M-Pesa total:       KSh " << fixed << setprecision(2)
         << mpesa << '\n';
    cout << "Card total:         KSh " << card << '\n';
    cout << "Cash total:         KSh " << cash << '\n';
    cout << "TOTAL COLLECTED:    KSh " << total << '\n';
    cout << "Active vehicles:    " << activeTickets.size() << '\n';
    cout << "Waiting vehicles:   " << waitingVehicles.size() << '\n';

    // The assignment mentions VAT but does not give a VAT percentage.
    cout << "\nVAT: Not calculated because the assignment does not "
            "provide a VAT rate.\n";

    cout << "========================================\n";
}

void ParkingSystem::run() {
    int choice = 0;

    while (choice != 7) {
        cout << "\n\n========================================\n";
        cout << "          PARKSMART KE\n";
        cout << "       MODERN PARKING SYSTEM\n";
        cout << "========================================\n";
        cout << "1. View live parking board\n";
        cout << "2. Register vehicle arrival\n";
        cout << "3. Process vehicle exit\n";
        cout << "4. Management report\n";
        cout << "5. Waiting queue\n";
        cout << "6. Admin / change rates\n";
        cout << "7. Exit program\n";
        cout << "Choose: ";

        cin >> choice;

        switch (choice) {
        case 1:
            showSlots();
            break;

        case 2:
            showSlots();
            vehicleArrival();
            break;

        case 3:
            vehicleExit();
            break;

        case 4:
            showReport();
            break;

        case 5:
            showWaitingList();
            break;

        case 6:
            adminMenu();
            break;

        case 7:
            cout << "Goodbye.\n";
            break;

        default:
            cout << "Invalid choice. Please try again.\n";
            break;
        }
    }
}
