-- Dynamic database design for ParkSmart KE.
-- This is a relational design document.
-- The C++ demo stores data in memory so it can run without external packages.

CREATE TABLE parking_slots (
    slot_id INTEGER PRIMARY KEY,
    occupied INTEGER NOT NULL DEFAULT 0,
    vehicle_plate VARCHAR(20)
);

CREATE TABLE parking_tickets (
    ticket_id INTEGER PRIMARY KEY,
    plate_number VARCHAR(20) NOT NULL,
    entry_time DATETIME NOT NULL,
    exit_time DATETIME,
    slot_id INTEGER NOT NULL,
    status VARCHAR(20) NOT NULL,
    FOREIGN KEY (slot_id) REFERENCES parking_slots(slot_id)
);

CREATE TABLE rate_bands (
    rate_id INTEGER PRIMARY KEY,
    max_minutes INTEGER,
    amount DECIMAL(10,2) NOT NULL
);

CREATE TABLE payments (
    payment_id INTEGER PRIMARY KEY,
    ticket_id INTEGER NOT NULL,
    amount DECIMAL(10,2) NOT NULL,
    method VARCHAR(20) NOT NULL,
    payment_time DATETIME NOT NULL,
    confirmed INTEGER NOT NULL,
    FOREIGN KEY (ticket_id) REFERENCES parking_tickets(ticket_id)
);

CREATE TABLE audit_log (
    audit_id INTEGER PRIMARY KEY,
    event_time DATETIME NOT NULL,
    event_type VARCHAR(50) NOT NULL,
    description VARCHAR(255) NOT NULL,
    amount DECIMAL(10,2)
);
