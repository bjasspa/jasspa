CREATE TABLE FORUMS (
  forumid int(11) NOT NULL auto_increment,
  categoryid int(11) NOT NULL default '0',
  forumname varchar(255) default NULL,
  description text,
  PRIMARY KEY  (forumid)
) TYPE=MyISAM;
INSERT INTO FORUMS (forumid,categoryid,forumname,description) VALUES (1,1,'iPostMX','Welcome to iPostMX 2005');
CREATE TABLE MEMBERS (
  memberid int(11) NOT NULL auto_increment,
  firstname varchar(50) default NULL,
  lastname varchar(50) default NULL,
  address varchar(50) default NULL,
  address2 varchar(50) default NULL,
  city varchar(50) default NULL,
  state varchar(30) default NULL,
  zip varchar(20) default NULL,
  age varchar(10) default NULL,
  country varchar(50) default NULL,
  username varchar(50) default NULL,
  password varchar(50) default NULL,
  email varchar(50) default NULL,
  noreceiveemail tinyint(4) NOT NULL default '0',
  signature text,
  showemail tinyint(4) NOT NULL default '0',
  posts int(11) default NULL,
  joindate date default NULL,
  lastlogin datetime default NULL,
  profilelastupdated datetime default NULL,
  accesslevel tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (memberid)
) TYPE=MyISAM;
INSERT INTO MEMBERS (memberid,firstname,lastname,address,address2,city,state,zip,age,country,username,password,email,noreceiveemail,signature,showemail,posts,joindate,lastlogin,profilelastupdated,accesslevel) VALUES (1,'admin','admin','123 Main','','Somecity','TX','78555','20','USA','admin','admin','admin@here',0,'',1,1,'2004-08-10',NULL,'2004-09-23 14:49:27',1);
CREATE TABLE MESSAGES (
  messageid int(10) unsigned NOT NULL auto_increment,
  topicid int(10) unsigned NOT NULL default '0',
  forumid int(10) unsigned NOT NULL default '0',
  memberid int(10) unsigned NOT NULL default '0',
  username varchar(50) default NULL,
  useremail varchar(50) default NULL,
  messagedate timestamp(14) NOT NULL,
  messagecopy text,
  avatar varchar(100) default '',
  format int(10) unsigned NOT NULL default '1',
  PRIMARY KEY  (messageid)
) TYPE=MyISAM;
INSERT INTO MESSAGES (messageid,topicid,forumid,memberid,username,useremail,messagedate,messagecopy,avatar,format) VALUES (8,6,1,1,'admin','admin@here',20041130170434,'Hi. Welcome to iPostMX 2005. This is a near-bare bones system with a few features.  First and foremost, let me start out by saying that it is highly recommended that you change the password for \"admin\" in your database.  Since there isn\'t an admin section built yet, you\'ll have to do it manually.<br><br>Secondly, this system is provided as-is with no warranty, written or implied.  iPostMX and its developers are not responsible for anything that may go wrong with your computer system after installation.<br><br>As mentioned before, there is no admin section so all database manipulation will have to be done manually - or if you\'re a developer you can contribute your code to do such tasks.<br><br>Since this application is OpenSource and if you plan on redistributing the source code on your own, you must keep all original credit and licenses intact.<br><br>Thanks! Enjoy and let me know if you contribute any code to this application.<br><br>Cole Barksdale<br>http://www.colebarksdale.com<br>http://www.ipostmx.com','',1);
CREATE TABLE TOPICS (
  topicid int(10) unsigned NOT NULL auto_increment,
  forumid int(10) unsigned NOT NULL default '0',
  topictitle varchar(255) default NULL,
  views int(10) unsigned default NULL,
  membername varchar(50) default NULL,
  lastmessagedate datetime default NULL,
  notifyauthor tinyint(3) unsigned default NULL,
  notifyemail varchar(50) default NULL,
  memberid int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (topicid)
) TYPE=MyISAM;
INSERT INTO TOPICS (topicid,forumid,topictitle,views,membername,lastmessagedate,notifyauthor,notifyemail,memberid) VALUES (6,1,'Welcome to iPostMX 2005',0,'admin','2005-05-18 17:00:00',0,'admin@here',1);
CREATE TABLE CATEGORIES (
  catid int(11) NOT NULL auto_increment,
  category varchar(50) default NULL,
  PRIMARY KEY  (catid)
) TYPE=MyISAM;
INSERT INTO CATEGORIES (catid,category) VALUES (1,'iPostMX 2005');