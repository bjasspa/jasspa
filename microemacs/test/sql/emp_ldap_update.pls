clear buffer
SET arraysize 1
SET termout OFF
SET linesize 132
SET echo ON
SET scan off
spool emp_ldap_update.lst
SET SERVEROUTPUT ON;

CREATE OR REPLACE PACKAGE emp_ldap_update as

   mylogfile varchar2(25) := 'ldap_emp_update.log';
   trainadm  varchar2(25) := 'TRAINADM@INEL.GOV';
   developer varchar2(25) := 'RLO@INEL.GOV';

   constructionorg varchar2(5) := '6302';
   flag varchar2(1);

procedure add_job_courses (p_emp_ssn_num in varchar2,
                                p_job_code in varchar2);

procedure add_inelcn_readings(p_emp_ssn_num in varchar2);

procedure add_jc0001_readings (p_emp_ssn_num in varchar2);

procedure add_job_jc0001 (p_emp_ssn_num in varchar2);

procedure emp_termination_process (p_emp_ssn_num in varchar2,
                                   p_cmp_alt_id in varchar2);

procedure build_employee_jobs (p_emp_ssn_num in varchar2);

PROCEDURE Update_Emp_Table;

procedure MailMessage(sender in varchar2,
                      recipient in varchar2,
                      message in varchar2);

end;
/
CREATE OR REPLACE PACKAGE BODY emp_ldap_update AS

procedure oa_update (p_mail_user_id in varchar2,
                     p_homeorg in varchar2,
                     p_emp_ssn_num in varchar2) is

   sql_err varchar2(50);
   descript varchar2(80);

begin
   <<chk_dup_block>>

   /* Check for existing record in table oracle_accounts */
   /* insert if new and update if exists                  */
   begin
      select 'Y'
         into flag
         from oracle_accounts
         where oracle_accounts.account_user_id = p_mail_user_id;

      if SQL%FOUND then
      begin
         update oracle_accounts
            set org_code = p_homeorg,
                           ssn_num = p_emp_ssn_num
            where account_user_id = p_mail_user_id;
         commit;

         trainrpt_util.LogMyMessage(mylogfile,'oa_update: updated '|| p_emp_ssn_num,true,true);

      end;
      end if;

   exception
      when NO_DATA_FOUND then
      begin
         insert
            into oracle_accounts (ssn_num, account_user_id,appl_name,org_code, default_printer_queue)
         values (p_emp_ssn_num, p_mail_user_id, 'TRAIN', p_homeorg,' ' );

         trainrpt_util.LogMyMessage(mylogfile,'oa_update: inserted '|| p_emp_ssn_num,true,true);
         commit;
      end;

      when OTHERS then
         sql_err := substr(sqlerrm, 1, 50);
         descript := 'Unknown insert/update error, primary key is - '|| p_emp_ssn_num || ' ' || p_mail_user_id;
         trainrpt_util.LogMyMessage(mylogfile,'oa_update: ' || sql_err || descript,true,true);

   end chk_dup_block;
end;

procedure toas_update (p_emp_ssn_num in varchar2,
                       p_old_org in varchar2,
                       p_new_org in varchar2,
                       p_cmp_alt_id in varchar2) is

   real_org            varchar2(5);
   sql_err             varchar2(50);
   descript            varchar2(80);
   counter             number(8);
   found               varchar2(1);
   flag                varchar2(1);
   new_org             varchar2(5);
   new_org_code        varchar2(5);
   old_org             varchar2(5);
   old_org_code        varchar2(5);
   addrecn             number(8);
   updrecn             number(8);
   delrecn             number(8);
   numrec              number(8);

begin

   select count(*)
     into numrec
     from train_oracle_accounts_security
    where account_user_id = p_cmp_alt_id;

   if numrec > 0 then
      /* update if needed the train_oracle_accounts_security table */

      /* if org change first see if new org already exists */
      /* and just delete old record */
      <<chk_exists>>
      begin
         if p_old_org <> p_new_org then
         begin
            select 'Y'
               into found
               from train_oracle_accounts_security
               where  account_user_id = p_cmp_alt_id
               and org_id = p_new_org;

            /* ************* */
            if SQL%FOUND then
            begin
               /* see if old org is in the table and delete it */
               select 'Y'
                  into found
                  from train_oracle_accounts_security
                  where  account_user_id = p_cmp_alt_id
                     and org_id = p_old_org;

               /* ******* */
               if SQL%FOUND then
               begin
                  delrecn := delrecn + 1;
                  sql_err := 'DELETing org ' || p_old_org || ' ' || p_cmp_alt_id;
                  descript := 'for user ' || p_emp_ssn_num || ' ' || p_new_org ||' '||new_org;
                  trainrpt_util.LogMyMessage(mylogfile,'toas_update: ' || sql_err || descript,true,true);

                  delete
                     from train_oracle_accounts_security
                     where account_user_id = p_cmp_alt_id
                     and org_id = p_old_org;

                  commit;
                  goto end_loop;
               end;
               end if;

               /* do nothing since already exists, moved into the org they are assignned */
            exception
               when others then
                  goto end_loop;
            end;
            end if;
            /* ******** */

            /* not a existing record so do nothing */
         exception
            when others then
               counter := counter;
         end;
         end if;
      end chk_exists;

      /* see if update or insert is needed */
      << upd_tr_ora_accts_sec>>
      begin
         if p_old_org <> p_new_org then
         begin
            /* see what needs to be done for org changing  */
            select 'Y'
               into found
               from train_oracle_accounts_security
               where  account_user_id = p_cmp_alt_id
               and org_id = p_old_org;

            if SQL%FOUND then
            begin
               updrecn := updrecn + 1;
               sql_err := 'changing org ' || p_old_org || ' ' || p_cmp_alt_id;
               descript := 'for user ' || p_emp_ssn_num || ' to ' || p_new_org;
               descript := descript || ' ' || new_org;
               trainrpt_util.LogMyMessage(mylogfile,'toas_update: ' || sql_err || descript,true,true);

               update train_oracle_accounts_security
                  set org_id = p_new_org
                  where org_id = p_old_org
                  and account_user_id = p_cmp_alt_id;

               commit;

            end;
            end if;

         exception
            when NO_DATA_FOUND then
            /* no existing record in the system for the old org  */
            begin
               addrecn := addrecn + 1;
               sql_err := 'INSERTing TRAIN_ORACLE_ACCOUNTS_SECURITY record';
               descript := 'for user ' || p_emp_ssn_num || ' ' || p_cmp_alt_id;
               descript := descript || ' with org ' || p_new_org||' '||new_org;
               trainrpt_util.LogMyMessage(mylogfile,'toas_update: ' || sql_err || descript,true,true);

               insert into train_oracle_accounts_security (account_user_id, org_id)
               values (p_cmp_alt_id,p_new_org);

               commit;
            end;

            when others then
            begin
               sql_err := substr(sqlerrm,1,50);
               descript := 'unknow error on train_oracle_accounts_security ';
               descript := descript|| p_emp_ssn_num || ' ' || p_cmp_alt_id;
               trainrpt_util.LogMyMessage(mylogfile,'toas_update: ' || sql_err || descript,true,true);
            end;

         end;
         end if;
      end upd_tr_ora_accts_sec;

      <<end_loop>>
      null;

   end if;

end;

FUNCTION get_jobcode4char  RETURN varchar2 is

   c1  number(2);
   c2  number(2);
   c3  number(2);
   c4  number(2);
   jc  varchar2(6);

BEGIN
   jc := NULL;
   select col1, col2, col3, col4
     into c1, c2, c3, c4
     from jobcode4char
    where rownum = 1;

   if SQL%FOUND then
   begin
      c4 := c4 + 1;
      if c4 = 36 then
      begin
         c3 := c3 + 1;
         c4 := 0;
         if c3 = 36 then
         begin
            c2 := c2 + 1;
            c3 := 0;
            if c2 = 36 then
            begin
               c1 := c1 + 1;
               c2 := 0;
               if c1 = 36 then
               begin
                  c1 := 0;
                  c2 := 0;
                  c3 := 0;
                  c4 := 0;
               end; /* c1 */
               end if;
            end; /* c2 */
            end if;
         end; /* c3 */
         end if;
      end; /* c4 */
      end if;

      begin
         update jobcode4char
            set col1 = c1, col2 = c2, col3 = c3, col4 = c4
          where rownum = 1;

      end;
      begin
         select 'JC'||
               decode(c1,0,'0',1,'1',2,'2',3,'3',4,'4',5,'5',6,'6',7,'7',8,'8',9,'9',chr(c1 + 55)) ||
               decode(c2,0,'0',1,'1',2,'2',3,'3',4,'4',5,'5',6,'6',7,'7',8,'8',9,'9',chr(c2 + 55)) ||
               decode(c3,0,'0',1,'1',2,'2',3,'3',4,'4',5,'5',6,'6',7,'7',8,'8',9,'9',chr(c3 + 55)) ||
               decode(c4,0,'0',1,'1',2,'2',3,'3',4,'4',5,'5',6,'6',7,'7',8,'8',9,'9',chr(c4 + 55))
          into jc
         from dual;
      end;

      trainrpt_util.LogMyMessage(mylogfile,'get_jobcode4char: created jobcode '||jc,true,true);
      RETURN jc;

   end;
   end if;

exception
   when OTHERS then
      trainrpt_util.LogMyMessage(mylogfile,'get_jobcode4char: created jobcode '||jc||' (exception path)',true,true);
      RETURN jc;

END;

procedure addblankempjob(empssn in varchar2,
                        jobcode in varchar2,
                        wasok in out varchar2) is

/* 4/24/2001   add blank job code to an employee   MPJ     */

   orgcd     varchar2(5);
   faccd     varchar2(2);
   locnm     varchar2(6);
   poc       varchar2(9);

begin
   orgcd := '99718';  /* temporary defaults for old keys */
   faccd := 'AA';
   locnm := 'INEEL';

   /* job code to employee  */
   <<emp_jobcd>>
   begin
      wasok := 'Y';
      insert into emp_job (emp_ssn_num, job_code, org_code, facility_code,loc_num, fit_for_duty_flag, asgmt_serv_unit_start_date)
             values(empssn, jobcode, orgcd, faccd, locnm, 'Y', sysdate);

   trainrpt_util.LogMyMessage(mylogfile,'addblankempjob: Inserted '||jobcode||' for '||empssn,true,true);

   exception
      when others then
         wasok := 'N';

   end emp_jobcd;
   wasok := wasok;

end;

procedure buildempjobcode(empssn in varchar2,
                          jobcode in varchar2,
                          jtitle in varchar2,
                          wasok in out varchar2)   is

/* 4/26/2001   build new job code   MPJ                          */
/* last 6 char of empssn is used in the location and jc_modifier */

   orgcd     varchar2(5);
   faccd     varchar2(2);
   locnm     varchar2(6);
   poc       varchar2(9);

begin
   wasok := 'Y';
   orgcd := '99718';  /* temporary defaults for old keys */
   faccd := 'AA';
   locnm := 'INEEL';
   poc := '888888885';

   /* build job record */
   <<build_job>>
   begin
      insert into job (job_code, org_code, facility_code, loc_num, job_desc, emp_ssn_num,
                     update_date, update_uid, in_book1, on_qual_card, function, position,
                     location, jc_modifier, job_title, resp_org)
      values(jobcode, orgcd, faccd, locnm, jtitle, poc,sysdate, 'TRAIN', 'N', 'N', '00', '00',
             substr(empssn,4,2), substr(empssn,6,4), jtitle, orgcd);

      trainrpt_util.LogMyMessage(mylogfile,'buildempjobcode: Inserted '||jobcode||' for '||empssn,true,true);

   exception
      when others then
         wasok := 'N';
         goto end_loop;

   end build_job;

   /* put title into job_code table */
   <<build_jobcode>>
   begin
      insert into job_code (job_code,job_code_desc)
      values (jobcode, jtitle);

      trainrpt_util.LogMyMessage(mylogfile,'buildempjobcode: Inserted '||jobcode||' title '||jtitle,true,true);

   exception
      when others then
         wasok := 'N';
         goto end_loop;

   end build_jobcode;

   <<end_loop>>
   wasok := wasok;
end;

procedure buildjobprog(jobcode in varchar2,
                       progcode in varchar2,
                       ptitle in varchar2,
                       porg in varchar2,
                       wasok in out varchar2 ) is

/* 4/26/2001 build new program code and attach to job code  MPJ */
/* 6/16/2001 pass in ORG (porg) to use in the program POC   MPJ */

   orgcd     varchar2(5);
   faccd     varchar2(2);
   locnm     varchar2(6);
   poc       varchar2(9);

begin
   wasok := 'Y';
   orgcd := '99718'; /* temporary defaults for old keys */
   faccd := 'AA';
   locnm := 'INEEL';
   poc := '888888885';

   /* set up program code and attach to job code */
   <<setup_pc>>
   begin
      begin
         insert into program (prog_code, accred_flag, avail_date, discon_date,est_cmpl_mth, prog_title, type_prog_code, parent_prog_flag)
                values(progcode, 'N', sysdate, NULL, 24, ptitle, 'NA', 'N');

      trainrpt_util.LogMyMessage(mylogfile,'buildjobprod: Inserted program '||progcode,true,true);

      exception
         when others then
            wasok := 'N';
            goto end_loop;
      end;

      begin  /* set up link between job and program */
         insert into job_prog (job_code, org_code, facility_code, loc_num, prog_code)
                values(jobcode, orgcd, faccd, locnm, progcode );

         trainrpt_util.LogMyMessage(mylogfile,'buildjobprod: Inserted job '||jobcode||' prog '||progcode,true,true);

      exception
         when others then
            wasok := 'N';
            goto end_loop;
      end;

   end setup_pc;

   <<end_loop>>
   wasok := wasok;
end;

PROCEDURE next_jobcode4char(jc out varchar2,
                            errcode out varchar2) is

   haveit varchar2(1);

BEGIN
   errcode :=  'N';
   haveit := 'N';
   jc := NULL;

   while haveit = 'N' LOOP
      errcode := 'Y';
      jc := get_jobcode4char;
      if jc <> 'JC0000' then
      begin
         errcode := 'N';
         select 'N'
           into errcode
           from JOB
          where job_code = jc;

         if SQL%FOUND then
            NULL;
         end if;

      exception
         when NO_DATA_FOUND then
            begin
               select 'N'
                 into errcode
                 from PROGRAM
                where PROG_CODE = substr(jc, 3,4);
               if SQL%FOUND then
                  NULL;
               end if;

            exception
               when NO_DATA_FOUND then
                  haveit := 'Y';
                  when others then
                  begin
                     jc := 'JC0000';
                     haveit := 'Y';
                     errcode := 'Y';
                  end;
            end;
         when others then
         begin
            jc := 'JC0000';
            haveit := 'Y';
            errcode := 'Y';
         end;
      end;
      end if;
   END LOOP;

END;

procedure add_job_courses (p_emp_ssn_num in varchar2,
                                p_job_code in varchar2) is
/**********************************************************************/
/* add_job_courses       replaces special programs to do this         */
/* 9/1/2000 Adds or sets to active JOB PROGRAM COURSES in a persons   */
/*          ITP - TRAINING PLAN                                       */
/**********************************************************************/

   LOC_COUNT  NUMBER;

   CURSOR GET_EMP (P_EMP_SSN_NUM IN VARCHAR2, P_JOBCODE IN VARCHAR2) IS
      SELECT A.EMP_SSN_NUM, B.COURSE_CODE, B.INSTR_TYPE_CODE
        FROM EMP_JOB A, PROG_COURSE B
       WHERE A.JOB_CODE = P_JOBCODE
         AND A.EMP_SSN_NUM = P_EMP_SSN_NUM
         AND B.PROG_CODE IN (SELECT PROG_CODE
                               FROM JOB_PROG
                              WHERE JOB_CODE = P_JOBCODE);
BEGIN

   LOC_COUNT := 0;
   FOR EMP_REC IN GET_EMP(p_emp_ssn_num,p_job_code) LOOP
      BEGIN
         INSERT INTO INDIV_TRAIN_PLAN  (EMP_SSN_NUM, UPDATE_UID, UPDATE_DATE, COURSE_CODE,
                     INSTR_TYPE_CODE, COURSE_ACTIVE_STAT_FLAG)
              VALUES (EMP_REC.EMP_SSN_NUM, 'COURSEADD', SYSDATE, EMP_REC.COURSE_CODE,EMP_REC.INSTR_TYPE_CODE, 'A');

      trainrpt_util.LogMyMessage(mylogfile,'add_job_courses: Inserted '||emp_rec.course_code||' for '||emp_rec.emp_ssn_num,true,true);

      EXCEPTION
         WHEN DUP_VAL_ON_INDEX THEN
            begin
               update indiv_train_plan
                  set course_active_stat_flag = 'A',
                      update_uid = 'COURSEADD',
                      update_date = sysdate
                where emp_ssn_num = emp_rec.emp_ssn_num
                  and course_code = emp_rec.course_code
                  and instr_type_code = emp_rec.instr_type_code
                  and course_active_stat_flag = 'I';

               trainrpt_util.LogMyMessage(mylogfile,'add_job_courses: Updated '||emp_rec.course_code||' for '||emp_rec.emp_ssn_num,true,true);

            exception
               when others then
                  NULL;
            end;

         when others then
            NULL;
      END;

   END LOOP;
   COMMIT;

end;

procedure add_inelcn_readings(p_emp_ssn_num in varchar2) is
/**********************************************************************/
/* 6/17/2000           add_inelcn_readings.sql                        */
/* this sql script will be run after add_job_inelcn.sql to add the    */
/* 'INELCN' program readings to the MISTY_RQR_TRN table and insert    */
/* or update the EMP_READING_PLAN table as required                   */
/**********************************************************************/

   MAXREV     VARCHAR2(4);
   CURSOR GET_EMP(P_EMP_SSN_NUM IN VARCHAR2) IS
      SELECT A.EMP_SSN_NUM, B.READ_CODE, B.PROG_CODE, B.FREQ_CODE,B.RE_READ_INTVL_MTH
        FROM EMP_JOB A, PROG_READING B
       WHERE A.JOB_CODE = 'INELCN'
         AND A.EMP_SSN_NUM=P_EMP_SSN_NUM
         AND B.PROG_CODE IN (SELECT PROG_CODE
                               FROM JOB_PROG
                              WHERE JOB_CODE = 'INELCN');
BEGIN

   FOR EMP_REC IN GET_EMP(P_EMP_SSN_NUM) LOOP
      BEGIN
      /* get max reading revision in case it is needed for new record */
         SELECT MAX(READ_REV)
           INTO MAXREV
           FROM READING
          WHERE READ_CODE = EMP_REC.READ_CODE;
      EXCEPTION
         WHEN OTHERS THEN
            MAXREV := '0000';
      END;

      begin /* misty_rqr_trn update */
         insert into misty_rqr_trn (emp_ssn_num, job_code, prog_code, trn_code, trn_status,
                     freq_code, re_read_intvl_mth, update_uid, update_date,initial_uid, initial_date)
               values (EMP_REC.EMP_SSN_NUM, 'INELCN', EMP_REC.PROG_CODE,EMP_REC.READ_CODE, 'A', EMP_REC.FREQ_CODE,
                        EMP_REC.RE_READ_INTVL_MTH, 'ADDINELCN', sysdate, 'ADDINELCN',sysdate);

         trainrpt_util.LogMyMessage(mylogfile,'add_inelcn_readings: Inserted '||emp_rec.read_code||' into misty_rqr_trn for '||emp_rec.emp_ssn_num,true,true);

      exception
         when others then
            NULL;
      end;  /* end of misty insertion */

      BEGIN /* erp udpate */
         INSERT INTO EMP_READING_PLAN(EMP_SSN_NUM,  READ_CODE, READ_REV, ACT_STAT_FLAG,RE_READ_INTVL_MTH, FREQ_CODE, UPDATE_UID, UPDATE_DATE)
               VALUES (EMP_REC.EMP_SSN_NUM, EMP_REC.READ_CODE,MAXREV, 'A', 0,'I', 'ADDINELCN', SYSDATE);
      EXCEPTION
         WHEN DUP_VAL_ON_INDEX THEN
         begin
            update EMP_READING_PLAN
               set act_stat_flag = 'A',
                     update_uid = 'ADDINELCN',
                     update_date = sysdate
               where emp_ssn_num = emp_rec.emp_ssn_num
               and read_code = emp_rec.read_code
               and act_stat_flag = 'I';

            trainrpt_util.LogMyMessage(mylogfile,'add_inelcn_readings: Inserted '||emp_rec.read_code||' into emp_reading_plan for '||emp_rec.emp_ssn_num,true,true);

         exception
            when others then
               NULL;
         end;

      WHEN OTHERS then
         NULL;
      END; /* erp update */


   END LOOP;
   COMMIT;

END;

procedure add_job_inelcn (p_emp_ssn_num in varchar2) is
/******************************************************************/
/*  6/12/2000 MPJ   add_job_inelcn.sql                            */
/*  ROUTINE TO ADD 'INELCN' JOB TO ALL CONSTRUCTION EMPLOYEES in  */
/*  ORG 6302 who do not currently have the job.  Then run         */
/*  add_inelcn_courses.sql to load courses into training plan     */
/******************************************************************/

   LOC_EMP_SSN_NUM   VARCHAR2(9);
   LOC_JOB_CODE      VARCHAR2(6);
   LOC_ORG_CODE      VARCHAR2(5);
   LOC_FACILITY_CODE VARCHAR2(2);
   LOC_LOCNUM        VARCHAR2(8);
   LOC_DATE          DATE;


   CURSOR GET_EMP (P_EMP_SSN_NUM IN VARCHAR2) IS
      SELECT A.EMP_SSN_NUM,'INELCN','99772','AA','CONST'
        FROM EMPLOYEE A, ORG B
       WHERE A.ORG_CODE IS NOT NULL
         AND A.EMP_SSN_NUM = P_EMP_SSN_NUM
         AND A.EMP_ACTIVE_STAT_FLAG = 'A'
         AND A.CO_CODE = '2115'
         AND B.ORG_CODE = A.ORG_CODE
         AND B.OLD_ORG_CODE = constructionorg
     MINUS
      SELECT EMP_SSN_NUM,JOB_CODE,ORG_CODE,FACILITY_CODE,LOC_NUM
        FROM EMP_JOB
       WHERE JOB_CODE = 'INELCN'
         AND EMP_SSN_NUM = P_EMP_SSN_NUM
         AND LOC_NUM = 'CONST';

BEGIN

   OPEN GET_EMP (P_EMP_SSN_NUM);
   LOOP

      FETCH GET_EMP
       INTO LOC_EMP_SSN_NUM,LOC_JOB_CODE,LOC_ORG_CODE,LOC_FACILITY_CODE,LOC_LOCNUM;

      EXIT WHEN GET_EMP%NOTFOUND;

      INSERT INTO EMP_JOB (EMP_SSN_NUM,JOB_CODE,ORG_CODE,FACILITY_CODE,LOC_NUM,ASGMT_SERV_UNIT_START_DATE,FIT_FOR_DUTY_FLAG)
                  SELECT LOC_EMP_SSN_NUM,LOC_JOB_CODE,LOC_ORG_CODE,LOC_FACILITY_CODE,LOC_LOCNUM,nvl(EMP_ORIG_HIRE_DATE,sysdate),'Y'
                    FROM EMPLOYEE
                   WHERE EMP_SSN_NUM = LOC_EMP_SSN_NUM;

      trainrpt_util.LogMyMessage(mylogfile,'add_job_inelcn: Inserted job INELCN for '||loc_emp_ssn_num,true,true);

   END LOOP;
   CLOSE GET_EMP;
   COMMIT;

END;

procedure add_jc0001_readings (p_emp_ssn_num in varchar2) is
/**********************************************************************/
/* 5/16/2001           add_jc0001_readings.sql                        */
/* this sql script will be run after add_job_jc0001.sql to add the    */
/* 'JC0001' program readings to the MISTY_RQR_TRN table and insert    */
/* or update the EMP_READING_PLAN table as required                   */
/**********************************************************************/


   MAXREV     VARCHAR2(4);

   CURSOR GET_EMP (p_emp_ssn_num in varchar2) IS
      SELECT A.EMP_SSN_NUM, B.READ_CODE, B.PROG_CODE, B.FREQ_CODE,B.RE_READ_INTVL_MTH
        FROM EMP_JOB A, PROG_READING B
       WHERE A.JOB_CODE = 'JC0001'
         and a.emp_ssn_num = p_emp_ssn_num
         AND B.PROG_CODE IN (SELECT PROG_CODE
                               FROM JOB_PROG
                              WHERE JOB_CODE = 'JC0001');

BEGIN

   FOR EMP_REC IN GET_EMP(p_emp_ssn_num) LOOP

      BEGIN
      /* get max reading revision in case it is needed for new record */
         SELECT MAX(READ_REV)
            INTO MAXREV
            FROM READING
            WHERE READ_CODE = EMP_REC.READ_CODE;
      EXCEPTION
         WHEN OTHERS THEN
            MAXREV := '0000';
      END;

      begin /* misty_rqr_trn update */
         insert into misty_rqr_trn (emp_ssn_num, job_code, prog_code, trn_code, trn_status,freq_code,
                                    re_read_intvl_mth, update_uid, update_date,initial_uid, initial_date)
         values (EMP_REC.EMP_SSN_NUM, 'JC0001', EMP_REC.PROG_CODE,EMP_REC.READ_CODE, 'A', EMP_REC.FREQ_CODE,
                  EMP_REC.RE_READ_INTVL_MTH, 'ADDJC0001', sysdate, 'ADDJC0001',sysdate);

         trainrpt_util.LogMyMessage(mylogfile,'add_jc0001_readings: Inserted '||emp_rec.read_code||' into misty_rqr_trn for '||emp_rec.emp_ssn_num,true,true);

      exception
         when others then
            NULL;
      end;  /* end of misty insertion */

      BEGIN /* erp udpate */
         INSERT INTO EMP_READING_PLAN (EMP_SSN_NUM,  READ_CODE, READ_REV, ACT_STAT_FLAG,RE_READ_INTVL_MTH, FREQ_CODE, UPDATE_UID, UPDATE_DATE)
         VALUES (EMP_REC.EMP_SSN_NUM, EMP_REC.READ_CODE, MAXREV, 'A', 0,'I', 'ADDJC0001', SYSDATE);

         trainrpt_util.LogMyMessage(mylogfile,'add_jc0001_readings: Inserted '||emp_rec.read_code||' into emp_reading_plan for '||emp_rec.emp_ssn_num,true,true);

      EXCEPTION
         WHEN DUP_VAL_ON_INDEX THEN
            begin
               update EMP_READING_PLAN
                  set act_stat_flag = 'A',
                        update_uid = 'ADDJC0001',
                        update_date = sysdate
                  where emp_ssn_num = emp_rec.emp_ssn_num
                  and read_code = emp_rec.read_code
                  and act_stat_flag = 'I';

               trainrpt_util.LogMyMessage(mylogfile,'add_jc0001_readings: Updated '||emp_rec.read_code||'  in emp_reading_plan for '||emp_rec.emp_ssn_num,true,true);

            exception
            when others then
               NULL;
            end;
         WHEN OTHERS then
            NULL;
      END; /* erp update */

   END LOOP;
   COMMIT;
END;

procedure add_job_jc0001 (p_emp_ssn_num in varchar2) is
/************************************************************************/
/*  ROUTINE TO ADD 'JC0001' JOB TO ALL EMPLOYEES WHO DO NOT HAVE IT     */
/*  5/16/2001  MPJ                                                      */
/************************************************************************/

   LOC_EMP_SSN_NUM   VARCHAR2(9);
   LOC_JOB_CODE      VARCHAR2(6);
   LOC_ORG_CODE      VARCHAR2(5);
   LOC_FACILITY_CODE VARCHAR2(2);
   LOC_LOCNUM        VARCHAR2(8);
   LOC_DATE          DATE;


   CURSOR GET_EMP (p_emp_ssn_num in varchar2) IS
      SELECT a.EMP_SSN_NUM, 'JC0001', '99157', 'AA', 'INEEL'
        FROM EMPLOYEE a, org b
       WHERE a.ORG_CODE IS NOT NULL
         AND A.EMP_SSN_NUM = P_EMP_SSN_NUM
         AND a.EMP_ACTIVE_STAT_FLAG = 'A'
         AND a.CO_CODE = '2115'
         and b.org_code = a.org_code
         and b.old_org_code <> constructionorg
      MINUS
      SELECT EMP_SSN_NUM, JOB_CODE, ORG_CODE, FACILITY_CODE, LOC_NUM
        FROM EMP_JOB
       WHERE JOB_CODE = 'JC0001'
         and emp_ssn_num = p_emp_ssn_num;

BEGIN

   OPEN GET_EMP (p_emp_ssn_num);

   FETCH GET_EMP
      INTO LOC_EMP_SSN_NUM,LOC_JOB_CODE,LOC_ORG_CODE,LOC_FACILITY_CODE,LOC_LOCNUM;

   IF NOT GET_EMP%NOTFOUND THEN
      INSERT INTO EMP_JOB (EMP_SSN_NUM,JOB_CODE,ORG_CODE,FACILITY_CODE,LOC_NUM,ASGMT_SERV_UNIT_START_DATE,FIT_FOR_DUTY_FLAG)
                  SELECT LOC_EMP_SSN_NUM,LOC_JOB_CODE,LOC_ORG_CODE,LOC_FACILITY_CODE,LOC_LOCNUM,nvl(EMP_ORIG_HIRE_DATE,sysdate),'Y'
                     FROM EMPLOYEE
                     WHERE EMP_SSN_NUM = LOC_EMP_SSN_NUM;

      trainrpt_util.LogMyMessage(mylogfile,'add_job_jc0001: Inserted JC0001 for '||loc_emp_ssn_num,true,true);
      COMMIT;
   END IF;

   CLOSE GET_EMP;
   COMMIT;

END;

procedure  build_employee_jobs (p_emp_ssn_num in varchar2) is
/* 4/18/2001   build_employee_jobs.sql   MPJ     */
/* used to build employee specific job code with */
/* attached program code                         */
/* 5/16/2001 revised to check company table to   */
/*        see if ADD_SPEC_JC is set to Y to add. */
/* 6/19/2001 revised jobprog call to have        */
/*       employee org in it                      */
/* 9/05/2002 revised to become part of the       */
/*       emp_ldap_update process                 */

   emp_rec   train.employee%rowtype;
   wasok     varchar2(1);
   errflg    varchar2(1);
   jobcode   varchar2(6);
   progcode  varchar2(5);
   jobtt     varchar2(30);
   prgtt     varchar2(30);

   cursor get_emp (p_emp_ssn_num in varchar2) is
      select *
        from employee
       where emp_active_stat_flag ='A'
         and emp_ssn_num = p_emp_ssn_num
         and ( (co_code in (select co_code
                              from company c
                             where c.co_code = employee.co_code
                               and c.add_spec_jc = 'Y'))
            or (co_code not in (select co_code
                                  from company d
                                 where d.co_code = employee.co_code
                                   and d.add_spec_jc = 'Y')
               and 'IC' = (select substr(org_id,1,2)
                             from org
                            where org_code = employee.org_code) ) )
         and ( substr(emp_ssn_num,4,6) not in (select substr(job_title,23,6)
                                                 from job
                                                where substr(job_title,1,21) = 'PERSONAL JOB CODE FOR'));
begin

   open get_emp (p_emp_ssn_num);
   loop
      fetch get_emp into emp_rec;
      if get_emp%NOTFOUND then
         exit;
      end if;

      errflg := 'N';
      wasok := 'Y';
      jobtt := 'PERSONAL JOB CODE FOR '||SUBSTR(emp_rec.EMP_SSN_NUM,4,6);
      prgtt := 'PERSONAL PRG FOR '||SUBSTR(emp_rec.EMP_SSN_NUM,4,6);

      <<chk_jc>> /* see if employee already has a personal job code */
      begin
         select 'Y',job_code
           into errflg,jobcode
           from job
          where job_title = jobtt;

         if SQL%FOUND then
         begin
            trainrpt_util.LogMyMessage(mylogfile,SUBSTR(emp_rec.EMP_SSN_NUM,4,6)||' already has personal job code '||jobcode,true,true);
            goto end_loop;
         end;
         end if;

      exception
         when NO_DATA_FOUND then
            errflg := 'N';

         when others then
            begin
            errflg := 'Y';
            trainrpt_util.LogMyMessage(mylogfile,SUBSTR(emp_rec.EMP_SSN_NUM,4,6)||' error in personal job code check',true,true);
            goto end_loop;
      end;
      end chk_jc;

      <<setup_jc>>
      begin
         jobcode := NULL;     /* get job code sequence number */
         next_jobcode4char(jobcode,errflg);
         if errflg = 'Y' then
            begin
               trainrpt_util.LogMyMessage(mylogfile,'problem in getting job code for '||emp_rec.emp_ssn_num,true,true);
            goto end_loop;
            end;
         end if;
         begin      /* build job and job code records */
            buildempjobcode(emp_rec.emp_ssn_num, jobcode, jobtt, wasok );
            if wasok = 'N' then
               trainrpt_util.LogMyMessage(mylogfile,'problem inserting job '||jobcode||' for '||emp_rec.emp_ssn_num,true,true);
               goto end_loop;
            end if;
         end;
      end setup_jc;

      <<setup_pc>> /* set up program code and attach to job code */
      begin
         prgtt := prgtt||' '||jobcode;
         progcode := substr(jobcode,3,4);
         buildjobprog(jobcode, progcode, prgtt, emp_rec.org_code, wasok);
         if wasok = 'N' then
            trainrpt_util.LogMyMessage(mylogfile,'problem inserting prog code '||progcode||' '||jobcode,true,true);
            goto end_loop;
         end if;
      end setup_pc;

      <<emp_jobcd>>  /* attach job code to employee  */
      begin
         addblankempjob(emp_rec.emp_ssn_num, jobcode, wasok);
         if wasok = 'N' then
            trainrpt_util.LogMyMessage(mylogfile,'problem adding job '||jobcode||' to '||emp_rec.emp_ssn_num,true,true);
            goto end_loop;
         end if;
      end emp_jobcd;

      commit;

      <<end_loop>>
      errflg := 'N';
      wasok := 'Y';

   end loop;

   close get_emp;

end; -- build_employee_jobs

procedure emp_termination_process (p_emp_ssn_num in varchar2, p_cmp_alt_id in varchar2) is
/*----------------------------------------------------------------------*/
/* 5/15/96           check_emp_terminated.sql         Michael Johnston  */
/* this script will be used to make changes to personnel records        */
/* because they have retired or terminated                              */
/* 7/18/96  remove persons entry in train oracle accounts security      */
/* 2/25/98  set course_active_stat_flag to I for T and R emp stat       */
/* 12/10/98 remove entries from ejc and ejp tables                      */
/* 2/1/99   remove entries from train_accounts table                    */
/* 7/25/99  remove entries from emp_job if terminated                   */
/* 7/17/2000 remove entries from misty_rqr_trn and set ERP table to I   */
/* 8/14/2000 work from employee table instead of new_emp_update table   */
/* 8/15/2000 revise for addition table updates and deletions            */
/* 5/18/2001 remove special employee job codes                          */
/* 6/19/2001 remove class attendance if E and set ITP plan date to NULL */
/* rlo changes follow ---                                               */
/* 8/30/2002 added status of 'I' since that is what comes from ent. dir.*/
/* 9/05/2002 incorporated into emp_ldap_update process                  */
/*----------------------------------------------------------------------*/

   flag         varchar2(1);
   orgnum       varchar2(5);
   oldorg       varchar2(8);
   oldstat      varchar2(1);
   oldmail      varchar2(12);
   sql_err      varchar2(50);
   descript     varchar2(80);
   counter      number(8);
   mailadd      number(8);
   maildel      number(8);
   mailupd      number(8);
   trainact     number(8);
   found        varchar2(1);
   new_ssn      varchar2(9);
   old_user     varchar2(9);
   specjc       varchar2(6);

begin
   counter := 0;
   mailadd := 0;
   maildel := 0;
   mailupd := 0;
   trainact := 0;
   new_ssn := p_emp_ssn_num;
   counter := counter + 1;

   trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || p_emp_ssn_num || ' termination updates!',true,true);

   /* check to see if user has a user id in oracle_accounts table */
   <<need_user_id>>
   begin
      old_user := p_cmp_alt_id;
      if old_user is null then
      begin
         select account_user_id
            into old_user
            from oracle_accounts
            where ssn_num = new_ssn;

         if SQL%NOTFOUND then
            old_user := NULL;
         end if;

      exception
         when OTHERS then
         old_user := NULL;

      end;
      end if;
   end need_user_id;

   /* delete from train oracle accounts security if in table    */
   <<check_toas>>
   begin
      select 'Y'
         into flag
         from train_oracle_accounts_security
         where account_user_id = old_user
         and ROWNUM = 1;

      if SQL%FOUND then
      begin
         trainact := trainact + 1;
         delete
            from train_oracle_accounts_security
            where account_user_id = old_user;
         commit;
      end;
      end if;

   exception
      when NO_DATA_FOUND then
         found := 'N';
      when OTHERS then
         sql_err := substr(sqlerrm, 1, 50);
         descript := 'Unknown error security table, USER ID is - ' || old_user;
         trainrpt_util.LogMyMessage(mylogfile,descript || ' error ' || sql_err,true,true);

   end check_toas;

   /* delete from train_accounts table if in table    */
   <<chk_train_accounts>>
   begin
      select 'Y'
         into flag
         from train_accounts
         where account_user_id = old_user;

      if SQL%FOUND then
      begin
         delete
            from train_accounts
            where account_user_id = old_user;
         commit;
      end;
      end if;

   exception
      when NO_DATA_FOUND then
         found := 'N';
      when OTHERS then
         sql_err := substr(sqlerrm, 1, 50);
         descript := 'Unknown error train_accounts, primary key is - ' || new_ssn;
         trainrpt_util.LogMyMessage(mylogfile,descript || ' error ' || sql_err,true,true);

   end chk_train_accounts;
   /* delete from oracle accounts table if in table  */
   <<chk_mail_user>>
   begin
      select 'Y'
         into flag
         from oracle_accounts
         where ssn_num = new_ssn
         and ROWNUM = 1;

      if SQL%FOUND then
      begin
         maildel := maildel + 1;
         delete
            from oracle_accounts
            where ssn_num = new_ssn;
         commit;
      end;
      end if;

   exception
      when NO_DATA_FOUND then
         found := 'N';
      when OTHERS then
         sql_err := substr(sqlerrm, 1, 50);
         descript := 'Unknown error, primary key is - ' || new_ssn;
         trainrpt_util.LogMyMessage(mylogfile,descript || ' error ' || sql_err,true,true);

   end chk_mail_user;
   /*  update emp_job table, remove entries termination  */
   <<chk_emp_job>>
   begin
      select 'Y'
         into flag
         from emp_job
         where emp_ssn_num = new_ssn
         and ROWNUM = 1;

      if SQL%FOUND then
      begin
         delete
            from emp_job
            where emp_ssn_num = new_ssn;
         commit;
      end;
      end if;

   exception
      when OTHERS then
         flag := 'N';

   end chk_emp_job;
   /*  update itp table, set status to I  */
   <<chk_emp_itp>>
   begin
      update indiv_train_plan
         set course_active_stat_flag = 'I'
         where emp_ssn_num = new_ssn
         and course_active_stat_flag <> 'I';
      commit;
   end chk_emp_itp;
   /* delete from ITP table if never completed */
   <<chk_emp_itp>>
   begin
      delete
         from indiv_train_plan
         where emp_ssn_num = new_ssn
         and complete_date is NULL;
      commit;
   end chk_emp_itp;
   /* update emp_qlf table, set active to inactive */
   <<chk_emp_qlf>>
   begin
      update emp_qlf
         set qlf_status_flag = 'I'
         where emp_ssn_num = new_ssn
         and qlf_status_flag <> 'I';
      commit;
   end chk_emp_qlf;
   /* delete from class attend table if E or W */
   <<chk_del_ca>>
   begin
      delete
         from class_attend
         where emp_ssn_num = new_ssn
         and class_attend_status_code in ('E','W');
      commit;
   end chk_del_ca;
   /* delete from course_waiting_list table */
   <<chk_del_cwl>>
   begin
      delete
         from course_waiting_list
         where emp_ssn_num = new_ssn;
   commit;
   end chk_del_cwl;
   /* update emp_job_prog table, remove entries  */
   <<chk_emp_job_prog>>
   begin
      select 'Y'
         into flag
         from emp_job_prog
         where emp_ssn_num = new_ssn
         and ROWNUM = 1;

      if SQL%FOUND then
      begin
         delete
            from emp_job_prog
            where emp_ssn_num = new_ssn;
         commit;
      end;
      end if;

   exception
      when NO_DATA_FOUND then
         found := 'N';
      when OTHERS then
      found := 'N';

   end chk_emp_job_prog;
   /*  update emp_job_course table, remove entries  */
   <<chk_emp_job_course>>
   begin
      select 'Y' into flag from emp_job_course
      where emp_ssn_num = new_ssn and ROWNUM = 1;

      if SQL%FOUND then
      begin
         delete
            from emp_job_course
            where emp_ssn_num = new_ssn;
         commit;
      end;
      end if;

   exception
      when NO_DATA_FOUND then
         found := 'N';
      when OTHERS then
         found := 'N';

   end chk_emp_job_course;

   /* remove MISTY_RQR_TRN entries  */
   <<chk_misty>>
   begin
      select 'Y'
         into flag
         from misty_rqr_trn
         where emp_ssn_num = new_ssn
         and ROWNUM = 1;

      if SQL%FOUND then
      begin
         delete
            from misty_rqr_trn
            where emp_ssn_num = new_ssn;
         commit;
      end;
      end if;

   exception
      when NO_DATA_FOUND then
         found := 'N';
      when OTHERS then
         found := 'N';

   end chk_misty;
   /* set ERP entries to 'I'  */
   <<ck_ERP>>
   begin
      select 'Y'
         into flag
         from emp_reading_plan
         where emp_ssn_num = new_ssn
         and act_stat_flag = 'A'
         and ROWNUM = 1;

      if SQL%FOUND then
      begin
         update emp_reading_plan
            set act_stat_flag = 'I'
            where emp_ssn_num = new_ssn;
         commit;
      end;
      end if;

   exception
      when NO_DATA_FOUND then
         found := 'N';
      when OTHERS then
         found := 'N';
   end ck_ERP;

      /* delete ERP entries that were never completed */
   <<del_erp>>
   begin
      select 'Y'
         into flag
         from emp_reading_plan
         where emp_ssn_num = new_ssn
         and cmpl_date is NULL
         and last_read is NULL
         and ROWNUM = 1;

      if SQL%FOUND then
      begin
         delete
            from emp_reading_plan
            where emp_ssn_num = new_ssn
            and cmpl_date is NULL
            and last_read is NULL
            and act_stat_flag = 'I';
         commit;
      end;
      end if;

   exception
      when NO_DATA_FOUND then
         found := 'N';
      when OTHERS then
         found := 'N';

   end del_erp;

   /* remove special employee job code
   from the system if one exists     */
   <<del_spec_jc>>
   begin
      select job_code into specjc from job
      where job_title = 'PERSONAL JOB CODE FOR '||SUBSTR(new_ssn,4,6);

      if SQL%FOUND then
      begin
         begin /* delete any links to the special program */
            delete
               from job_prog
               where job_code = specjc;
         end;
         begin /* courses attached to program */
            delete
               from prog_course
               where prog_code = substr(specjc,3,4);
         end;
         begin /* readings attached to program */
            delete
               from prog_reading
               where prog_code = substr(specjc,3,4);
         end;
         begin /* qualifications attached to program */
            delete
               from prog_qlf
               where prog_code = substr(specjc,3,4);
         end;
         begin /* program to be deleted */
            delete
               from program
               where prog_code = substr(specjc,3,4)
               and prog_title = 'PERSONAL PRG FOR '||SUBSTR(new_ssn,4,6)||' '||specjc;
         end;
         begin /* qualifications attached to the job */
            delete
               from job_qlf
               where job_code = specjc;
         end;
         begin /* now delete special job code */
            delete
               from job
               where job_code = specjc;
         end;
         begin
            delete
               from job_code
               where job_code = specjc;
         end;
         commit;
      end;
      end if;

   exception
      when others then
         NULL;

   end del_spec_jc;

   <<itp_pd>> /* set ITP planned dates to NULL */
   begin
      update indiv_train_plan
         set planned_date = NULL
         where planned_date is not null
         and emp_ssn_num = new_ssn;
   end itp_pd;
   commit;
   <<end_loop>>

   NULL;
   commit;

end; -- emp_termination_process


procedure MailMessage(sender in varchar2,
                      recipient in varchar2,
                      message in varchar2) is

   mailhost       VARCHAR2 (30) := 'mail2.inel.gov';
   v_connection   UTL_SMTP.CONNECTION;
BEGIN

   v_connection := UTL_SMTP.OPEN_CONNECTION('mail2.inel.gov',25);
   UTL_SMTP.HELO(v_connection,'mail2.inel.gov');
   UTL_SMTP.MAIL(v_connection,sender);
   UTL_SMTP.RCPT(v_connection,recipient);
   UTL_SMTP.DATA(v_connection,message);
   UTL_SMTP.QUIT(v_connection);

exception
   when others then
      trainrpt_util.LogMyMessage(mylogfile,' ***** Error sending mail message',true,true);
      trainrpt_util.LogMyMessage(mylogfile,' ***** Error(Cont.) - Sender is ' || sender,true,true);
      trainrpt_util.LogMyMessage(mylogfile,' ***** Error(Cont.) - Recipient is ' || recipient,true,true);
      trainrpt_util.LogMyMessage(mylogfile,' ***** Error(Cont.) - Message is ' || message,true,true);
      trainrpt_util.LogMyMessage(mylogfile,' ***** Error(Cont.) - Error is ' || SQLERRM,true,true);

end;

PROCEDURE Update_Emp_Table is

   cursor Get_ldap_data is
      select *
        from ldap_emp
       order by empnum;

   cursor Get_emp_data (p_emp_ssn_num in varchar2) is
      select *
        from employee
       where emp_ssn_num=lpad(p_emp_ssn_num,9,'0');

   ckcount pls_integer;
   emprec  train.employee%rowtype;
   ldaprec train.ldap_emp%rowtype;
   doupdate boolean;
   statupdate boolean;

   trainhomeorg train.org.org_code%type;
   trainworkorg train.org.org_code%type;

   realhomeorg train.org.old_org_code%type;
   realworkorg train.org.old_org_code%type;

   do_term_updates boolean;
   do_newemp_updates boolean;
   do_toas_update boolean;
   min_org_code pls_integer;

   padded_empnum varchar2(20);
   ssn_value pls_integer;

begin

   trainrpt_util.ClearMyMessageFile(mylogfile);
   trainrpt_util.LogMyMessage(mylogfile,'Employee processing started at ' || to_char(sysdate,'mm/dd/yyyy hh:mi:ss'),true,true);

   open get_ldap_data;

   loop
      fetch get_ldap_data into ldaprec;
      if get_ldap_data%NOTFOUND then
         exit;
      end if;

      declare
      begin
         ssn_value := to_number(ldaprec.ssn_num);
      exception
         when others then
            ssn_value := 0;
      end;

      -- we only load them from the enterprise directory if they have something in the ssn field
      if nvl(ssn_value,0) > 0 then

         padded_empnum := lpad(ldaprec.empnum,9,'0');

         if (ldaprec.homeorg = '6302') then
            declare
            begin
               select org_code
                 into trainhomeorg
                 from org
                where old_org_code = ldaprec.homeorg;

               realhomeorg := ldaprec.homeorg;
               ldaprec.homeorg := trainhomeorg;

            exception
               when NO_DATA_FOUND then
                  realhomeorg := 99157;
                  ldaprec.homeorg := 99157;
                  ldaprec.workorg := '';
                  realworkorg := '';
               when others then
                  if length(ldaprec.homeorg) > 0 then
                     trainrpt_util.LogMyMessage(mylogfile,'Error finding TRAIN org code for home org ' || ldaprec.homeorg || ' employee ' || padded_empnum,true,true);
                     trainrpt_util.LogMyMessage(mylogfile,sqlerrm,true,true);
                     MailMessage(developer,developer,'Error (1) finding TRAIN org code for ' || padded_empnum || ' - home org ' || ldaprec.homeorg || ' ' || sqlerrm);
                     goto next_record;
                  end if;
            end;

            declare
            begin
               select org_code
                 into trainworkorg
                 from org
                where old_org_code = ldaprec.workorg;

               realworkorg := ldaprec.workorg;
               ldaprec.workorg := trainworkorg;

            exception
               when NO_DATA_FOUND then
                  realworkorg := '';
                  ldaprec.workorg := '';
               when others then
                  if length(ldaprec.workorg) > 0 then
                     trainrpt_util.LogMyMessage(mylogfile,'Error finding TRAIN org code for work org ' || ldaprec.workorg || ' employee ' || padded_empnum,true,true);
                     trainrpt_util.LogMyMessage(mylogfile,sqlerrm,true,true);
                     MailMessage(developer,developer,'Error (2) finding TRAIN org code for ' || padded_empnum || ' - work org ' || ldaprec.homeorg || ' ' || sqlerrm);
                     goto next_record;
                  end if;
            end;

         else
            if (ldaprec.companycode = 2115) then

               declare
               begin
                  select org_code
                    into trainhomeorg
                    from org
                   where old_org_code = ldaprec.homeorg;

                  realhomeorg := ldaprec.homeorg;
                  ldaprec.homeorg := trainhomeorg;

               exception
                  when NO_DATA_FOUND then
                     realhomeorg := 99157;
                     ldaprec.homeorg := 99157;
                     ldaprec.workorg := '';
                     realworkorg := '';
                  when others then
                     if length(ldaprec.homeorg) > 0 then
                        trainrpt_util.LogMyMessage(mylogfile,'Error finding TRAIN org code for home org ' || ldaprec.homeorg || ' employee ' || padded_empnum,true,true);
                        trainrpt_util.LogMyMessage(mylogfile,sqlerrm,true,true);
                        MailMessage(developer,developer,'Error (1) finding TRAIN org code for ' || padded_empnum || ' - home org ' || ldaprec.homeorg || ' ' || sqlerrm);
                        goto next_record;
                     end if;
               end;

               declare
               begin
                  select org_code
                    into trainworkorg
                    from org
                   where old_org_code = ldaprec.workorg;

                  realworkorg := ldaprec.workorg;
                  ldaprec.workorg := trainworkorg;

               exception
                  when NO_DATA_FOUND then
                     realworkorg := '';
                     ldaprec.workorg := '';
                  when others then
                     if length(ldaprec.workorg) > 0 then
                        trainrpt_util.LogMyMessage(mylogfile,'Error finding TRAIN org code for work org ' || ldaprec.workorg || ' employee ' || padded_empnum,true,true);
                        trainrpt_util.LogMyMessage(mylogfile,sqlerrm,true,true);
                        MailMessage(developer,developer,'Error (2) finding TRAIN org code for ' || padded_empnum || ' - work org ' || ldaprec.homeorg || ' ' || sqlerrm);
                        goto next_record;
                     end if;
               end;

            else

               /* get the home org in TRAIN format */

               if (ldaprec.orgstatus='A') then
                  if nvl(ldaprec.homeorg,'xxxx') != 'xxxx' then
                     -- we likely have a matrixed employee from different company
                     -- so move it to the work org field.
                     declare
                     begin
                        select org_code
                           into trainworkorg
                           from org
                           where old_org_code = ldaprec.homeorg;

                        ldaprec.workorg := trainworkorg;
                        ldaprec.homeorg := null;

                     exception
                        when NO_DATA_FOUND then

                           trainrpt_util.LogMyMessage(mylogfile,'Error finding TRAIN work org code for ' || ldaprec.homeorg || ' for non BBWI employee ' || padded_empnum,true,true);
                           trainrpt_util.LogMyMessage(mylogfile,sqlerrm,true,true);
                           MailMessage(developer,developer,'Error (3) finding TRAIN org code for ' || padded_empnum || ' - home org ' || ldaprec.homeorg || ' for non BBWI employee');
                     end;

                  end if;

                  if nvl(ldaprec.companycode,'xxxx') != 'xxxx' then
                     declare
                     begin
                        select org_code
                           into trainhomeorg
                           from org
                           where old_org_code = 'IN' || ldaprec.companycode;

                        ldaprec.homeorg := trainhomeorg;
                        ldaprec.workorg := '';

                     exception
                        when NO_DATA_FOUND then

                           select count(*)
                             into ckcount
                             from company
                            where co_code = ldaprec.companycode;

                           if ckcount = 0 then
                              trainrpt_util.LogMyMessage(mylogfile,'Error (4)  ' || ldaprec.companycode || ' ' || ldaprec.companyname || ' employee ' || padded_empnum,true,true);
                              ldaprec.companycode := 9999; /* unknown company */

                           end if;

                           trainrpt_util.LogMyMessage(mylogfile,'Error finding TRAIN home org code for IN' || ldaprec.companycode || ' employee ' || padded_empnum,true,true);
                           trainrpt_util.LogMyMessage(mylogfile,sqlerrm,true,true);
                           MailMessage(developer,developer,'Error (5) finding TRAIN org code for IN' || ldaprec.companycode || ' employee ' || padded_empnum);
                     end;
                  else
                     ldaprec.companycode := 9999; /* unknown company */
                  end if;
               else
                  ldaprec.homeorg := 99233; /* unknown org */
                  ldaprec.workorg := null;
               end if;

            end if;
         end if;

         if length(ldaprec.telephone) > 0 then
            ldaprec.telephone := substr(ldaprec.telephone,1,3) || substr(ldaprec.telephone,5,3) || substr(ldaprec.telephone,9,4);
         end if;

         IF (ldaprec.termination_date = '0000000000') or (length(ltrim(ldaprec.termination_date)) = 0) THEN
            ldaprec.termination_date := NULL;
         END IF;

         IF length(ldaprec.ssn_num) = 0  THEN
            ldaprec.ssn_num := '000000000';
         END IF;

         open get_emp_data(padded_empnum);
         fetch get_emp_data into emprec;
         if get_emp_data%NOTFOUND then
            -- the employee does not exist in the employee table
            -- insert them

            DECLARE
            BEGIN
               insert into employee (emp_ssn_num,
                                     emp_last_name,
                                     emp_first_name,
                                     emp_mid_name,
                                     emp_suff_name,
                                     org_code,
                                     job_code,
                                     roll_code,
                                     emp_auth_lvl_code,
                                     emp_shft_code,
                                     emp_bir_date,
                                     emp_hcap_flag,
                                     emp_vet_flag,
                                     emp_race_code,
                                     emp_sex_code,
                                     emp_work_tel_num,
                                     area_code,
                                     bld_code,
                                     rm_num,
                                     eeo_job_grp_code,
                                     eeo_job_cat_code,
                                     emp_active_stat_flag,
                                     emp_term_date,
                                     emp_adj_serv_date,
                                     emp_orig_hire_date,
                                     unit_code,
                                     asgmt_serv_unit_start_date,
                                     group_code,
                                     asgmt_serv_grp_start_date,
                                     update_uid,
                                     update_date,
                                     co_code,
                                     facility_code,
                                     org_work_id,
                                     cmp_alt_id,
                                     fund_org_id,
                                     reqd_train_org_id,
                                     ssn_num,
                                     clear_level,
                                     work_disc,
                                     manager_level_code,
                                     craft_level,
                                     mail_stop)
                     VALUES ( padded_empnum,
                              ldaprec.lastname,
                              substr(ldaprec.givenname,1,12),
                              ldaprec.middlename,
                              ldaprec.suffixname,
                              nvl(ldaprec.homeorg,99233),
                              ldaprec.job_class_code,
                              NULL,
                              NULL,
                              ldaprec.shiftcode,
                              to_date(ldaprec.birthdate,'mm/dd/yyyy'),
                              ldaprec.disability_flag,
                              NULL,
                              NULL,
                              ldaprec.gender,
                              ldaprec.telephone,
                              substr(ldaprec.site_area,1,2),
                              ldaprec.building_name,
                              ldaprec.room_number,
                              NULL,
                              NULL,
                              ldaprec.orgstatus,
                              to_date(ldaprec.termination_date,'mm/dd/yyyy'),
                              NULL,
                              null,
                              null,
                              null,
                              null,
                              null,
                              'ENTDIR',
                              SYSDATE,
                              nvl(ldaprec.companycode,9999),
                              NULL,
                              nvl(ldaprec.workorg,99233),
                              ldaprec.mail_user_id,
                              NULL,
                              NULL,
                              lpad(ldaprec.ssn_num,9,'0'),
                              ldaprec.clearance_code,
                              rpad(ldaprec.workdisp,6,'0'),
                              ldaprec.officer_code,
                              NULL,
                              ldaprec.ms);

               commit;

               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' inserted!',true,true);
               -- do the new employee processing

               if ldaprec.orgstatus = 'A' then

                  if (nvl(ldaprec.mail_user_id,'isnull') != 'isnull') and (nvl(ldaprec.homeorg,'99233') != '99233') then
                     oa_update(ldaprec.mail_user_id, ldaprec.homeorg, padded_empnum);
                  end if;

                  trainrpt_util.LogMyMessage(mylogfile,'New employee processing for ' || padded_empnum,true,true);
                  build_employee_jobs(padded_empnum);

                  if (ldaprec.companycode = '2115') or (ldaprec.companycode='1214') then
                     if realhomeorg != constructionorg then
                        trainrpt_util.LogMyMessage(mylogfile,'Construction processing for ' || padded_empnum,true,true);
                        add_job_jc0001 (padded_empnum);
                        add_jc0001_readings(padded_empnum);
                        add_job_courses(padded_empnum,'JC0001');
                     elsif realhomeorg = constructionorg then
                        trainrpt_util.LogMyMessage(mylogfile,'Non-construction processing for ' || padded_empnum,true,true);
                        add_job_inelcn(padded_empnum);
                        add_inelcn_readings(padded_empnum);
                        add_job_courses(padded_empnum,'INELCN');
                     end if;
                  end if;

               end if;

            EXCEPTION
               WHEN OTHERS THEN
                  trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' insert failed with error ' || SQLERRM,true,true);
                  goto next_record;
            END;

         else

            -- now see if anything in the record has changed.
            doupdate  := false;
            do_term_updates := false;
            do_newemp_updates := false;
            do_toas_update := false;

            if (upper(rtrim(ldaprec.givenName)) != rtrim(emprec.emp_first_name)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' first name needs to be updated '||upper(rtrim(ldaprec.givenName)) || ' ' || emprec.emp_first_name,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.lastname)) != rtrim(emprec.emp_last_name)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' last name needs to be updated '||upper(rtrim(ldaprec.lastname)) || ' ' || emprec.emp_last_name,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(substr(ldaprec.middlename,1,12))) != rtrim(emprec.emp_mid_name)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' middle name needs to be updated '||upper(rtrim(ldaprec.middlename)) || ' ' || emprec.emp_mid_name,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.suffixname)) != rtrim(emprec.emp_suff_name)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' suffix name needs to be updated '||upper(rtrim(ldaprec.suffixname)) || ' ' || emprec.emp_suff_name,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.companycode)) != rtrim(emprec.co_code)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' company code needs to be updated '||upper(rtrim(ldaprec.companycode)) || ' ' || emprec.co_code,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.orgstatus)) != rtrim(emprec.emp_active_stat_flag)) then
               statupdate := false;
               if emprec.emp_active_stat_flag = 'U' then
                  if ldaprec.orgstatus != 'A' then
                     ldaprec.orgstatus := emprec.emp_active_stat_flag;
                     do_newemp_updates := true;
                     statupdate := true;
                     doupdate  := true;
                  else
                     ldaprec.orgstatus := emprec.emp_active_stat_flag;
                  end if;
               elsif emprec.emp_active_stat_flag = 'A' then
                  if ldaprec.orgstatus != 'L' then
                     do_term_updates := true;
                     statupdate := true;
                     doupdate  := true;
                  end if;
               elsif emprec.emp_active_stat_flag not in ('A','L') then
                  if ldaprec.orgstatus = 'A' then
                     do_newemp_updates := true;
                     statupdate := true;
                     doupdate  := true;
                  end if;
               end if;
               if statupdate then
                  trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' status changed '||upper(rtrim(ldaprec.orgstatus)) || ' ' || emprec.emp_active_stat_flag,true,true);
               end if;
            end if;
            if (upper(rtrim(ldaprec.gender)) != rtrim(nvl(emprec.emp_sex_code,'*'))) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' gender needs to be updated '||upper(rtrim(ldaprec.gender)) || ' ' || emprec.emp_sex_code,true,true);
               doupdate  := true;
            end if;
            if (lpad(rtrim(ldaprec.ssn_num),9,'0') != rtrim(emprec.ssn_num)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' ssn needs to be updated '||lpad(rtrim(ldaprec.ssn_num),9,'0') || ' ' || emprec.ssn_num,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.homeorg)) != rtrim(emprec.org_code)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' org_code needs to be updated '||upper(rtrim(ldaprec.homeorg)) || ' ' || emprec.org_code,true,true);
               do_toas_update := true;
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.workorg)) != rtrim(emprec.org_work_id)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' work org needs to be updated '||upper(rtrim(ldaprec.workorg)) || ' ' || emprec.org_work_id,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.shiftcode)) != rtrim(emprec.emp_shft_code)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' shift_code needs to be updated '||upper(rtrim(ldaprec.shiftcode)) || ' ' || emprec.emp_shft_code,true,true);
               doupdate  := true;
            end if;
            if (rpad(ldaprec.workdisp,6,'0') != rtrim(emprec.work_disc)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' work_disc code needs to be updated '||upper(rtrim(ldaprec.workdisp)) || ' ' || emprec.work_disc,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.officer_code)) != rtrim(emprec.manager_level_code)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' manager_level needs to be updated '||upper(rtrim(ldaprec.officer_code)) || ' ' || emprec.manager_level_code,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.job_class_code)) != rtrim(emprec.job_code)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' job_class_code needs to be updated '||upper(rtrim(ldaprec.job_class_code)) || ' ' || emprec.job_code,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.ms)) != rtrim(emprec.mail_stop)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' mail stop needs to be updated '||upper(rtrim(ldaprec.ms)) || ' ' || emprec.mail_stop,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.telephone)) != rtrim(emprec.emp_work_tel_num)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' work phone number needs to be updated '||upper(rtrim(ldaprec.telephone)) || ' ' || emprec.emp_work_tel_num,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.mail_user_id)) != rtrim(emprec.cmp_alt_id)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' mail user id needs to be updated '||upper(rtrim(ldaprec.mail_user_id)) || ' ' || emprec.cmp_alt_id,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(substr(ldaprec.site_area,1,2))) != rtrim(emprec.area_code)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' area code needs to be updated '||upper(rtrim(substr(ldaprec.site_area,1,2))) || ' ' || emprec.area_code,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.building_name)) != rtrim(emprec.bld_code)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' building code needs to be updated '||upper(rtrim(ldaprec.building_name)) || ' ' || emprec.bld_code,true,true);
               doupdate := true;
            end if;
            if (upper(rtrim(ldaprec.room_number)) != rtrim(emprec.rm_num)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' room number needs to be updated '||upper(rtrim(ldaprec.room_number)) || ' ' || emprec.rm_num,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.clearance_code)) != rtrim(emprec.clear_level)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' security clearance code '||upper(rtrim(ldaprec.clearance_code)) || ' ' || emprec.clear_level,true,true);
               doupdate  := true;
            end if;
            if (upper(rtrim(ldaprec.disability_flag)) != rtrim(emprec.emp_hcap_flag)) then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' employee handicap flag '||upper(rtrim(ldaprec.disability_flag)) || ' ' || emprec.emp_hcap_flag,true,true);
               doupdate  := true;
            end if;
            if (rtrim(ldaprec.birthdate)) != to_char(emprec.emp_bir_date,'mm/dd/yyyy') then
               trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || padded_empnum || ' birthdate updated '|| rtrim(ldaprec.birthdate) || ' ' || to_char(emprec.emp_bir_date,'mm/dd/yyyy'),true,true);
               doupdate  := true;
            end if;

            if  doupdate then

               declare
               begin

                  update employee
                     set emp_first_name = upper(ldaprec.givenname),
                         emp_last_name = upper(ldaprec.lastname),
                         emp_mid_name = upper(substr(ldaprec.middlename,1,12)),
                         emp_suff_name = upper(substr(ldaprec.suffixname,1,3)),
                         co_code = nvl(ldaprec.companycode,9999),
                         emp_active_stat_flag = ldaprec.orgstatus,
                         emp_sex_code = decode(ldaprec.gender,'U',emprec.emp_sex_code,ldaprec.gender),
                         ssn_num = lpad(ldaprec.ssn_num,9,'0'),
                         org_code = nvl(ldaprec.homeorg,99249),
                         emp_shft_code = ldaprec.shiftcode,
                         work_disc = rpad(ldaprec.workdisp,6,'0'),
                         org_work_id = nvl(ldaprec.workorg,99249),
                         manager_level_code = ldaprec.officer_code,
                         mail_stop = ldaprec.ms,
                         emp_work_tel_num = ldaprec.telephone,
                         cmp_alt_id = ldaprec.mail_user_id,
                         area_code = substr(ldaprec.site_area,1,2),
                         bld_code = ldaprec.building_name,
                         job_code = ldaprec.job_class_code,
                         rm_num = ldaprec.room_number,
                         clear_level = ldaprec.clearance_code,
                         emp_hcap_flag = ldaprec.disability_flag,
                         emp_bir_date = to_date(ldaprec.birthdate,'mm/dd/yyyy'),
                         update_date = sysdate,
                         update_uid = 'ENTDIR'
                   where emp_ssn_num = emprec.emp_ssn_num;

                   commit;

                   trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || emprec.emp_ssn_num || ' updated!',true,true);
                   if do_term_updates then
                     trainrpt_util.LogMyMessage(mylogfile,'Termination processing for employee  ' || padded_empnum,true,true);
                     emp_termination_process(emprec.emp_ssn_num,emprec.cmp_alt_id);
                   end if;
                   if do_newemp_updates then
                     trainrpt_util.LogMyMessage(mylogfile,'New employee processing for ' || padded_empnum,true,true);

                     if (nvl(ldaprec.mail_user_id,'isnull') != 'isnull') and (nvl(ldaprec.homeorg,'99233') != '99233') then
                        oa_update(ldaprec.mail_user_id, ldaprec.homeorg, padded_empnum);
                     end if;

                     build_employee_jobs(padded_empnum);

                     if (ldaprec.companycode = '2115') or (ldaprec.companycode='1214') then
                        if realhomeorg != constructionorg then
                           trainrpt_util.LogMyMessage(mylogfile,'Non construction processing for ' || padded_empnum,true,true);
                           add_job_jc0001 (padded_empnum);
                           add_jc0001_readings(padded_empnum);
                           add_job_courses(padded_empnum,'JC0001');
                        elsif realhomeorg = constructionorg then
                           trainrpt_util.LogMyMessage(mylogfile,'Construction processing for ' || padded_empnum,true,true);
                           add_job_inelcn(padded_empnum);
                           add_inelcn_readings(padded_empnum);
                           add_job_courses(padded_empnum,'INELCN');
                        end if;
                     end if;
                   end if;

                   if (ldaprec.companycode = '2115') and (ldaprec.orgstatus='A') and (do_toas_update) then
                      toas_update(padded_empnum,emprec.org_code,trainhomeorg,ldaprec.mail_user_id);
                   end if;

               exception
                  when others then
                     trainrpt_util.LogMyMessage(mylogfile,'Employee  ' || emprec.emp_ssn_num || ' update failed with error ' || SQLERRM,true,true);
                     goto next_record;

               end;

            end if;

         end if;

         <<next_record>>
         if get_emp_data%ISOPEN then
            close get_emp_data;
         end if;
      end if;

   end loop;
   trainrpt_util.LogMyMessage(mylogfile,'Employee processing completed at ' || to_char(sysdate,'mm/dd/yyyy hh:mi:ss'),true,true);

   close get_ldap_data;

exception
    WHEN OTHERS THEN
        trainrpt_util.LogMyMessage(mylogfile,'Error ' || SQLERRM || ' occurred while processing the LDAP_EMP table ',true,true);
        trainrpt_util.LogMyMessage(mylogfile,'Error (cont.) ldaprec.empnum is ' || ldaprec.empnum,true,true);

end;

end emp_ldap_update;
/
SET termout ON
spool emp_ldap_update.err
show errors
spool off
SELECT to_char(sysdate,'HH:MI:SS') FROM dual;
/


