-- Lab 05: Database Design from Requirements (MediTrack)
-- data.sql: Sample Data Population

-- 1. Clinics (≥ 2 clinics)
INSERT INTO clinic (clinic_id, name, address, phone) VALUES
(1, 'MediTrack City Center', '123 Main St, Sector 5, City', '+91-9876543210'),
(2, 'MediTrack Westend Clinic', '45 Park Avenue, West District', '+91-9876543211'),
(3, 'MediTrack Northside Care', '88 River Road, North Campus', '+91-9876543212');

-- 2. Doctors (≥ 4 doctors)
INSERT INTO doctor (doctor_id, name, specialization, clinic_id) VALUES
(1, 'Dr. Aris Thorne', 'Cardiology', 1),
(2, 'Dr. Beatriz Vance', 'Pediatrics', 1),
(3, 'Dr. Charles Xavier', 'Neurology', 2),
(4, 'Dr. Diana Prince', 'General Medicine', 2),
(5, 'Dr. Edward Nygma', 'Dermatology', 3);

-- 3. Patients (≥ 6 patients; Patient 1 has two phones; Patient 6 has no appointments)
INSERT INTO patient (patient_id, name, dob, blood_group) VALUES
(1, 'Aarav Sharma', '1990-05-15', 'O+'),
(2, 'Bhavna Patel', '1985-11-20', 'A+'),
(3, 'Chirag Gupta', '1998-03-10', 'B+'),
(4, 'Deepika Padukone', '1992-08-25', 'AB+'),
(5, 'Esha Deol', '2001-01-05', 'O-'),
(6, 'Farhan Akhtar', '1974-01-09', 'A-'); -- Patient with no appointments

-- Patient Phone Numbers (Patient 1 has 2 phones)
INSERT INTO patient_phone (patient_id, phone) VALUES
(1, '+91-9111111111'),
(1, '+91-9111111112'),
(2, '+91-9222222222'),
(3, '+91-9333333333'),
(4, '+91-9444444444'),
(5, '+91-9555555555'),
(6, '+91-9666666666');

-- 4. Appointments (≥ 8 appointments across clinics and statuses)
INSERT INTO appointment (appointment_id, patient_id, doctor_id, clinic_id, appointment_date, appointment_time, reason, status) VALUES
(1, 1, 1, 1, '2026-09-01', '09:00:00', 'Chest discomfort and routine checkup', 'completed'),
(2, 2, 1, 1, '2026-09-01', '10:30:00', 'Follow-up ECG consultation', 'completed'),
(3, 3, 2, 1, '2026-09-01', '11:00:00', 'Pediatric fever consultation', 'completed'),
(4, 4, 3, 2, '2026-09-02', '14:00:00', 'Migraine and dizziness', 'completed'),
(5, 5, 4, 2, '2026-09-02', '15:30:00', 'General flu symptoms', 'completed'),
(6, 1, 3, 2, '2026-09-03', '10:00:00', 'Neurological screening', 'completed'),
(7, 2, 2, 1, '2026-09-10', '09:30:00', 'Routine vaccination', 'booked'),
(8, 3, 4, 2, '2026-09-11', '11:30:00', 'Annual wellness check', 'cancelled');

-- 5. Drugs (≥ 5 drugs)
INSERT INTO drug (drug_id, name, manufacturer) VALUES
(1, 'Amoxicillin 500mg', 'Sun Pharma'),
(2, 'Atorvastatin 10mg', 'Cipla Labs'),
(3, 'Paracetamol 650mg', 'Mankind Pharma'),
(4, 'Sumatriptan 50mg', 'Dr. Reddys'),
(5, 'Metoprolol 25mg', 'Torrent Pharma');

-- 6. Prescriptions (for completed appointments 1, 2, 3, 4, 5, 6)
INSERT INTO prescription (prescription_id, appointment_id) VALUES
(101, 1),
(102, 2),
(103, 3),
(104, 4),
(105, 5),
(106, 6);

-- Prescription-Drug Entries (M:N with dosage and duration)
INSERT INTO prescription_drug (prescription_id, drug_id, dosage, duration) VALUES
(101, 2, '1 tablet daily at night', '30 days'),
(101, 5, '1 tablet twice daily', '15 days'),
(102, 2, '1 tablet daily at night', '60 days'),
(103, 1, '1 capsule thrice daily after meals', '5 days'),
(103, 3, '1 tablet as needed for fever', '3 days'),
(104, 4, '1 tablet at onset of migraine', '10 days'),
(105, 3, '1 tablet every 6 hours', '5 days'),
(106, 4, '1 tablet as needed', '5 days'),
(106, 2, '1 tablet daily', '30 days');

-- 7. Bills (for completed appointments 1, 2, 3, 4, 5, 6; mix of paid/unpaid)
INSERT INTO bill (bill_id, appointment_id, amount, status) VALUES
(501, 1, 1500.00, 'paid'),
(502, 2, 1200.00, 'paid'),
(503, 3, 800.00, 'unpaid'),
(504, 4, 2500.00, 'paid'),
(505, 5, 600.00, 'unpaid'),
(506, 6, 2000.00, 'paid');
