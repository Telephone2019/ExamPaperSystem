PRAGMA foreign_keys = ON;
PRAGMA encoding = 'UTF-8';
# 创建考试数据表
create table exam ( 
	start_ts INTEGER not null DEFAULT (strftime('%s', 'now')),
	start_dt_gmt0 TEXT not null DEFAULT (datetime('now')),
	start_dt_gmt8 TEXT not null DEFAULT (datetime('now', '+8 hours')),
	duration_s INTEGER not null,
	name TEXT not null DEFAULT '考试',
	id INTEGER,
	PRIMARY KEY(id)
);

# 创建试卷数据表
create table paper (
	file_path TEXT not null,
	mime_type TEXT not null,
	name TEXT not null DEFAULT '试卷',
	id INTEGER,
	PRIMARY KEY(id)
);

# 创建 “考试-试卷关系” 数据表
create table exam_paper (
	eid INTEGER references exam(id),
	pid INTEGER references paper(id),
	PRIMARY KEY(eid, pid)
) WITHOUT ROWID;

# 创建考试结果文件夹数据表
create table resdir (
	dir_path TEXT not null,
	id INTEGER,
	PRIMARY KEY(id)
);

# 创建 “座位号-考试结果文件夹关系” 数据表
create table pos_resdir (
	pos INTEGER,
	rid INTEGER references resdir(id),
	PRIMARY KEY(pos, rid)
) WITHOUT ROWID;