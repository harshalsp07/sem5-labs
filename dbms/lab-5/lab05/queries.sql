-- Lab 05: Database Design from Requirements (MediTrack)
-- queries.sql: Verification Queries

-- Query 1: All appointments at a given clinic on a given date, with patient and doctor names.
-- Description: Retrieves appointment schedule for Clinic ID 1 on 2026-09-01.
SELECT 
    a.appointment_id,
    a.appointment_time,
    p.name AS patient_name,
    d.name AS doctor_name,
    a.reason,
    a.status
FROM appointment a
JOIN patient p ON a.patient_id = p.patient_id
JOIN doctor d ON a.doctor_id = d.doctor_id
WHERE a.clinic_id = 1 
  AND a.appointment_date = '2026-09-01'
ORDER BY a.appointment_time;


-- Query 2: The busiest doctor (most completed appointments).
-- Description: Finds the doctor with the highest count of completed appointments.
SELECT 
    d.doctor_id,
    d.name AS doctor_name,
    d.specialization,
    c.name AS clinic_name,
    COUNT(a.appointment_id) AS completed_appointments
FROM doctor d
JOIN appointment a ON d.doctor_id = a.doctor_id
JOIN clinic c ON d.clinic_id = c.clinic_id
WHERE a.status = 'completed'
GROUP BY d.doctor_id, d.name, d.specialization, c.name
ORDER BY completed_appointments DESC
LIMIT 1;


-- Query 3: The top 3 most-prescribed drugs (by number of prescriptions they appear on).
-- Description: Ranks drugs based on how many distinct prescriptions include them.
SELECT 
    dr.drug_id,
    dr.name AS drug_name,
    dr.manufacturer,
    COUNT(pd.prescription_id) AS times_prescribed
FROM drug dr
JOIN prescription_drug pd ON dr.drug_id = pd.drug_id
GROUP BY dr.drug_id, dr.name, dr.manufacturer
ORDER BY times_prescribed DESC, dr.name ASC
LIMIT 3;


-- Query 4: Revenue per clinic from paid bills (include clinics with 0 — outer join).
-- Description: Computes total paid revenue for every clinic, using LEFT JOIN so clinics without paid bills show 0.
SELECT 
    c.clinic_id,
    c.name AS clinic_name,
    COALESCE(SUM(b.amount), 0.00) AS total_paid_revenue
FROM clinic c
LEFT JOIN appointment a ON c.clinic_id = a.clinic_id
LEFT JOIN bill b ON a.appointment_id = b.appointment_id AND b.status = 'paid'
GROUP BY c.clinic_id, c.name
ORDER BY total_paid_revenue DESC;


-- Query 5: Patients who have never had a completed appointment (NOT EXISTS).
-- Description: Uses NOT EXISTS subquery to find registered patients without any completed appointments.
SELECT 
    p.patient_id,
    p.name AS patient_name,
    p.dob,
    p.blood_group
FROM patient p
WHERE NOT EXISTS (
    SELECT 1 
    FROM appointment a 
    WHERE a.patient_id = p.patient_id 
      AND a.status = 'completed'
);


-- Query 6: For each patient, the list of drugs ever prescribed to them.
-- Description: Joins across patient -> appointment -> prescription -> prescription_drug -> drug.
SELECT DISTINCT
    p.patient_id,
    p.name AS patient_name,
    dr.drug_id,
    dr.name AS drug_name,
    dr.manufacturer
FROM patient p
JOIN appointment a ON p.patient_id = a.patient_id
JOIN prescription pr ON a.appointment_id = pr.appointment_id
JOIN prescription_drug pd ON pr.prescription_id = pd.prescription_id
JOIN drug dr ON pd.drug_id = dr.drug_id
ORDER BY p.patient_id, dr.name;
