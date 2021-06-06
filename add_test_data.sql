PRAGMA foreign_keys = ON;
PRAGMA encoding = 'UTF-8';
insert into exam (start_ts, duration_s, name)
values
(strftime('%s', 'now')+7200*(-4), 7200, 'SQL程序设计'),
(strftime('%s', 'now')+7200*(-3), 7200, 'ACM程序设计'),
(strftime('%s', 'now')+7200*(-2), 7200, 'C程序设计'),
(strftime('%s', 'now')+7200*(-1), 7200, 'C++程序设计'),
(strftime('%s', 'now')+7200*0, 7200, 'Java程序设计'),
(strftime('%s', 'now')+7200*1, 7200, 'Go程序设计'),
(strftime('%s', 'now')+7200*2, 7200, 'Python程序设计'),
(strftime('%s', 'now')+7200*3, 7200, 'C#程序设计')
;
select * from exam;
delete from exam where id=1;
insert into exam (start_ts, duration_s, name) values
(strftime('%s', 'now')+7200*4, 7200, 'Rust程序设计')
;
select * from exam;

insert into paper (file_path, mime_type, name)
values
('D:\p1.txt', 'text/plain', 'papre1.txt'),
('D:\p2.txt', 'text/plain', 'papre2.txt'),
('D:\p3.txt', 'text/plain', 'papre3.txt'),
('D:\p4.txt', 'text/plain', 'papre4.txt'),
('D:\p5.txt', 'text/plain', 'papre5.txt'),
('D:\p6.txt', 'text/plain', 'papre6.txt'),
('D:\p7.txt', 'text/plain', 'papre7.txt'),
('D:\p8.txt', 'text/plain', 'papre8.txt'),
('D:\p9.txt', 'text/plain', 'papre9.txt'),
('D:\p10.txt', 'text/plain', 'papre10.txt'),
('D:\p11.txt', 'text/plain', 'papre11.txt')
;
select * from paper;

insert into resdir (dir_path)
values
('D:\exam1'),
('D:\exam2'),
('D:\exam3'),
('D:\exam4'),
('D:\exam5'),
('D:\exam6'),
('D:\exam7'),
('D:\exam8'),
('D:\exam9'),
('D:\exam10'),
('D:\exam11')
;
select * from resdir;

insert into pos_exam (pos, eid, pid, rid, sub_res_dir_name)
values
(0, 2, 2, 2, '0'),
(0, 3, 3, 3, '0'),
(0, 4, 4, 4, '0'),
(0, 5, 5, 5, '0'),
(0, 6, 6, 6, '0'),
(0, 7, 7, 7, '0'),
(0, 8, 8, 8, '0'),
(0, 9, 9, 9, '0'),
(1, 2, 3, 2, '1'),
(1, 3, 4, 3, '1'),
(1, 4, 5, 4, '1'),
(1, 5, 6, 5, '1'),
(1, 6, 7, 6, '1'),
(1, 7, 8, 7, '1'),
(1, 8, 9, 8, '1'),
(1, 9, 10, 9, '1')
;
select * from pos_exam;