-- ============================================================================
-- university.sql  --  CSL303 shared teaching dataset (the "University" schema)
-- ----------------------------------------------------------------------------
-- A compact version of the classic Silberschatz "University" database, used
-- across the lectures and Labs 1-3. Written in portable SQL that loads on BOTH
-- SQLite 3 and PostgreSQL 16.
--
--   Load in SQLite:     sqlite3 university.db < university.sql
--   Load in Postgres:   psql -d university -f university.sql
--
-- Portability notes:
--   * Types used (INTEGER / TEXT / NUMERIC) exist in both engines.
--   * No AUTOINCREMENT/SERIAL: every id is inserted explicitly.
--   * Foreign keys are declared. SQLite only enforces them if you first run
--     `PRAGMA foreign_keys = ON;` (Lab 1 covers this).
-- ============================================================================

-- Drop in reverse-dependency order so re-loading is clean.
DROP TABLE IF EXISTS takes;
DROP TABLE IF EXISTS teaches;
DROP TABLE IF EXISTS prereq;
DROP TABLE IF EXISTS section;
DROP TABLE IF EXISTS advisor;
DROP TABLE IF EXISTS student;
DROP TABLE IF EXISTS instructor;
DROP TABLE IF EXISTS course;
DROP TABLE IF EXISTS classroom;
DROP TABLE IF EXISTS department;

-- ---------------------------------------------------------------------------
-- Schema
-- ---------------------------------------------------------------------------
CREATE TABLE department (
    dept_name  TEXT     PRIMARY KEY,
    building   TEXT,
    budget     NUMERIC  CHECK (budget > 0)
);

CREATE TABLE classroom (
    building   TEXT,
    room_no    TEXT,
    capacity   INTEGER  CHECK (capacity >= 0),
    PRIMARY KEY (building, room_no)
);

CREATE TABLE course (
    course_id  TEXT     PRIMARY KEY,
    title      TEXT     NOT NULL,
    dept_name  TEXT     REFERENCES department (dept_name),
    credits    INTEGER  CHECK (credits > 0)
);

CREATE TABLE instructor (
    id         TEXT     PRIMARY KEY,
    name       TEXT     NOT NULL,
    dept_name  TEXT     REFERENCES department (dept_name),
    salary     NUMERIC  CHECK (salary > 0)
);

CREATE TABLE student (
    id         TEXT     PRIMARY KEY,
    name       TEXT     NOT NULL,
    dept_name  TEXT     REFERENCES department (dept_name),
    tot_cred   INTEGER  CHECK (tot_cred >= 0)
);

CREATE TABLE advisor (
    s_id  TEXT  PRIMARY KEY REFERENCES student (id),
    i_id  TEXT  REFERENCES instructor (id)
);

CREATE TABLE section (
    course_id   TEXT,
    sec_id      TEXT,
    semester    TEXT     CHECK (semester IN ('Spring','Summer','Fall','Winter')),
    year        INTEGER  CHECK (year > 1900 AND year < 2100),
    building    TEXT,
    room_no     TEXT,
    PRIMARY KEY (course_id, sec_id, semester, year),
    FOREIGN KEY (course_id) REFERENCES course (course_id),
    FOREIGN KEY (building, room_no) REFERENCES classroom (building, room_no)
);

CREATE TABLE prereq (
    course_id   TEXT REFERENCES course (course_id),
    prereq_id   TEXT REFERENCES course (course_id),
    PRIMARY KEY (course_id, prereq_id)
);

CREATE TABLE teaches (
    id          TEXT,
    course_id   TEXT,
    sec_id      TEXT,
    semester    TEXT,
    year        INTEGER,
    PRIMARY KEY (id, course_id, sec_id, semester, year),
    FOREIGN KEY (id) REFERENCES instructor (id),
    FOREIGN KEY (course_id, sec_id, semester, year)
        REFERENCES section (course_id, sec_id, semester, year)
);

CREATE TABLE takes (
    id          TEXT,
    course_id   TEXT,
    sec_id      TEXT,
    semester    TEXT,
    year        INTEGER,
    grade       TEXT,       -- NULL = currently enrolled, not yet graded
    PRIMARY KEY (id, course_id, sec_id, semester, year),
    FOREIGN KEY (id) REFERENCES student (id),
    FOREIGN KEY (course_id, sec_id, semester, year)
        REFERENCES section (course_id, sec_id, semester, year)
);

-- ---------------------------------------------------------------------------
-- Data
-- ---------------------------------------------------------------------------
INSERT INTO department (dept_name, building, budget) VALUES
    ('Comp. Sci.', 'Taylor',   1100000),
    ('Physics',    'Watson',    700000),
    ('Elec. Eng.', 'Taylor',    850000),
    ('Mathematics','Watson',    600000),
    ('History',    'Painter',   500000),
    ('Finance',    'Painter',   1200000),
    ('Biology',    'Watson',    900000);

INSERT INTO classroom (building, room_no, capacity) VALUES
    ('Taylor',  '3128', 70),
    ('Taylor',  '3129', 40),
    ('Watson',  '100',  30),
    ('Watson',  '120',  50),
    ('Watson',  '183',  60),
    ('Painter', '514',  20),
    ('Painter', '515',  25);

INSERT INTO course (course_id, title, dept_name, credits) VALUES
    ('CS-101', 'Intro. to Computer Science', 'Comp. Sci.', 4),
    ('CS-190', 'Game Design',                'Comp. Sci.', 4),
    ('CS-315', 'Robotics',                   'Comp. Sci.', 3),
    ('CS-319', 'Image Processing',           'Comp. Sci.', 3),
    ('CS-347', 'Database System Concepts',   'Comp. Sci.', 3),
    ('PHY-101','Physical Principles',        'Physics',    4),
    ('EE-181', 'Intro. to Digital Systems',  'Elec. Eng.', 3),
    ('MU-199', 'Music Video Production',      'History',    3),
    ('FIN-201','Investment Banking',         'Finance',    3),
    ('MATH-101','Calculus I',                'Mathematics',4),
    ('BIO-101','Intro. to Biology',          'Biology',    4),
    ('BIO-301','Genetics',                   'Biology',    4),
    ('HIS-351','World History',              'History',    3);

INSERT INTO instructor (id, name, dept_name, salary) VALUES
    ('10101', 'Srinivasan', 'Comp. Sci.',  95000),
    ('12121', 'Wu',         'Finance',      90000),
    ('15151', 'Mozart',     'History',      40000),
    ('22222', 'Einstein',   'Physics',      95000),
    ('32343', 'El Said',    'History',      60000),
    ('33456', 'Gold',       'Physics',      87000),
    ('45565', 'Katz',       'Comp. Sci.',   75000),
    ('58583', 'Califieri',  'History',      62000),
    ('76543', 'Singh',      'Finance',      80000),
    ('76766', 'Crick',      'Biology',      72000),
    ('83821', 'Brandt',     'Comp. Sci.',   92000),
    ('98345', 'Kim',        'Elec. Eng.',   80000);

INSERT INTO student (id, name, dept_name, tot_cred) VALUES
    ('00128', 'Zhang',    'Comp. Sci.',  102),
    ('12345', 'Shankar',  'Comp. Sci.',   32),
    ('19991', 'Brandt',   'History',      80),
    ('23121', 'Chavez',   'Finance',     110),
    ('44553', 'Peltier',  'Physics',      56),
    ('45678', 'Levy',     'Physics',      46),
    ('54321', 'Williams', 'Comp. Sci.',   54),
    ('55739', 'Sanchez',  'Mathematics',  38),
    ('70557', 'Snow',     'Physics',       0),
    ('76543', 'Brown',    'Comp. Sci.',   58),
    ('76653', 'Aoi',      'Elec. Eng.',   60),
    ('98765', 'Bourikas', 'Elec. Eng.',   98),
    ('98988', 'Tanaka',   'Biology',     120),
    ('34567', 'Martin',   'Biology',      24),
    ('41000', 'Okafor',   'Comp. Sci.',    8);

INSERT INTO advisor (s_id, i_id) VALUES
    ('00128', '45565'),
    ('12345', '10101'),
    ('23121', '76543'),
    ('44553', '22222'),
    ('45678', '22222'),
    ('54321', '10101'),
    ('55739', '22222'),
    ('76543', '45565'),
    ('76653', '98345'),
    ('98765', '98345'),
    ('98988', '76766'),
    ('34567', '76766'),
    ('41000', '83821');

INSERT INTO section (course_id, sec_id, semester, year, building, room_no) VALUES
    ('CS-101', '1', 'Fall',   2021, 'Taylor',  '3128'),
    ('CS-101', '1', 'Spring', 2022, 'Taylor',  '3128'),
    ('CS-190', '1', 'Spring', 2022, 'Taylor',  '3128'),
    ('CS-190', '2', 'Spring', 2022, 'Taylor',  '3128'),
    ('CS-315', '1', 'Spring', 2022, 'Watson',  '120'),
    ('CS-319', '1', 'Spring', 2022, 'Watson',  '100'),
    ('CS-319', '2', 'Spring', 2022, 'Taylor',  '3128'),
    ('CS-347', '1', 'Fall',   2021, 'Taylor',  '3128'),
    ('FIN-201','1', 'Spring', 2022, 'Painter', '514'),
    ('HIS-351','1', 'Spring', 2022, 'Painter', '514'),
    ('MU-199', '1', 'Spring', 2022, 'Painter', '514'),
    ('PHY-101','1', 'Fall',   2021, 'Watson',  '100'),
    ('EE-181', '1', 'Spring', 2022, 'Taylor',  '3129'),
    ('BIO-101','1', 'Summer', 2021, 'Watson',  '183'),
    ('BIO-301','1', 'Summer', 2022, 'Watson',  '183'),
    ('MATH-101','1','Fall',   2021, 'Watson',  '120');

INSERT INTO prereq (course_id, prereq_id) VALUES
    ('CS-190', 'CS-101'),
    ('CS-315', 'CS-101'),
    ('CS-319', 'CS-101'),
    ('CS-347', 'CS-101'),
    ('BIO-301','BIO-101');

INSERT INTO teaches (id, course_id, sec_id, semester, year) VALUES
    ('10101', 'CS-101', '1', 'Fall',   2021),
    ('10101', 'CS-347', '1', 'Fall',   2021),
    ('10101', 'CS-315', '1', 'Spring', 2022),
    ('83821', 'CS-190', '1', 'Spring', 2022),
    ('83821', 'CS-190', '2', 'Spring', 2022),
    ('45565', 'CS-101', '1', 'Spring', 2022),
    ('45565', 'CS-319', '1', 'Spring', 2022),
    ('98345', 'EE-181', '1', 'Spring', 2022),
    ('76766', 'BIO-101','1', 'Summer', 2021),
    ('76766', 'BIO-301','1', 'Summer', 2022),
    ('22222', 'PHY-101','1', 'Fall',   2021),
    ('32343', 'HIS-351','1', 'Spring', 2022),
    ('15151', 'MU-199', '1', 'Spring', 2022),
    ('12121', 'FIN-201','1', 'Spring', 2022);
    -- Note: CS-319 section 2 has NO instructor in teaches (used for outer-join examples).

INSERT INTO takes (id, course_id, sec_id, semester, year, grade) VALUES
    ('00128', 'CS-101', '1', 'Fall',   2021, 'A'),
    ('00128', 'CS-347', '1', 'Fall',   2021, 'A-'),
    ('12345', 'CS-101', '1', 'Fall',   2021, 'C'),
    ('12345', 'CS-190', '2', 'Spring', 2022, 'A'),
    ('12345', 'CS-315', '1', 'Spring', 2022, 'A'),
    ('12345', 'CS-347', '1', 'Fall',   2021, 'A'),
    ('54321', 'CS-101', '1', 'Fall',   2021, 'A-'),
    ('54321', 'CS-190', '2', 'Spring', 2022, 'B+'),
    ('76543', 'CS-101', '1', 'Fall',   2021, 'A'),
    ('76543', 'CS-319', '2', 'Spring', 2022, 'B'),
    ('23121', 'FIN-201','1', 'Spring', 2022, 'C+'),
    ('44553', 'PHY-101','1', 'Fall',   2021, 'B-'),
    ('45678', 'CS-101', '1', 'Fall',   2021, 'F'),
    ('45678', 'CS-319', '1', 'Spring', 2022, 'B+'),
    ('45678', 'PHY-101','1', 'Fall',   2021, 'B+'),
    ('98765', 'CS-101', '1', 'Fall',   2021, 'C-'),
    ('98765', 'CS-315', '1', 'Spring', 2022, 'B'),
    ('98988', 'BIO-101','1', 'Summer', 2021, 'A'),
    ('98988', 'BIO-301','1', 'Summer', 2022, NULL),  -- enrolled, not graded
    ('34567', 'BIO-101','1', 'Summer', 2021, 'B'),
    ('19991', 'HIS-351','1', 'Spring', 2022, 'B'),
    ('55739', 'MU-199', '1', 'Spring', 2022, 'A-'),
    ('76653', 'EE-181', '1', 'Spring', 2022, 'C'),
    ('00128', 'CS-315', '1', 'Spring', 2022, NULL);  -- enrolled, not graded

-- End of university.sql
