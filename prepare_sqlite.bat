@echo off
chcp 65001
IF NOT EXIST "ExamPaperSystem.db" (
	echo .read prepare_sqlite.sql | sqlite3 ExamPaperSystem.db
) ELSE (
	echo Database already exists!
	pause
)