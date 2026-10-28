#ifndef MATCHMINT_DATABASE_H
#define MATCHMINT_DATABASE_H

#include <stdbool.h>
#include <sqlite3.h>

/**
 * Initialize SQLite3 database connection.
 * Enables foreign keys and WAL mode.
 * Automatically executes schema_path if tables are missing.
 *
 * @param db_path Filepath to SQLite database (e.g. "matchmint.db" or ":memory:")
 * @param schema_path Optional path to schema.sql for auto-initialization
 * @return 0 on success, non-zero on error.
 */
int db_init(const char *db_path, const char *schema_path);

/**
 * Close database connection safely.
 */
void db_close(void);

/**
 * Returns the active SQLite3 handle.
 */
sqlite3 *db_get_handle(void);

/**
 * Executes a SQL script file from disk.
 *
 * @param filepath Path to .sql file
 * @return 0 on success, non-zero on error.
 */
int db_execute_file(const char *filepath);

/**
 * Transaction helpers.
 */
int db_begin_transaction(void);
int db_commit_transaction(void);
int db_rollback_transaction(void);

/**
 * Resets database to sample collegiate seed data.
 *
 * @param schema_path Path to schema.sql
 * @param seed_path Path to seed.sql
 * @return 0 on success, non-zero on error.
 */
int db_reset_to_seed(const char *schema_path, const char *seed_path);

/**
 * Check if a table exists.
 *
 * @param table_name Name of the table
 * @return 1 if exists, 0 if not, -1 on error.
 */
int db_table_exists(const char *table_name);

/**
 * Returns total row count in a table.
 */
int db_get_row_count(const char *table_name);

#endif /* MATCHMINT_DATABASE_H */
