CREATE TABLE topics(title TEXT NOT NULL);
INSERT INTO topics(rowid, title) VALUES(0, 'Tasks');

CREATE TABLE tasks(
    description TEXT NOT NULL,
    completed INTEGER DEFAULT 0 CHECK(completed >= 0 AND completed <= 1),
    topic NOT NULL DEFAULT 0 REFERENCES topics(rowid)
);
