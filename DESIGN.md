# ParkSmart KE – Design, Algorithms and Answers

## 1. Analysis of the requirements

The assignment says that drivers should see available parking slots before entry, vehicles must be recorded on arrival, the system must calculate time and amount at exit, and the exit barrier must open after payment. It also gives the parking fee bands.

The provided project brief asks for:
- an algorithm for each module;
- data structures and reasons for their use;
- a dynamic database;
- a functional C++, Java, or Python system.

The project uses **eight modules** so every major operation has a clear responsibility.

---

# 2. The eight modules and algorithms

## Module 1 – Slot Monitoring and Display

### Purpose
Show free and occupied bays before a vehicle enters.

### Algorithm
1. Start.
2. Visit every parking slot.
3. Check whether the slot is occupied.
4. If it is free, mark it `FREE`.
5. If it is occupied, mark it `OCCUPIED`.
6. Count all free slots.
7. Display the slot map and total free slots.
8. Stop.

### Data structure
`vector<ParkingSlot>`.

### Why?
Parking slots are naturally stored as a list. A vector is simple and allows the program to visit every bay.

---

## Module 2 – Vehicle Arrival and Entry Recording

### Purpose
Record the vehicle number plate and entry time.

### Algorithm
1. Start.
2. Ask for the vehicle number plate.
3. Convert it to a consistent uppercase form.
4. Check whether the plate is already inside.
5. If it is already inside, reject the entry.
6. If there is no free slot, put the plate in the waiting queue.
7. Otherwise create a parking ticket.
8. Save the current entry time.
9. Allocate a free slot.
10. Save the ticket in the active-ticket table.
11. Print the ticket details.
12. Stop.

### Data structures
- `unordered_map<string, ParkingTicket>` for active vehicles.
- `queue<string>` for vehicles waiting for a bay.

### Why?
The hash map gives fast lookup by plate. The queue gives fair first-in-first-out waiting.

---

## Module 3 – Slot Allocation

### Purpose
Give an arriving vehicle a free bay.

### Algorithm
1. Start.
2. Visit the parking slots from the first slot to the last slot.
3. Find the first free slot.
4. Mark the slot occupied.
5. Store the vehicle plate in the slot.
6. Return the slot number.
7. If no slot is found, return "no slot".
8. Stop.

### Data structure
`vector<ParkingSlot>`.

### Why?
It is easy to scan and update a small/medium parking area. A larger version could use a set or indexed free-slot structure.

---

## Module 4 – Duration and Fee Calculation

### Purpose
Calculate how long the vehicle stayed and the amount to pay.

### Algorithm
1. Start.
2. Read entry time.
3. Read exit time.
4. Calculate the difference in seconds.
5. Convert the difference to minutes.
6. If the stay is 30 minutes or less, charge KSh 0.
7. If the stay is up to 120 minutes, charge KSh 50.
8. If the stay is up to 240 minutes, charge KSh 100.
9. If the stay is up to 360 minutes, charge KSh 300.
10. Otherwise charge KSh 500.
11. Return the duration and fee.
12. Stop.

### Data structure
`vector<RateBand>`.

### Why?
The rates can be changed in the configuration file or by the admin without changing the program code.

---

## Module 5 – Payment Collection

### Purpose
Collect and confirm payment.

### Algorithm
1. Start.
2. Receive the amount calculated by Module 4.
3. Ask for payment method: M-Pesa, card, or cash.
4. If the method is not valid, reject it.
5. Simulate payment confirmation.
6. If payment is confirmed, create a payment record.
7. Add the amount to the audit total.
8. Return payment confirmation.
9. If payment fails, do not open the barrier.
10. Stop.

### Data structure
`vector<PaymentRecord>`.

### Why?
Every successful payment is kept in order for reconciliation and audit.

---

## Module 6 – Exit and Barrier Control

### Purpose
Open the barrier only after payment is confirmed.

### Algorithm
1. Start.
2. Ask for the number plate.
3. Find the vehicle in the active-ticket map.
4. If it does not exist, show an error.
5. Calculate duration and fee.
6. Collect payment.
7. If payment is confirmed:
   - open the barrier;
   - mark the parking slot free;
   - remove the vehicle from active tickets;
   - record exit time.
8. Try to allocate the newly free slot to the first waiting vehicle.
9. Close the barrier.
10. Stop.

### Data structures
`unordered_map`, `vector`, and `queue`.

### Why?
The map finds the vehicle, the vector updates the bay, and the queue handles waiting vehicles fairly.

---

## Module 7 – Exception Handling and Administration

### Purpose
Handle invalid operations and allow management to change parking rates.

### Algorithm
1. Start.
2. Ask the administrator for the password.
3. If authentication fails, reject the operation.
4. For rate editing, ask for the maximum minutes and price.
5. Validate the values.
6. Save the new rate table to `config/rates.txt`.
7. Record the rate change in the audit log.
8. For invalid vehicle plates or invalid payment methods, display a clear error.
9. Stop.

### Data structure
`vector<RateBand>` and `vector<AuditRecord>`.

### Why?
Rates are ordered by maximum time. Audit records preserve a history of important actions.

---

## Module 8 – Reporting, Audit and VAT Support

### Purpose
Produce an auditable record of money collected and completed exits.

### Algorithm
1. Start.
2. Read all payment records.
3. Count the number of payments.
4. Add every confirmed payment amount.
5. Group or display payments by method.
6. Display the gross amount collected.
7. Display completed exits.
8. Display audit events.
9. Display a VAT field if a VAT rate has been configured.
10. Stop.

### Important VAT note
The supplied assignment says the system should support reconciliation and VAT, but it does not give a VAT percentage. Therefore this project does **not** invent a VAT rate. A future database/configuration value can be used for the actual rate approved by management.

### Data structures
`vector<PaymentRecord>` and `vector<AuditRecord>`.

### Why?
A chronological list makes auditing and reconciliation simple.

---

# 3. Parking fee algorithm

The source gives these bands:

| Stay | Fee |
|---|---:|
| 0–30 minutes | KSh 0 |
| 31–120 minutes | KSh 50 |
| 121–240 minutes | KSh 100 |
| 241–360 minutes | KSh 300 |
| More than 360 minutes | KSh 500 |

The code reads these bands from `config/rates.txt`, so management can change them without recompiling.

---

# 4. Dynamic database design

The database has five main areas:

1. `parking_slots`
   - slot ID
   - occupied status
   - vehicle plate

2. `parking_tickets`
   - ticket ID
   - plate number
   - entry time
   - exit time
   - slot
   - status

3. `rate_bands`
   - rate ID
   - maximum minutes
   - amount

4. `payments`
   - payment ID
   - ticket ID
   - amount
   - payment method
   - payment time
   - confirmation

5. `audit_log`
   - audit ID
   - event time
   - event type
   - description
   - amount

The SQL version is in `data/parking.db.sql`.

---

# 5. Why these data structures were selected

| Data structure | Used for | Reason |
|---|---|---|
| vector | slots, rates, payments, audit | Simple ordered storage |
| unordered_map | active tickets | Fast search by number plate |
| queue | waiting vehicles | First vehicle waiting is served first |
| struct/class | ticket, payment, slot, audit | Keeps related information together |

---

# 6. Complexity

Let `n` be the number of parking slots.

- Display slots: `O(n)`
- Find a free slot: `O(n)` in the simple implementation.
- Find active vehicle by plate: average `O(1)` with `unordered_map`.
- Add payment: `O(1)` amortized.
- Add waiting vehicle: `O(1)`.
- Remove waiting vehicle: `O(1)`.

The simple `O(n)` slot scan is suitable for a small/medium parking area and is easy to understand for a DSA primer task.

---

# 7. Test cases

## Test 1 – Free parking
Entry and exit are 20 minutes apart.
Expected fee: KSh 0.

## Test 2 – Two-hour band
Stay is 120 minutes.
Expected fee: KSh 50.

## Test 3 – Four-hour band
Stay is 240 minutes.
Expected fee: KSh 100.

## Test 4 – Six-hour band
Stay is 360 minutes.
Expected fee: KSh 300.

## Test 5 – More than six hours
Stay is 361 minutes or more.
Expected fee: KSh 500.

## Test 6 – Full parking
Fill all bays.
Expected result: the next vehicle is placed in the waiting queue.

## Test 7 – Duplicate vehicle
Enter the same plate twice.
Expected result: second entry is rejected.

## Test 8 – Payment failure
Choose an invalid payment method.
Expected result: barrier remains closed.

---

# 8. Use cases

### Driver
- View free slots.
- Enter vehicle.
- Receive allocated bay.
- Pay at exit.
- Exit after payment.

### Parking management
- View slots.
- View reports.
- Change parking rates.
- Review audit information.

### System
- Record arrivals.
- Allocate slots.
- Calculate time.
- Calculate fee.
- Confirm payment.
- Control barrier.
- Record transactions.
