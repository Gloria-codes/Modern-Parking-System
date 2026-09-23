#include "ParkingSystem.h"

// Include basic C++ libraries for text formatting, files, and math
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

// Constructor: Runs automatically when the parking system is created.
ParkingSystem::ParkingSystem(int numberOfSlots) {
    // Make sure the number of parking slots is at least 1.
    if (numberOfSlots < 1) {
        numberOfSlots = 1;
    }

    // Create all parking slots. At first, every slot is empty (false) and has no plate number.
    for (int i = 1; i <= numberOfSlots; ++i) {
        slots.push_back({i, false, ""});
    }

    // Load price rules from a file and save a system startup event.
    loadRates();
    addAudit("SYSTEM", "Parking system started");
}

// Convert a number plate to standard format: removes spaces and converts letters to uppercase.
string ParkingSystem::normalizePlate(const string& plate) const {
    string result;

    // Look at each character in the given plate string.
    for (char c : plate) {
        // Skip spaces and add capitalized letters/numbers to the result.
        if (!isspace(static_cast<unsigned char>(c))) {
            result += static_cast<char>(
                toupper(static_cast<unsigned char>(c)));
        }
    }

    return result;
}

// Convert date and time into readable text (Format: YYYY-MM-DD HH:MM:SS).
string ParkingSystem::timeToString(
    const chrono::system_clock::time_point& time) const {

    // Get raw time value.
    time_t rawTime = chrono::system_clock::to_time_t(time);
    tm localTime{};

// Check operating system and safely convert to local time format.
#ifdef _WIN32
    localtime_s(&localTime, &rawTime);
#else
    localtime_r(&rawTime, &localTime);
#endif

    // Format the time as a string.
    ostringstream out;
    out << put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

// Find the first empty parking spot. Returns slot ID, or -1 if full.
int ParkingSystem::findFreeSlot() const {
    // Loop through every slot.
    for (const auto& slot : slots) {
        // Return the ID of the first unoccupied slot found.
        if (!slot.occupied) {
            return slot.id;
        }
    }

    // Return -1 if no slots are available.
    return -1;
}

// Free up a specific parking slot by ID.
void ParkingSystem::freeSlot(int slotId) {
    for (auto& slot : slots) {
        if (slot.id == slotId) {
            slot.occupied = false; // Mark slot as free
            slot.plate.clear();    // Clear saved number plate
            return;
        }
    }
}

// Module 1: Display a visual status board of all parking bays.
void ParkingSystem::showSlots() const {
    cout << "\n========== LIVE PARKING BOARD ==========\n";

    int freeCount = 0;

    // Check and print the state of each bay.
    for (const auto& slot : slots) {
        cout << "[Bay " << setw(2) << slot.id << ": ";

        if (slot.occupied) {
            cout << "OCCUPIED - " << slot.plate;
        } else {
            cout << "FREE";
            ++freeCount; // Count free slots
        }

        cout << "]\n";
    }

    // Print summary showing free spots left.
    cout << "----------------------------------------\n";
    cout << "Free bays: " << freeCount << " / " << slots.size() << '\n';
    cout << "========================================\n";
}

// Load parking rates from the "config/rates.txt" file.
void ParkingSystem::loadRates() {
    rates.clear();

    // Try to open the configuration file.
    ifstream file("config/rates.txt");

    // If file is missing or cannot open, use default backup rates.
    if (!file) {
        rates = {
            {30, 0},     // 0 - 30 mins: Free
            {120, 50},   // Up to 120 mins: KSh 50
            {240, 100},  // Up to 240 mins: KSh 100
            {360, 300},  // Up to 360 mins: KSh 300
            {-1, 500}    // Over 360 mins: KSh 500
        };
        return;
    }

    // Read the file line-by-line.
    string line;
    while (getline(file, line)) {
        // Ignore blank lines and comments starting with '#'
        if (line.empty() || line[0] == '#') {
            continue;
        }

        istringstream input(line);
        RateBand band{};

        // Extract maximum minutes and price value.
        if (input >> band.maxMinutes >> band.amount) {
            rates.push_back(band);
        }
    }

    // If file was empty, load default rate list.
    if (rates.empty()) {
        rates = {
            {30, 0},
            {120, 50},
            {240, 100},
            {360, 300},
            {-1, 500}
        };
    }

    // Sort rates from shortest time limit to longest time limit.
    sort(rates.begin(), rates.end(),
         [](const RateBand& a, const RateBand& b) {
             if (a.maxMinutes == -1) return false;
             if (b.maxMinutes == -1) return true;
             return a.maxMinutes < b.maxMinutes;
         });
}

// Save rate changes back into the text file.
void ParkingSystem::saveRates() const {
    ofstream file("config/rates.txt");

    // Print error message if file cannot be created or opened.
    if (!file) {
        cerr << "Warning: could not save rates.\n";
        return;
    }

    file << "# Maximum minutes followed by price in KSh.\n";

    // Write all active rates to file.
    for (const auto& rate : rates) {
        file << rate.maxMinutes << ' ' << rate.amount << '\n';
    }
}

// Add a new record entry to the system history log.
void ParkingSystem::addAudit(const string& type,
                             const string& description,
                             double amount) {
    auditLog.push_back({
        chrono::system_clock::now(), // Current date and time
        type,                        // Event type (e.g., ENTRY, EXIT, ERROR)
        description,                 // Detailed description
        amount                       // Money amount (if applicable)
    });
}

// Module 2 + 3: Register a new vehicle entry and assign a parking bay.
void ParkingSystem::vehicleArrival() {
    string plate;

    // Ask user to enter the plate number.
    cout << "\nEnter vehicle number plate: ";
    getline(cin >> ws, plate);

    // Clean up plate string format.
    plate = normalizePlate(plate);

    // Check if plate input is empty.
    if (plate.empty()) {
        cout << "Invalid number plate.\n";
        return;
    }

    // Stop if the vehicle is already parked inside.
    if (activeTickets.find(plate) != activeTickets.end()) {
        cout << "This vehicle is already inside the car park.\n";
        addAudit("ERROR", "Duplicate vehicle entry: " + plate);
        return;
    }

    // Search for a free parking bay.
    int freeSlotId = findFreeSlot();

    // If parking lot is full, add vehicle to waiting line.
    if (freeSlotId == -1) {
        waitingVehicles.push(plate);
        cout << "Parking is full.\n";
        cout << "Vehicle was added to the waiting queue.\n";
        addAudit("QUEUE", "Vehicle added to waiting queue: " + plate);
        return;
    }

    // Create entry ticket for the vehicle.
    ParkingTicket ticket{
        nextTicketId++,
        plate,
        freeSlotId,
        chrono::system_clock::now() // Record current entry time
    };

    // Save ticket into active ticket list.
    activeTickets[plate] = ticket;

    // Mark the assigned slot as taken.
    for (auto& slot : slots) {
        if (slot.id == freeSlotId) {
            slot.occupied = true;
            slot.plate = plate;
            break;
        }
    }

    // Display ticket information on screen.
    cout << "\n========== ENTRY TICKET ==========\n";
    cout << "Ticket ID: " << ticket.ticketId << '\n';
    cout << "Plate:     " << ticket.plate << '\n';
    cout << "Bay:       " << ticket.slotId << '\n';
    cout << "Entry:     " << timeToString(ticket.entryTime) << '\n';
    cout << "==================================\n";

    addAudit("ENTRY", "Vehicle entered: " + plate);
}

// Module 4: Calculate total parking cost based on total minutes stayed.
double calculateFee(const vector<RateBand>& rates, long long minutes) {
    for (const auto& rate : rates) {
        // Return fee for the matching time band.
        if (rate.maxMinutes == -1 || minutes <= rate.maxMinutes) {
            return rate.amount;
        }
    }

    return 0.0;
}

// Module 5: Handle payment process for a parking ticket.
bool ParkingSystem::processPayment(const ParkingTicket& ticket,
                                   double amount) {
    cout << "\nAmount to pay: KSh " << fixed << setprecision(2)
         << amount << '\n';

    // Show available payment options.
    cout << "Payment method:\n";
    cout << "1. M-Pesa\n";
    cout << "2. Card\n";
    cout << "3. Cash\n";
    cout << "Choose method: ";

    int choice;
    cin >> choice;

    string method;

    // Set selected payment option string.
    if (choice == 1) {
        method = "M-Pesa";
    } else if (choice == 2) {
        method = "Card";
    } else if (choice == 3) {
        method = "Cash";
    } else {
        // Fail payment if invalid choice is given.
        cout << "Invalid payment method. Payment was not confirmed.\n";
        addAudit("PAYMENT_FAILED",
                 "Invalid payment method for ticket " +
                 to_string(ticket.ticketId));
        return false;
    }

    cout << method << " payment confirmed for this demo.\n";

    // Save payment details.
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

    return true; // Payment success
}

// Module 6: Handle exit process, calculate duration, collect payment, open barrier.
void ParkingSystem::vehicleExit() {
    string plate;

    cout << "\nEnter vehicle number plate: ";
    getline(cin >> ws, plate);

    plate = normalizePlate(plate);

    // Look for ticket matching plate number.
    auto found = activeTickets.find(plate);

    if (found == activeTickets.end()) {
        cout << "Vehicle not found among active vehicles.\n";
        addAudit("ERROR", "Exit requested for unknown vehicle: " + plate);
        return;
    }

    ParkingTicket ticket = found->second;
    auto exitTime = chrono::system_clock::now();

    // Calculate total seconds spent in parking.
    auto secondsStayed =
        chrono::duration_cast<chrono::seconds>(
            exitTime - ticket.entryTime).count();

    if (secondsStayed < 0) {
        cout << "Invalid system time. Exit cancelled.\n";
        return;
    }

    // Convert seconds into total minutes (rounds up partial minutes).
    long long minutes = (secondsStayed + 59) / 60;

    // Calculate parking price.
    double fee = calculateFee(rates, minutes);

    // Show exit receipt details.
    cout << "\n========== EXIT BILL ==========\n";
    cout << "Ticket ID: " << ticket.ticketId << '\n';
    cout << "Plate:     " << ticket.plate << '\n';
    cout << "Bay:       " << ticket.slotId << '\n';
    cout << "Entry:     " << timeToString(ticket.entryTime) << '\n';
    cout << "Exit:      " << timeToString(exitTime) << '\n';
    cout << "Duration:  " << minutes << " minute(s)\n";
    cout << "Amount:    KSh " << fixed << setprecision(2) << fee << '\n';
    cout << "===============================\n";

    // Process payment before opening exit barrier.
    if (!processPayment(ticket, fee)) {
        cout << "Payment was not confirmed.\n";
        cout << "Barrier remains CLOSED.\n";
        return;
    }

    // Open barrier after successful payment.
    cout << "\nPAYMENT CONFIRMED.\n";
    cout << "Barrier: OPEN\n";

    // Clear bay assignment and remove active ticket.
    freeSlot(ticket.slotId);
    activeTickets.erase(found);

    addAudit("EXIT", "Vehicle exited: " + ticket.plate, fee);

    // Check if any car in waiting queue can now enter.
    allocateWaitingVehicle();

    cout << "Barrier: CLOSED\n";
}

// Module 6: Move first waiting car from waiting queue into an open bay.
void ParkingSystem::allocateWaitingVehicle() {
    // Stop if no cars are waiting.
    if (waitingVehicles.empty()) {
        return;
    }

    // Stop if no free bays are available.
    int freeSlotId = findFreeSlot();

    if (freeSlotId == -1) {
        return;
    }

    // Get plate from front of waiting queue and remove it from queue.
    string plate = waitingVehicles.front();
    waitingVehicles.pop();

    // Create entry ticket for waiting vehicle.
    ParkingTicket ticket{
        nextTicketId++,
        plate,
        freeSlotId,
        chrono::system_clock::now()
    };

    activeTickets[plate] = ticket;

    // Mark slot as taken by vehicle.
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

// Display list of all cars currently waiting in queue line.
void ParkingSystem::showWaitingList() const {
    cout << "\n========== WAITING QUEUE ==========\n";

    if (waitingVehicles.empty()) {
        cout << "No vehicles are waiting.\n";
    } else {
        // Copy queue to read items without modifying original queue structure.
        queue<string> copy = waitingVehicles;
        int position = 1;

        // Print vehicles line by line.
        while (!copy.empty()) {
            cout << position++ << ". " << copy.front() << '\n';
            copy.pop();
        }
    }

    cout << "===================================\n";
}

// Module 7: Check administrator password.
bool ParkingSystem::adminLogin() const {
    string password;

    cout << "Admin password: ";
    cin >> password;

    // Returns true if password matches.
    return password == "admin123";
}

// Module 7: Display admin options menu for updating rates and viewing audit log.
void ParkingSystem::adminMenu() {
    // Check login credentials first.
    if (!adminLogin()) {
        cout << "Wrong password.\n";
        addAudit("ADMIN_FAILED", "Failed administrator login");
        return;
    }

    int choice = 0;

    // Loop until user picks option 4 (Exit Admin Menu).
    while (choice != 4) {
        cout << "\n========== ADMIN MENU ==========\n";
        cout << "1. View current rates\n";
        cout << "2. Change a rate\n";
        cout << "3. View audit log\n";
        cout << "4. Back\n";
        cout << "Choose: ";
        cin >> choice;

        if (choice == 1) {
            // Option 1: Show pricing rules.
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
            // Option 2: Modify price of a rate band.
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

            // Check if band choice is valid.
            if (band < 1 || band > static_cast<int>(rates.size())) {
                cout << "Invalid band.\n";
                continue;
            }

            cout << "New amount in KSh: ";
            cin >> amount;

            // Do not allow negative money amounts.
            if (amount < 0) {
                cout << "Amount cannot be negative.\n";
                continue;
            }

            // Update rate and write to file.
            rates[band - 1].amount = amount;
            saveRates();

            addAudit("RATE_CHANGE",
                     "Rate band " + to_string(band) + " changed",
                     amount);

            cout << "Rate saved. No recompilation is needed.\n";
        } else if (choice == 3) {
            // Option 3: Print system log history.
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

// Module 8: Print summary report of earnings and current status.
void ParkingSystem::showReport() const {
    double total = 0.0;
    double mpesa = 0.0;
    double card = 0.0;
    double cash = 0.0;

    // Calculate money totals by payment method.
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

    // Print summary report.
    cout << "\n========== MANAGEMENT REPORT ==========\n";
    cout << "Confirmed payments: " << payments.size() << '\n';
    cout << "M-Pesa total:       KSh " << fixed << setprecision(2)
         << mpesa << '\n';
    cout << "Card total:         KSh " << card << '\n';
    cout << "Cash total:         KSh " << cash << '\n';
    cout << "TOTAL COLLECTED:    KSh " << total << '\n';
    cout << "Active vehicles:    " << activeTickets.size() << '\n';
    cout << "Waiting vehicles:   " << waitingVehicles.size() << '\n';

    cout << "\nVAT: Not calculated because the assignment does not "
            "provide a VAT rate.\n";

    cout << "========================================\n";
}

// Main interactive menu loop for running the parking application.
void ParkingSystem::run() {
    int choice = 0;

    // Keep displaying system menu until user chooses option 7 (Exit).
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

        // Perform action matching user choice.
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