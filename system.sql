--
-- PostgreSQL database dump
--

-- Dumped from database version 17.5
-- Dumped by pg_dump version 17.5

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: classroom; Type: TABLE; Schema: public; Owner: enrolluser
--

CREATE TABLE public.classroom (
    id character varying(20) NOT NULL,
    building character varying(50) NOT NULL,
    capacity integer NOT NULL,
    is_available boolean DEFAULT true
);


ALTER TABLE public.classroom OWNER TO enrolluser;

--
-- Name: classroomschedule; Type: TABLE; Schema: public; Owner: enrolluser
--

CREATE TABLE public.classroomschedule (
    classroom_id character varying(20) NOT NULL,
    course_id character varying(20) NOT NULL,
    time_slot character varying(50) NOT NULL
);


ALTER TABLE public.classroomschedule OWNER TO enrolluser;

--
-- Name: course; Type: TABLE; Schema: public; Owner: enrolluser
--

CREATE TABLE public.course (
    id character varying(20) NOT NULL,
    name character varying(50) NOT NULL,
    major character varying(50),
    grade integer,
    credit double precision,
    syllabus character varying(100),
    teacher_id character varying(20)
);


ALTER TABLE public.course OWNER TO enrolluser;

--
-- Name: courseclass; Type: TABLE; Schema: public; Owner: enrolluser
--

CREATE TABLE public.courseclass (
    id character varying(20) NOT NULL,
    course_id character varying(20) NOT NULL,
    teacher_id character varying(20) NOT NULL,
    student_class character varying(50)
);


ALTER TABLE public.courseclass OWNER TO enrolluser;

--
-- Name: courseprerequisite; Type: TABLE; Schema: public; Owner: enrolluser
--

CREATE TABLE public.courseprerequisite (
    course_id character varying(20) NOT NULL,
    pre_course_id character varying(20) NOT NULL
);


ALTER TABLE public.courseprerequisite OWNER TO enrolluser;

--
-- Name: courseschedule; Type: TABLE; Schema: public; Owner: enrolluser
--

CREATE TABLE public.courseschedule (
    classroom_id character varying(20) NOT NULL,
    course_id character varying(20) NOT NULL,
    teacher_id character varying(20) NOT NULL,
    time_slot character varying(50) NOT NULL
);


ALTER TABLE public.courseschedule OWNER TO enrolluser;

--
-- Name: enrollment; Type: TABLE; Schema: public; Owner: enrolluser
--

CREATE TABLE public.enrollment (
    student_id character varying(20) NOT NULL,
    course_id character varying(20) NOT NULL,
    score numeric(5,2) DEFAULT '-1.0'::numeric,
    enroll_time character varying(50) NOT NULL
);


ALTER TABLE public.enrollment OWNER TO enrolluser;

--
-- Name: secretary; Type: TABLE; Schema: public; Owner: enrolluser
--

CREATE TABLE public.secretary (
    id character varying(20) NOT NULL,
    name character varying(50) NOT NULL,
    dept character varying(50) NOT NULL,
    gender character varying(10)
);


ALTER TABLE public.secretary OWNER TO enrolluser;

--
-- Name: student; Type: TABLE; Schema: public; Owner: enrolluser
--

CREATE TABLE public.student (
    id character varying(20) NOT NULL,
    name character varying(50) NOT NULL,
    gender character varying(10) NOT NULL,
    dept character varying(40) NOT NULL,
    grade integer NOT NULL,
    major character varying(50) NOT NULL,
    studentclass character varying(50) NOT NULL,
    creditlimit double precision NOT NULL
);


ALTER TABLE public.student OWNER TO enrolluser;

--
-- Name: teacher; Type: TABLE; Schema: public; Owner: enrolluser
--

CREATE TABLE public.teacher (
    id character varying(20) NOT NULL,
    name character varying(50) NOT NULL,
    gender character varying(8) NOT NULL,
    dept character varying(50) NOT NULL,
    title character varying(20) NOT NULL
);


ALTER TABLE public.teacher OWNER TO enrolluser;

--
-- Data for Name: classroom; Type: TABLE DATA; Schema: public; Owner: enrolluser
--

COPY public.classroom (id, building, capacity, is_available) FROM stdin;
A101	一号教学楼	50	t
B202	二号教学楼	30	t
\.


--
-- Data for Name: classroomschedule; Type: TABLE DATA; Schema: public; Owner: enrolluser
--

COPY public.classroomschedule (classroom_id, course_id, time_slot) FROM stdin;
\.


--
-- Data for Name: course; Type: TABLE DATA; Schema: public; Owner: enrolluser
--

COPY public.course (id, name, major, grade, credit, syllabus, teacher_id) FROM stdin;
1002	数据结构	计算机	1	3	必修课	\N
1003	操作系统	计算机	2	3.5	必修课	\N
1004	Python编程	计算机	1	2	选修课	2002
1001	C++程序设计	计算机	1	2	必修课	2001
\.


--
-- Data for Name: courseclass; Type: TABLE DATA; Schema: public; Owner: enrolluser
--

COPY public.courseclass (id, course_id, teacher_id, student_class) FROM stdin;
1001-信安1801	1001	2001	信安1801
1004-安1801	1004	2002	安1801
1001-安1801	1001	2001	安1801
\.


--
-- Data for Name: courseprerequisite; Type: TABLE DATA; Schema: public; Owner: enrolluser
--

COPY public.courseprerequisite (course_id, pre_course_id) FROM stdin;
1002	1001
1003	1002
\.


--
-- Data for Name: courseschedule; Type: TABLE DATA; Schema: public; Owner: enrolluser
--

COPY public.courseschedule (classroom_id, course_id, teacher_id, time_slot) FROM stdin;
A101	1004	2002	周二1-2节
A101	1001	2001	周一1-2节
\.


--
-- Data for Name: enrollment; Type: TABLE DATA; Schema: public; Owner: enrolluser
--

COPY public.enrollment (student_id, course_id, score, enroll_time) FROM stdin;
\.


--
-- Data for Name: secretary; Type: TABLE DATA; Schema: public; Owner: enrolluser
--

COPY public.secretary (id, name, dept, gender) FROM stdin;
3001	张秘书	计算机学院	女
3002	李秘书	数学学院	男
\.


--
-- Data for Name: student; Type: TABLE DATA; Schema: public; Owner: enrolluser
--

COPY public.student (id, name, gender, dept, grade, major, studentclass, creditlimit) FROM stdin;
2018001	Jake	男	信息安全	2018	信息安全	信安1801	20
2018002	Noah	女	计算机科学与技术	2018	计算机科学与技术	计科1801	20
2018003	Olivia	女	计算机科学与技术	2018	计算机科学与技术	计科1802	20
2018004	Ethan	男	计算机科学与技术	2019	计算机科学与技术	计科1901	20
2018005	Liam	男	信息管理	2018	信息管理	信管1801	20
2018006	Zoe	男	数据科学与大数据技术	2018	数据科学	数据1801	20
\.


--
-- Data for Name: teacher; Type: TABLE DATA; Schema: public; Owner: enrolluser
--

COPY public.teacher (id, name, gender, dept, title) FROM stdin;
2001	张三	男	计算机学院	教授
2002	李四	男	计算机学院	副教授
2003	王五	女	电子工程学院	讲师
2004	赵六	女	数学学院	教授
2005	王明	男	数学学院	教授
\.


--
-- Name: classroom classroom_pkey; Type: CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.classroom
    ADD CONSTRAINT classroom_pkey PRIMARY KEY (id);


--
-- Name: classroomschedule classroomschedule_pkey; Type: CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.classroomschedule
    ADD CONSTRAINT classroomschedule_pkey PRIMARY KEY (classroom_id, time_slot);


--
-- Name: course course_pkey; Type: CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.course
    ADD CONSTRAINT course_pkey PRIMARY KEY (id);


--
-- Name: courseclass courseclass_pkey; Type: CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.courseclass
    ADD CONSTRAINT courseclass_pkey PRIMARY KEY (id);


--
-- Name: courseprerequisite courseprerequisite_pkey; Type: CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.courseprerequisite
    ADD CONSTRAINT courseprerequisite_pkey PRIMARY KEY (course_id, pre_course_id);


--
-- Name: courseschedule courseschedule_pkey; Type: CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.courseschedule
    ADD CONSTRAINT courseschedule_pkey PRIMARY KEY (classroom_id, time_slot);


--
-- Name: enrollment enrollment_pkey; Type: CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.enrollment
    ADD CONSTRAINT enrollment_pkey PRIMARY KEY (student_id, course_id);


--
-- Name: secretary secretary_pkey; Type: CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.secretary
    ADD CONSTRAINT secretary_pkey PRIMARY KEY (id);


--
-- Name: student student_pkey; Type: CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.student
    ADD CONSTRAINT student_pkey PRIMARY KEY (id);


--
-- Name: teacher teacher_pkey; Type: CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.teacher
    ADD CONSTRAINT teacher_pkey PRIMARY KEY (id);


--
-- Name: courseclass courseclass_course_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.courseclass
    ADD CONSTRAINT courseclass_course_id_fkey FOREIGN KEY (course_id) REFERENCES public.course(id) ON DELETE CASCADE;


--
-- Name: courseclass courseclass_teacher_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.courseclass
    ADD CONSTRAINT courseclass_teacher_id_fkey FOREIGN KEY (teacher_id) REFERENCES public.teacher(id) ON DELETE CASCADE;


--
-- Name: courseprerequisite courseprerequisite_course_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.courseprerequisite
    ADD CONSTRAINT courseprerequisite_course_id_fkey FOREIGN KEY (course_id) REFERENCES public.course(id) ON DELETE CASCADE;


--
-- Name: courseprerequisite courseprerequisite_pre_course_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: enrolluser
--

ALTER TABLE ONLY public.courseprerequisite
    ADD CONSTRAINT courseprerequisite_pre_course_id_fkey FOREIGN KEY (pre_course_id) REFERENCES public.course(id) ON DELETE CASCADE;


--
-- PostgreSQL database dump complete
--

