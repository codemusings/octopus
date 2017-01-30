CREATE TABLE tasks(
    description TEXT,
    completed INTEGER DEFAULT 0 CHECK(completed >= 0 AND completed <= 1)
);
