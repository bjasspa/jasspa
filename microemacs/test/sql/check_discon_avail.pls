clear buffer
SET arraysize 1
SET termout OFF
SET linesize 132
SET echo ON
SET scan off
spool get_ldap.lst
SET SERVEROUTPUT ON;

create or replace procedure check_discon_date is

   cursor get_available_discon is
      select course_code, rowid
        from course
       where nvl(discon_date,sysdate+1) < sysdate
         and status_text = 'AVAILABLE'
       order by course_code;

   discontinued_list varchar2(20000);

   mailhost       VARCHAR2 (30) := 'zzzz.zzz.zzz';
   v_connection   UTL_SMTP.CONNECTION;
   mylogfile varchar2(30) := 'course_discon_avail.log';
   mailto varchar2(20) := 'trainadm@zzz.zzz';
   mailfrom varchar2(20) := 'trainadm@zzz.zzz';
   discon_count pls_integer := 0;

BEGIN

   discontinued_list := '';
   trainrpt_util.ClearMyMessageFile(mylogfile);
   trainrpt_util.LogMyMessage(mylogfile,'Course processing started at ' || to_char(sysdate,'mm/dd/yyyy hh:mi:ss'),true,true);
   trainrpt_util.LogMyMessage(mylogfile,'Building list',true,true);
   for e in get_available_discon loop
      trainrpt_util.LogMyMessage(mylogfile,e.course_code,true,true);
      discontinued_list := discontinued_list || e.course_code || ' ';
      discon_count := discon_count + 1;
   end loop;

   if discon_count > 0 then
       discontinued_list := 'The following ' || to_char(discon_count) || ' courses had discontinue dates in the past, but were still marked as AVAILABLE. Their status was changed to DISCONTINUED: ' || discontinued_list;
       v_connection := UTL_SMTP.OPEN_CONNECTION(mailhost,25);
       UTL_SMTP.HELO(v_connection,mailhost);
       UTL_SMTP.MAIL(v_connection,mailfrom);
       UTL_SMTP.RCPT(v_connection,mailto);
       UTL_SMTP.DATA(v_connection,discontinued_list);
       UTL_SMTP.QUIT(v_connection);
       for e in get_available_discon loop
          begin
             trainrpt_util.LogMyMessage(mylogfile,e.course_code || ' DISCONTINUED',true,true);
             update course
                set status_text = 'DISCONTINUED'
              where rowid=e.rowid
                and course_code=e.course_code;

             commit;

          exception
             when others then
                trainrpt_util.LogMyMessage(mylogfile,e.course_code || ' '||e.rowid || ' ' || SQLERRM,true,true);
                v_connection := UTL_SMTP.OPEN_CONNECTION(mailhost,25);
                UTL_SMTP.HELO(v_connection,mailhost);
                UTL_SMTP.MAIL(v_connection,mailfrom);
                UTL_SMTP.RCPT(v_connection,mailto);
                UTL_SMTP.DATA(v_connection,' The following error occured while attempting to update course: ' || e.course_code || substr(SQLERRM,1,75));
                UTL_SMTP.QUIT(v_connection);
                EXIT;
          end;
       end loop;
   else
      v_connection := UTL_SMTP.OPEN_CONNECTION(mailhost,25);
      UTL_SMTP.HELO(v_connection,mailhost);
      UTL_SMTP.MAIL(v_connection,mailfrom);
      UTL_SMTP.RCPT(v_connection,mailto);
      UTL_SMTP.DATA(v_connection,'For ' || to_char(sysdate,'mm/dd/yyyy') || ' there were no discontinued courses that were still marked as available.');
      UTL_SMTP.QUIT(v_connection);
   end if;

exception
   when others then
      trainrpt_util.LogMyMessage(mylogfile,' ***** General Error ' || SQLERRM,true,true);

end;
/

