-- this enable for the sql to follow the foreign key constraints. 
PRAGMA foreign_keys = ON;
-- Part 1 — Single-table queries
-- 1
SELECT * FROM course
WHERE dept_name = 'Comp. Sci.' ;

-- 2
SELECT name, ROUND( salary / 12.0 ,2) AS monthly_pay FROM instructor
WHERE salary > 70000 AND salary < 90000
ORDER BY monthly_pay DESC;

-- 3
SELECT DISTINCT semester FROM section;

-- 4
SELECT id,name,tot_cred FROM student
ORDER BY tot_cred DESC, name ASC
LIMIT 3;

-- 5
SELECT course_id, title 
FROM course
WHERE (title LIKE '%System%' OR title LIKE '%sys-tem%')
  AND title NOT LIKE '%Intro%';

-- 6
SELECT * FROM instructor
WHERE dept_name NOT IN ('History','Music')
ORDER BY dept_name , name;

-- 7
SELECT COUNT(*) AS number_of_instructor FROM instructor;

-- 8
SELECT name, salary,
    CASE 
        WHEN salary >= 90000 THEN 'High'
        WHEN salary BETWEEN 70000 AND 89999 THEN 'Medium'
        ELSE 'Low'
    END AS salary_category
FROM 
    instructor;


-- 9
SELECT * FROM student 
WHERE tot_cred%10 = 0;

--- PART 2 - INNER JOIN

-- 10
SELECT student.name, department.building
FROM student
JOIN department ON student.dept_name = department.dept_name;

-- 11
SELECT 
    i.name AS instructor_name, 
    c.title AS course_title
FROM 
    instructor i
JOIN 
    teaches t ON i.ID = t.ID
JOIN 
    section s ON t.course_id = s.course_id 
              AND t.sec_id = s.sec_id 
              AND t.semester = s.semester 
              AND t.year = s.year
JOIN 
    course c ON s.course_id = c.course_id;

-- 12
SELECT 
    s.name AS student_name, 
    c.title AS course_title,
    t.grade
FROM 
    student s
JOIN 
    takes t ON s.id = t.id
JOIN 
    section sec ON t.course_id = sec.course_id 
                AND t.sec_id = sec.sec_id 
                AND t.semester = sec.semester 
                AND t.year = sec.year
JOIN 
    course c ON t.course_id = c.course_id;

-- 13
SELECT s.name
FROM student s
JOIN advisor a ON s.id = a.s_id
JOIN instructor i ON a.i_id = i.id
WHERE i.name = 'Katz';

-- 14
SELECT c.title, cl.capacity
FROM section sec
JOIN course c ON sec.course_id = c.course_id
JOIN classroom cl ON sec.building = cl.building 
                  AND sec.room_no = cl.room_no;

-- 15
SELECT DISTINCT dept.building
FROM section sec
JOIN classroom cl ON sec.building = cl.building 
                  AND sec.room_no = cl.room_no
JOIN course c ON sec.course_id = c.course_id
JOIN department dept ON c.dept_name = dept.dept_name;

-- 16
SELECT DISTINCT i.name
FROM instructor i
JOIN teaches t ON i.id = t.id
JOIN section sec ON t.course_id = sec.course_id 
                 AND t.sec_id = sec.sec_id 
                 AND t.semester = sec.semester 
                 AND t.year = sec.year
WHERE sec.building = 'Taylor';

--- PART 3 - OUTER JOINS & ANTI-JOINS

-- 17
-- Left join section with teaches to show all sections and their instructors (if any)
-- Sections with no instructor will have NULL in the teaches columns
SELECT sec.course_id, sec.sec_id, sec.semester, sec.year, t.id
FROM section sec
LEFT JOIN teaches t ON sec.course_id = t.course_id 
                    AND sec.sec_id = t.sec_id 
                    AND sec.semester = t.semester 
                    AND sec.year = t.year;

-- 18
-- Anti-join: sections that have NO instructor
-- Note: the WHERE clause must check t.id IS NULL (not a teaches column) to avoid 
-- silently converting the LEFT JOIN back to INNER JOIN
SELECT sec.course_id, sec.sec_id
FROM section sec
LEFT JOIN teaches t ON sec.course_id = t.course_id 
                    AND sec.sec_id = t.sec_id 
                    AND sec.semester = t.semester 
                    AND sec.year = t.year
WHERE t.id IS NULL;

-- 19
-- All departments and count of students, showing 0 for departments with no students
SELECT d.dept_name, COUNT(s.id) AS num_students
FROM department d
LEFT JOIN student s ON d.dept_name = s.dept_name
GROUP BY d.dept_name;

-- 20
-- Instructors who teach nothing (LEFT JOIN ... IS NULL anti-join pattern)
SELECT i.id, i.name
FROM instructor i
LEFT JOIN teaches t ON i.id = t.id
WHERE t.id IS NULL;

-- 21
-- Every course title and the name of an instructor who taught it 
-- Including courses taught by no one (chain of LEFT JOINs)
SELECT DISTINCT c.title, i.name
FROM course c
LEFT JOIN section sec ON c.course_id = sec.course_id
LEFT JOIN teaches t ON sec.course_id = t.course_id 
                    AND sec.sec_id = t.sec_id 
                    AND sec.semester = t.semester 
                    AND sec.year = t.year
LEFT JOIN instructor i ON t.id = i.id;

--- PART 4 - SELF JOINS & GOTCHAS

-- 22
-- All pairs of instructors in the same department, no self-pairs, no mirror duplicates (a.id < b.id)
SELECT a.name, b.name, a.dept_name
FROM instructor a
JOIN instructor b ON a.dept_name = b.dept_name 
                  AND a.id < b.id;

-- 23
-- Each course title with its prerequisite's title (self-join on course via prereq)
SELECT c1.title AS course_title, c2.title AS prerequisite_title
FROM course c1
JOIN prereq p ON c1.course_id = p.course_id
JOIN course c2 ON p.prereq_id = c2.course_id;

-- 24
-- Without a JOIN condition (WHERE or ON), this produces a Cartesian product.
SELECT COUNT(*) AS cross_product_count
FROM student, department;

-- 25
-- INNER vs LEFT counts on section × teaches
-- INNER JOIN: only sections that have at least one assigned instructor
-- LEFT JOIN: all sections, regardless of whether they have an instructor
SELECT 'INNER JOIN' AS join_type, COUNT(*) AS row_count
FROM section s
INNER JOIN teaches t ON s.course_id = t.course_id 
                     AND s.sec_id = t.sec_id 
                     AND s.semester = t.semester 
                     AND s.year = t.year
UNION ALL
SELECT 'LEFT JOIN' AS join_type, COUNT(*) AS row_count
FROM section s
LEFT JOIN teaches t ON s.course_id = t.course_id 
                    AND s.sec_id = t.sec_id 
                    AND s.semester = t.semester 
                    AND s.year = t.year;
-- Explanation: LEFT JOIN returns more rows because it includes all sections,
-- while INNER JOIN excludes sections with no assigned instructor.