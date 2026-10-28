#include "database.h"
#include "common.h"

static sqlite3 *g_db = NULL;

sqlite3 *db_get_handle(void) {
    return g_db;
}

int db_execute_file(const char *filepath) {
    if (!g_db || !filepath) return -1;

    FILE *f = fopen(filepath, "rb");
    if (!f) {
        fprintf(stderr, "[DB ERROR] Failed to open SQL file: %s\n", filepath);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (length < 0) {
        fclose(f);
        return -1;
    }

    char *buffer = (char *)malloc((size_t)length + 1);
    if (!buffer) {
        fclose(f);
        fprintf(stderr, "[DB ERROR] Allocation failed reading: %s\n", filepath);
        return -1;
    }

    size_t read_bytes = fread(buffer, 1, (size_t)length, f);
    buffer[read_bytes] = '\0';
    fclose(f);

    char *errmsg = NULL;
    int rc = sqlite3_exec(g_db, buffer, NULL, NULL, &errmsg);
    free(buffer);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DB ERROR] Executing SQL script '%s': %s\n", filepath, errmsg ? errmsg : "unknown");
        if (errmsg) sqlite3_free(errmsg);
        return rc;
    }

    return 0;
}

int db_table_exists(const char *table_name) {
    if (!g_db || !table_name) return -1;

    const char *sql = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name=?;";
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;

    sqlite3_bind_text(stmt, 1, table_name, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    int exists = 0;
    if (rc == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return exists > 0 ? 1 : 0;
}

int db_get_row_count(const char *table_name) {
    if (!g_db || !table_name) return -1;

    char query[256];
    snprintf(query, sizeof(query), "SELECT count(*) FROM %s;", table_name);

    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(g_db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;

    rc = sqlite3_step(stmt);
    int count = -1;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

int db_begin_transaction(void) {
    if (!g_db) return -1;
    return sqlite3_exec(g_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}

int db_commit_transaction(void) {
    if (!g_db) return -1;
    return sqlite3_exec(g_db, "COMMIT;", NULL, NULL, NULL);
}

int db_rollback_transaction(void) {
    if (!g_db) return -1;
    return sqlite3_exec(g_db, "ROLLBACK;", NULL, NULL, NULL);
}

int db_init(const char *db_path, const char *schema_path) {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }

    int rc = sqlite3_open(db_path, &g_db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DB ERROR] Cannot open SQLite database '%s': %s\n", db_path, sqlite3_errmsg(g_db));
        if (g_db) {
            sqlite3_close(g_db);
            g_db = NULL;
        }
        return rc;
    }

    /* 1. Enable Foreign Key Constraints */
    rc = sqlite3_exec(g_db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DB ERROR] Failed to enable PRAGMA foreign_keys: %s\n", sqlite3_errmsg(g_db));
    }

    /* 2. Enable WAL mode for high concurrent read performance (if not in-memory) */
    if (strcmp(db_path, ":memory:") != 0) {
        rc = sqlite3_exec(g_db, "PRAGMA journal_mode = WAL;", NULL, NULL, NULL);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "[DB WARNING] Could not set WAL mode: %s\n", sqlite3_errmsg(g_db));
        }
    }

    /* 3. Auto-initialize schema if tables do not exist */
    if (schema_path && db_table_exists("competitions") == 0) {
        printf("[INFO] Initializing SQLite database schema from %s...\n", schema_path);
        rc = db_execute_file(schema_path);
        if (rc != 0) {
            fprintf(stderr, "[DB ERROR] Failed to apply database schema: %s\n", schema_path);
            return rc;
        }
        printf("[INFO] Database schema applied successfully.\n");
    }

    return 0;
}

int db_reset_to_seed(const char *schema_path, const char *seed_path) {
    if (!g_db || !schema_path || !seed_path) return -1;

    int rc = db_begin_transaction();
    if (rc != SQLITE_OK) return rc;

    rc = db_execute_file(schema_path);
    if (rc != SQLITE_OK) {
        db_rollback_transaction();
        return rc;
    }

    rc = db_execute_file(seed_path);
    if (rc != SQLITE_OK) {
        db_rollback_transaction();
        return rc;
    }

    return db_commit_transaction();
}

void db_close(void) {
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
}
