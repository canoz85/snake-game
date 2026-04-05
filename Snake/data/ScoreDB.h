#pragma once

#include <QString>
#include <QVector>

struct ScoreEntry {
    QString name;
    int     score = 0;
    QString date;
};

// Thin abstraction over a local SQLite database that stores high scores.
// All SQL headers are kept out of this file intentionally.
class ScoreDB
{
public:
    ~ScoreDB();

    // Open (or create) the database at the given file path.
    // Returns false and logs a warning on failure; the object remains usable
    // (all other methods become no-ops).
    bool open(const QString &dbPath);

    // Persist a new entry. name should already be trimmed by the caller.
    bool save(const QString &name, int score);

    // Returns the highest score stored, or 0 if the table is empty / DB not open.
    int getHighScore() const;

    // Returns up to `limit` entries ordered by score descending.
    QVector<ScoreEntry> getTopScores(int limit = 5) const;

private:
    bool ensureTable();

    QString m_connName;   // named connection so we don't touch the default one
};
