-- Lab 05: Database Design from Requirements (MediTrack)
-- schema.sql: Relational Schema DDL for PostgreSQL / SQLite

DROP TABLE IF EXISTS bill;
DROP TABLE IF EXISTS prescription_drug;
DROP TABLE IF EXISTS drug;
DROP TABLE IF EXISTS prescription;
DROP TABLE IF EXISTS appointment;
DROP TABLE IF EXISTS doctor;
DROP TABLE IF EXISTS patient_phone;
DROP TABLE IF EXISTS patient;
DROP TABLE IF EXISTS clinic;

-- 1. Clinic Entity
CREATE TABLE clinic (
    clinic_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    address TEXT NOT NULL,
    phone VARCHAR(20) NOT NULL
);

-- 2. Patient Entity
CREATE TABLE patient (
    patient_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    dob DATE NOT NULL,
    blood_group VARCHAR(5) NOT NULL CHECK (blood_group IN ('A+', 'A-', 'B+', 'B-', 'AB+', 'AB-', 'O+', 'O-'))
);

-- 3. Patient Multivalued Attribute (Phone Numbers)
CREATE TABLE patient_phone (
    patient_id INTEGER NOT NULL REFERENCES patient(patient_id) ON DELETE CASCADE,
    phone VARCHAR(20) NOT NULL,
    PRIMARY KEY (patient_id, phone)
);

-- 4. Doctor Entity (1:N with Clinic)
CREATE TABLE doctor (
    doctor_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    specialization VARCHAR(100) NOT NULL,
    clinic_id INTEGER NOT NULL REFERENCES clinic(clinic_id) ON DELETE RESTRICT
);

-- 5. Appointment Entity (connects Patient, Doctor, Clinic)
CREATE TABLE appointment (
    appointment_id SERIAL PRIMARY KEY,
    patient_id INTEGER NOT NULL REFERENCES patient(patient_id) ON DELETE CASCADE,
    doctor_id INTEGER NOT NULL REFERENCES doctor(doctor_id) ON DELETE CASCADE,
    clinic_id INTEGER NOT NULL REFERENCES clinic(clinic_id) ON DELETE CASCADE,
    appointment_date DATE NOT NULL,
    appointment_time TIME NOT NULL,
    reason TEXT NOT NULL,
    status VARCHAR(20) NOT NULL DEFAULT 'booked' CHECK (status IN ('booked', 'completed', 'cancelled'))
);

-- 6. Prescription Entity (1:1 with completed Appointment)
CREATE TABLE prescription (
    prescription_id SERIAL PRIMARY KEY,
    appointment_id INTEGER NOT NULL UNIQUE REFERENCES appointment(appointment_id) ON DELETE CASCADE
);

-- 7. Drug Entity
CREATE TABLE drug (
    drug_id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    manufacturer VARCHAR(100) NOT NULL
);

-- 8. Prescription-Drug M:N Junction Table
CREATE TABLE prescription_drug (
    prescription_id INTEGER NOT NULL REFERENCES prescription(prescription_id) ON DELETE CASCADE,
    drug_id INTEGER NOT NULL REFERENCES drug(drug_id) ON DELETE RESTRICT,
    dosage VARCHAR(50) NOT NULL,
    duration VARCHAR(50) NOT NULL,
    PRIMARY KEY (prescription_id, drug_id)
);

-- 9. Bill Entity (1:1 with completed Appointment)
CREATE TABLE bill (
    bill_id SERIAL PRIMARY KEY,
    appointment_id INTEGER NOT NULL UNIQUE REFERENCES appointment(appointment_id) ON DELETE CASCADE,
    amount NUMERIC(10, 2) NOT NULL CHECK (amount >= 0),
    status VARCHAR(20) NOT NULL DEFAULT 'unpaid' CHECK (status IN ('paid', 'unpaid'))
);
