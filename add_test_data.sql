insert into exam (start_ts, duration_s, name)
values
(1622239200, 7200, 'SQL程序设计'),
(1622246400, 7200, 'ACM程序设计'),
(1622253600, 7200, 'C程序设计'),
(1622260800, 7200, 'C++程序设计'),
(1622268000, 7200, 'Java程序设计'),
(1622275200, 7200, 'Go程序设计'),
(1622282400, 7200, 'Python程序设计'),
(1622289600, 7200, 'C#程序设计')
;
select * from exam;
delete from exam where id=1;
insert into exam (start_ts, duration_s, name) values
(1622296800, 7200, 'Rust程序设计')
;
select * from exam;

insert into paper (file_path, mime_type, name)
values
('a/b/c', 'text', 'papre1'),
('a/b/d', 'jpg', 'papre2'),
('a/b/e', 'pdf', 'papre3'),
('a/b/f', 'pdf', 'papre4'),
('a/b/g', 'pdf', 'papre5'),
('a/b/h', 'pdf', 'papre6'),
('a/b/i', 'pdf', 'papre7')
;
select * from paper;

insert into pos_exam (pos, eid)
values
(2, 3),
(2, 4),
(2, 6),
(3, 3),
(3, 4),
(3, 6),
(4, 3),
(4, 4),
(4, 6)
;
select * from pos_exam;

insert into exam_paper (eid, pid)
values
(3, 2),
(6, 3)
;
select * from exam_paper;