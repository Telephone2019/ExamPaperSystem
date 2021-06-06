@echo off
chcp 65001
IF NOT EXIST "ExamPaperSystem.db" (
	prepare_sqlite.bat
	echo .read add_test_data.sql | sqlite3.exe ExamPaperSystem.db
) ELSE (
	echo Database already exists!
	pause
)