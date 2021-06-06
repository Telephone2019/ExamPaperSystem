#define db_file_name "ExamPaperSystem.db"

#include <string.h>

#include "sqlite3.h"
#include "vutils.h"

typedef sqlite3* Database;
typedef struct Paper {
	int valid;
	char* path;
	char* mime_type;
	char* dl_name;
} Paper;

volatile Database db = NULL;

// must be called from single thread environment!
void db_init() {
	while (sqlite3_open(db_file_name, &db) != SQLITE_OK);
}

// must be called from single thread environment!
void db_close() {
	if (db)
	{
		while (sqlite3_close(db) != SQLITE_OK);
		db = NULL;
	}
}

void check_db() {
	while (!db);
}

void db_deletePaper(Paper* paper) {
	free(paper->path); paper->path = NULL;
	free(paper->mime_type); paper->mime_type = NULL;
	free(paper->dl_name); paper->dl_name = NULL;
}

Paper db_get_paper(long long pos) {
	check_db();
	Paper paper = { .valid = 0 };
	sqlite3_stmt* sql_statement = NULL;
	int prepared_code = sqlite3_prepare(db,
		"select file_path, mime_type, name from"
		"(select pe.pid as current_pid from (select pid, eid from pos_exam where pos=@pos)pe,(select * from exam)e where pe.eid=e.id and e.start_ts<=cast(strftime('%s', 'now') as INTEGER) and e.start_ts+e.duration_s>=cast(strftime('%s', 'now') as INTEGER)),"
		"(select * from paper)"
		"where current_pid=id;"
		, -1, &sql_statement, NULL);
	const char *prepared_err_msg =sqlite3_errmsg(db);
	int pos_index = sqlite3_bind_parameter_index(sql_statement, "@pos");
	int bind_code = sqlite3_bind_int(sql_statement, pos_index, pos);
	const char* bind_err_msg = sqlite3_errmsg(db);
	int step_result;
	if ((step_result = sqlite3_step(sql_statement)) == SQLITE_ROW)
	{
		const char* file_path = sqlite3_column_text(sql_statement, 0);
		const char* mime_type = sqlite3_column_text(sql_statement, 1);
		const char* dl_name = sqlite3_column_text(sql_statement, 2);
		paper.path = zero_malloc(strlen(file_path) + 1);
		paper.mime_type = zero_malloc(strlen(mime_type) + 1);
		paper.dl_name = zero_malloc(strlen(dl_name) + 1);
		if (paper.path && paper.mime_type && paper.dl_name)
		{
			memcpy(paper.path, file_path, strlen(file_path));
			memcpy(paper.mime_type, mime_type, strlen(mime_type));
			memcpy(paper.dl_name, dl_name, strlen(dl_name));
			paper.valid = 1;
		}
	}
	const char* step_err_msg = sqlite3_errmsg(db);
	sqlite3_finalize(sql_statement);
	return paper;
}