PRAGMA foreign_keys = ON;
PRAGMA encoding = 'UTF-8';
# 创建考试数据表
create table exam (
	start_ts INTEGER not null DEFAULT (strftime('%s', 'now')),
	start_dt_gmt0 TEXT,
	start_dt_gmt8 TEXT,
	duration_s INTEGER not null,
	name TEXT not null DEFAULT '考试',
	id INTEGER PRIMARY KEY AUTOINCREMENT
);
DROP TRIGGER IF EXISTS gmt0u;
DROP TRIGGER IF EXISTS gmt0i;
DROP TRIGGER IF EXISTS gmt8u;
DROP TRIGGER IF EXISTS gmt8i;
CREATE TRIGGER gmt0u AFTER UPDATE OF start_ts ON exam
BEGIN
    UPDATE exam SET start_dt_gmt0 = datetime(new.start_ts, 'unixepoch') WHERE id=new.id;
END;
CREATE TRIGGER gmt8u AFTER UPDATE OF start_ts ON exam
BEGIN
    UPDATE exam SET start_dt_gmt8 = datetime(new.start_ts, 'unixepoch', '+8 hours') WHERE id=new.id;
END;
CREATE TRIGGER gmt0i AFTER INSERT ON exam
BEGIN
    UPDATE exam SET start_dt_gmt0 = datetime(new.start_ts, 'unixepoch') WHERE id=new.id;
END;
CREATE TRIGGER gmt8i AFTER INSERT ON exam
BEGIN
    UPDATE exam SET start_dt_gmt8 = datetime(new.start_ts, 'unixepoch', '+8 hours') WHERE id=new.id;
END;

# 创建试卷数据表
create table paper (
	file_path TEXT not null,
	mime_type TEXT not null,
	name TEXT not null DEFAULT '试卷',
	id INTEGER PRIMARY KEY AUTOINCREMENT
);

# 创建 “考试-试卷关系” 数据表 [试卷1 : 考试n]
create table exam_paper (
	eid INTEGER references exam(id),
	pid INTEGER references paper(id),
	PRIMARY KEY(eid)
) WITHOUT ROWID;

# 创建 “座位号-考试关系” 数据表 [座位号n : 考试n]
create table pos_exam (
	pos INTEGER,
	eid INTEGER references exam(id),
	PRIMARY KEY(pos, eid)
) WITHOUT ROWID;

# 创建考试结果文件夹数据表
create table resdir (
	dir_path TEXT not null,
	id INTEGER PRIMARY KEY AUTOINCREMENT
);

# 创建 “座位号-考试结果文件夹关系” 数据表 [座位号n : 考试结果文件夹n]
create table pos_resdir (
	pos INTEGER,
	rid INTEGER references resdir(id),
	PRIMARY KEY(pos, rid)
) WITHOUT ROWID;