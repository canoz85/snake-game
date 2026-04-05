#include "ScoreDB.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

ScoreDB::~ScoreDB()
{
    if (!m_connName.isEmpty()) {
        QSqlDatabase::database(m_connName).close();
        QSqlDatabase::removeDatabase(m_connName);
    }
}

bool ScoreDB::open(const QString &dbPath)
{
    m_connName = QStringLiteral("snake_scores");
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connName);
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qWarning() << "ScoreDB: cannot open database:" << db.lastError().text();
        m_connName.clear();
        return false;
    }

    return ensureTable();
}

bool ScoreDB::ensureTable()
{
    QSqlQuery q(QSqlDatabase::database(m_connName));
    const bool ok = q.exec(
        "CREATE TABLE IF NOT EXISTS scores ("
        "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username   TEXT    NOT NULL,"
        "  score      INTEGER NOT NULL,"
        "  created_at TEXT    NOT NULL"
        ")"
    );
    if (!ok)
        qWarning() << "ScoreDB: cannot create table:" << q.lastError().text();
    return ok;
}

bool ScoreDB::save(const QString &name, int score)
{
    if (m_connName.isEmpty())
        return false;

    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("INSERT INTO scores (username, score, created_at) "
              "VALUES (:name, :score, :ts)");
    q.bindValue(":name",  name);
    q.bindValue(":score", score);
    q.bindValue(":ts",    QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));

    if (!q.exec()) {
        qWarning() << "ScoreDB: insert failed:" << q.lastError().text();
        return false;
    }
    return true;
}

int ScoreDB::getHighScore() const
{
    if (m_connName.isEmpty())
        return 0;

    QSqlQuery q(QSqlDatabase::database(m_connName));
    if (!q.exec("SELECT MAX(score) FROM scores") || !q.next())
        return 0;
    return q.value(0).isNull() ? 0 : q.value(0).toInt();
}

QVector<ScoreEntry> ScoreDB::getTopScores(int limit) const
{
    QVector<ScoreEntry> result;
    if (m_connName.isEmpty())
        return result;

    QSqlQuery q(QSqlDatabase::database(m_connName));
    q.prepare("SELECT username, score, created_at "
              "FROM scores ORDER BY score DESC, id DESC LIMIT :lim");
    q.bindValue(":lim", limit);

    if (!q.exec()) {
        qWarning() << "ScoreDB: query failed:" << q.lastError().text();
        return result;
    }

    while (q.next()) {
        ScoreEntry e;
        e.name  = q.value(0).toString();
        e.score = q.value(1).toInt();
        e.date  = q.value(2).toString();
        result.append(e);
    }
    return result;
}
