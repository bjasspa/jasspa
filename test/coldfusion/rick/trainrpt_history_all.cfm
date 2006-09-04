<cfif REDIRECTREPORTS>
  <cflocation url="#reports_host#train_reports.cfm" />
</cfif>
<cfif NOT AllowReports>
  <cflocation url="no_reports.htm" />
</cfif>
<cfset whoami="trainrpt_history_all.cfm" />
<cfset title="Employee Training History" />
<cfset page_title="Employee Training History" />
<html>
  <head>
    
    <cfparam name="option" default="input" />
    <cfparam name="ckmatrix" default="no" />
    <cfparam name="searchtype" default="simple" />
    <cfparam name="contract_id" default="L" />
    
    <cfif NOT CompareNoCase(option,"input")>
      
      <cfparam name="def_bgcolor" default="##d2dcdc" />
      <cfinclude template="trainrpt_initial_queries_inc.cfm" />
      
      <SCRIPT LANGUAGE="javascript">
        <!--
        <cfset allowhier=true>
        <cfinclude template="trainrpt_simple_js_inc.cfm">
        function checkafewmorethings(myform)
        {
          var ckcount = 0;
          ckcount = (myform.histall.checked      ? ++ckcount : ckcount);
          ckcount = (myform.histcourse.checked   ? ++ckcount : ckcount);
          ckcount = (myform.histqual.checked     ? ++ckcount : ckcount);
          ckcount = (myform.histread.checked     ? ++ckcount : ckcount);
          ckcount = (myform.histadmin.checked    ? ++ckcount : ckcount);
          if (ckcount == 0)
          {
            alert('You must select at least one Report Types (i.e. Course, Qual, Reading, Admin Form, or All)');
            clickcount = 0;
            return false;
          }
        }
        function manage_clicks(myform,myfield)
        {
          if (myfield.name == 'histall')
          {
            if (myfield.checked)
            {
              myform.histcourse.checked = false;
              myform.histqual.checked = false;
              myform.histread.checked = false;
              myform.histadmin.checked = false;
            }
          }
          var ckcount = 0;
          ckcount = (myform.histcourse.checked   ? ++ckcount : ckcount);
          ckcount = (myform.histqual.checked     ? ++ckcount : ckcount);
          ckcount = (myform.histread.checked     ? ++ckcount : ckcount);
          ckcount = (myform.histadmin.checked    ? ++ckcount : ckcount);
          if (ckcount == 4)
          {
            myform.histall.checked = true;
            myform.histcourse.checked = false;
            myform.histqual.checked = false;
            myform.histread.checked = false;
            myform.histadmin.checked = false;
          } else {
            if (ckcount > 0)
            {
              myform.histall.checked = false;
            }
          }
        }
        
        //-->
      </SCRIPT>
    </head>
    <body bgcolor="white" onload="clickcount=0;setfocus(document.myform.report_emp);">
    
    
    <cfset style_context='screen' />
    <cfinclude template="trainrpt_header_inc.cfm" />
    
    <hr width="25%" noshade size="1">
    
    <div align="center">
    <form action="<cfoutput>#whoami#</cfoutput>" method="post" onSubmit="return verify(this) && checkafewmorethings(this);" name="myform">
    <table width="625" border="1" cellspacing="0"><tr><td align="center">
          
          <cfset def_anyemp=false />
          <cfset def_snumber=true />
          <cfset def_arealist=true />
          <cfset def_contractors=false />
          <cfinclude template="trainrpt_select_who_inc.cfm">
          
          <cfset def_searchdate=true />
          <cfset def_end_date=DateFormat(CreateDate(DatePart("yyyy",Now())+1,12,31),"mm/dd/yyyy") />
          <cfoutput>
            <cfinclude template="trainrpt_search_when_inc.cfm" />
          </cfoutput>
          
          <tr>
            <td align="right">
              <b>Report Type:</b>&nbsp;&nbsp;
            </td><td colspan="2" align="left">
              <input type="checkbox" name="histall" value="true" checked onclick="manage_clicks(this.form,this);"> All
              <input type="checkbox" name="histcourse" value="true" onclick="manage_clicks(this.form,this);"> Course
              <input type="checkbox" name="histqual" value="true" onclick="manage_clicks(this.form,this);"> Qual (<input type="radio" name="whichquals" value="all"> <b>All</b> <input type="radio" name="whichquals" value="current" checked> <b>Current</b>)
              <input type="checkbox" name="histread" value="true" onclick="manage_clicks(this.form,this);"> Reading<br>
              <input type="checkbox" name="histadmin" value="true" onclick="manage_clicks(this.form,this);"> Admin Form
            </td>
          </tr>
          <cfinclude template="trainrpt_sort_by_inc.cfm">
        </td></tr></table>
    <br>
    <br>
    <input type="hidden" name="option" value="report">
    <input type="hidden" name="contract_id" value="<cfoutput>#contract_id#</cfoutput>">
    <input type="hidden" name="RequestTimeout" value="120">
    <input type="submit" value="Search">
    <input type="reset">
    </form>
    </div>
    </div>
    <cfinclude template="trainrpt_footer_link_inc.cfm">
  <cfelseif NOT CompareNoCase(option,"report")>
    
    <cfset style_context="report" />
    <cfinclude template="trainrpt_header_inc.cfm" />
    
    <cfinclude template = "trainrpt_defparm_inc.cfm" />
    
    <cfset awhileago = DateAdd("y",-90,Now()) />
    
    <cfparam name="admin_code" default="" />
    <cfparam name="area_code" default="" />
    <cfparam name="orgqry" default="" />
    <cfparam name="search_admin" default="" />
    <cfparam name="search_area" default="" />
    <cfparam name="search_dates" default="" />
    <cfparam name="search_emp" default="" />
    <cfparam name="search_org" default="" />
    <cfparam name="search_wdc" default="" />
    <cfparam name="searchtype" default="complex" />
    <cfparam name="searchwhat" default="emp" />
    <cfparam name="sort_course" default="" />
    <cfparam name="sort_date" default="" />
    <cfparam name="sort_date_order" default="asc" />
    <cfparam name="sort_emp" default="" />
    <cfparam name="sort_lname" default="" />
    <cfparam name="sort_org" default="" />
    <cfparam name="start_date" default=Now() />
    <cfparam name="work_code" default="" />
    <cfparam name="histall" default="false" />
    <cfparam name="histcourse" default="false" />
    <cfparam name="histqual" default="false" />
    <cfparam name="histread" default="false" />
    <cfparam name="histadmin" default="false" />
    
    <cfset ignore_active_flag=true />
    <cfinclude template="trainrpt_prescreen_inc.cfm" />
    
    <cfinclude template="trainrpt_xlate_simple_inc.cfm" />
    
    <cfif NOT CompareNoCase(searchwhat,"emp")>
      <cfset search_emp = "yes" />
    </cfif>
    
    <cfif NOT CompareNoCase(searchtype,'simple')>
      <cfset search_dates = 'yes' />
      <cfset start_dt = ParseDateTime(xstart_date) />
      <cfset end_dt   = ParseDateTime(xend_date) />
    <cfelse>
      <cfif NOT CompareNoCase(searchwhat,'emp')>
        <cfset search_dates = 'yes' />
        <cfif NOT CompareNoCase(dateRange,'all')>
          <cfset start_dt = ParseDateTime('01/01/1950') />
          <cfset end_dt = ParseDateTime(DateFormat(now(),'MM/DD/YYYY')) />
        <cfelse>
          <cfset start_dt = ParseDateTime(since_date) />
          <cfset end_dt = ParseDateTime(DateFormat(now(),'MM/DD/YYYY')) />
        </cfif>
      <cfelse>
        <cfif IsDefined("form.search_dates")>
          <cfif not CompareNoCase(search_dates,'yes')>
            <cfset start_dt = ParseDateTime(xstart_date) />
            <cfset end_dt   = ParseDateTime(xend_date) />
          </cfif>
        </cfif>
      </cfif>
    </cfif>
    
    <!--- build the sort string --->
    <cfset sort_list = "" />
    <cfset sort_title = "" />
    <cfset sort_list1 = "" />
    <cfif Len(sort_org) GT 0>
      <cfset sort_list = ListAppend(sort_list,"x") />
      <cfset sort_title = ListAppend(sort_title,"x") />
    </cfif>
    <cfif Len(sort_emp) GT 0>
      <cfset sort_list = ListAppend(sort_list,"x") />
      <cfset sort_title = ListAppend(sort_title,"x") />
    </cfif>
    <cfif Len(sort_lname) GT 0>
      <cfset sort_list = ListAppend(sort_list,"x") />
      <cfset sort_title = ListAppend(sort_title,"x") />
    </cfif>
    <cfif Len(sort_org) GT 0>
      <cfset sort_list = ListSetAt(sort_list,#sort_org#,"org.old_org_code") />
      <cfset sort_title = ListSetAt(sort_title,#sort_org#,"Org Code") />
    </cfif>
    <cfif Len(sort_emp) GT 0>
      <cfset sort_list = ListSetAt(sort_list,#sort_emp#,"employee.emp_ssn_num") />
      <cfset sort_title = ListSetAt(sort_title,#sort_emp#,"Employee S-Number") />
    </cfif>
    <cfif Len(sort_lname) GT 0>
      <cfset sort_list = ListSetAt(sort_list,#sort_lname#,"ename") />
      <cfset sort_title = ListSetAt(sort_title,#sort_lname#,"Employee Last Name") />
    </cfif>
    <cfif Len(sort_course) GT 0>
      <cfset sort_title = ListSetAt(sort_title,#sort_lname#,"Course Code") />
    </cfif>
    <cfif Len(sort_date) GT 0>
      <cfset sort_title = ListSetAt(sort_title,#sort_date#,"Course Completion Date") />
    </cfif>
    
    <cfif Len(sort_list) EQ 0>
      <cfset sort_list = "org.old_org_code" />
    </cfif>
    
    <cfif Len(sort_list1) EQ 0>
      <cfset sort_list1 = "course,rev,completed" />
    </cfif>
    
    <cfif Len(sort_title) EQ 0>
      <cfset sort_title = "Org Code" />
    </cfif>
    
    <cfif (Len(sort_course) GT 0) OR (Len(sort_date) GT 0)>
      <cfif Len(sort_course) EQ 0>
        <cfset sort_course=0 />
      <cfelse>
        <cfset sort_course = Val(sort_course) />
      </cfif>
      <cfif Len(sort_date) EQ 0>
        <cfset sort_date=0 />
      <cfelse>
        <cfset sort_date = Val(sort_date) />
      </cfif>
      
      <cfif sort_course GT sort_date>
        <cfset sort_list1 = 'type, course, rev, setting, completed, classnum desc' />
      <cfelse>
        <cfset sort_list1 = 'type, completed, setting, course, rev, classnum desc' />
      </cfif>
    </cfif>
    
    <cfset sort_list = "order by " & sort_list />
    <cfset sort_title = "Sorted By " & sort_title />
    <cfset sort_list1 = "order by " & sort_list1 />
    
    <body bgcolor="white">
    <cfparam name="search" default="" />
    
    <cfif NOT CompareNoCase(sourcetype,'oracle')>
      <cfquery name="q_employee" datasource="#data#">
        SELECT rtrim(emp_last_name) || ', ' || rtrim(emp_first_name) || ' ' || substr(emp_mid_name,1,1) ename,
        emp_ssn_num, ORG.OLD_ORG_CODE AS OLD_ORG_CODE
        FROM EMPLOYEE, ORG
        WHERE EMPLOYEE.ORG_CODE = ORG.ORG_CODE
        <cfif NOT CompareNoCase(search_emp,'yes')>
          <cfif NOT CompareNoCase(empqry,'snumber')>
            AND employee.emp_ssn_num = '#report_emp#'
          <cfelseif NOT CompareNoCase(empqry,'area')>
            <cfif NOT CompareNoCase(area_code,'AW')>
              and employee.prime_co_code in ('#amwtp_list#')
              and employee.area_code = 'RW'
            <cfelse>
              and employee.area_code = '#area_code#'
            </cfif>
          <cfelseif NOT CompareNoCase(empqry,'all')>
            and employee.emp_ssn_num in (select emp_ssn_num from employee where emp_active_stat_flag in ('A','L','U'))
          </cfif>
        </cfif>
        <cfinclude template="trainrpt_search_inc_org.cfm">
        
        #sort_list#
        
      </cfquery>
    <cfelseif NOT CompareNoCase(sourcetype,'sqlserver')>
      <cfquery name="q_employee" datasource="#data#">
        SELECT rtrim(emp_last_name) + ', ' + rtrim(emp_first_name) + ' ' + substring(IsNull(emp_mid_name,' '),1,1) ename,
        emp_ssn_num, ORG.OLD_ORG_CODE OLD_ORG_CODE
        FROM EMPLOYEE, ORG
        WHERE EMPLOYEE.ORG_CODE = ORG.ORG_CODE
        <cfif NOT CompareNoCase(search_emp,'yes')>
          <cfif NOT CompareNoCase(empqry,'snumber')>
            AND employee.emp_ssn_num = '#report_emp#'
          <cfelseif NOT CompareNoCase(empqry,'area')>
            <cfif NOT CompareNoCase(area_code,'AW')>
              and employee.prime_co_code in ('#amwtp_list#')
              and employee.area_code = 'RW'
            <cfelse>
              and employee.area_code = '#area_code#'
            </cfif>
          <cfelseif NOT CompareNoCase(empqry,'all')>
            and employee.emp_ssn_num in (select emp_ssn_num from employee where emp_active_stat_flag in ('A','L','U'))
          </cfif>
        </cfif>
        <cfinclude template="trainrpt_search_inc_org.cfm">
        
        #sort_list#
        
      </cfquery>
    </cfif>
    
    <cfsetting EnableCFOutputOnly="yes">
    <cfoutput><div align="center">
      <cfinclude template="trainrpt_rep_criteria_inc.cfm">
      
      <cfscript>
        
        if (histall IS "true")
        {
          WriteOutput('<br><b>Showing All Training History</b><br>');
        } else {
          tmp = 'Showing ';
          needcomma = false;
          if (histcourse IS true)
          {
            tmp = tmp & 'Courses';
            needcomma = true;
          }
          if (histqual IS true)
          {
            if (needcomma)
            {
              tmp = tmp & ', ';
            }
            tmp = tmp & 'Quals';
            needcomma = true;
          }
          if (histread IS true)
          {
            if (needcomma)
            {
              tmp = tmp & ', ';
            }
            tmp = tmp & 'Required Reading';
            needcomma = true;
          }
          if (histadmin IS true)
          {
            if (needcomma)
            {
              tmp = tmp & ', ';
              needcomma = true;
            }
            tmp = tmp & 'Admin Forms';
          }
          
          WriteOutput('<b>' & tmp & '</b><br>');
        }
        
      </cfscript>
    </cfoutput>
    
    <cfscript>
      if (NOT CompareNoCase(search_dates,"yes"))
      {
        WriteOutput('<b>from ' & DateFormat(start_dt,"mm/dd/yy") & ' to ' & DateFormat(end_dt,"mm/dd/yy") & '</b><br>');
      }
      WriteOutput('<hr width="25%" size="1" noshade align="center">');
      WriteOutput('</div>');
    </cfscript>
    <cfloop query="q_employee">
      
      <cfscript>
        WriteOutput('<div align="center"><table width="675" border="0"><tr><td><pre>');
        snumber=mid(q_employee.emp_ssn_num,4,6);
        WriteOutput('<br>');
        empssn_num=q_employee.emp_ssn_num;
        WriteOutput('<table width="100%" border="0" cellpadding="1" cellspacing="0"><tr><td bgcolor="##cccccc" width="5%"><a href="#plsql_host##job_requirements_report#.#jrr_option#?report_emp=#empssn_num#&expandjobs=yes&expandrnipp=N&myjob=&showprog=N&contract_id=#contract_id#"><img src="images/jrr.gif" border="0"></a></td><td bgcolor="##cccccc" width="40%" align="left"><b>Name: #Mid(q_employee.ENAME,1,30)#</td><td bgcolor="##cccccc" width="40%"><b>Employee Number: ');
        if (NOT CompareNoCase(left(q_employee.emp_ssn_num,3),"000"))
        {
          WriteOutput(Right(q_employee.EMP_SSN_NUM,6));
        } else {
          WriteOutput(Left(q_employee.emp_ssn_num,3) & '-' & Mid(q_employee.emp_ssn_num,4,2) & '-' & Right(q_employee.emp_ssn_num,4));
        }
        WriteOutput('</b></td><td bgcolor="##cccccc" width="15%"><b>Org: #q_employee.OLD_ORG_CODE#</b></td></tr></table>');
      </cfscript>
      
      <!------------------------------------------------------------------------------------------------------------------
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////  Courses  ///////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ------------------------------------------------------------------------------------------------------------------->
      <cfif (histall IS true) or (histcourse is true)>
        
        <cfif NOT CompareNoCase(sourcetype,'oracle')>
          <cfquery name="q_get_courses" datasource="#data#" >
            select b.course_code as course,to_char(retrain_date,'mm/dd/yyyy') due_date,
            b.instr_type_code as setting,
            b.course_rev_num as rev,
            c.course_title as title,
            {fn IfNull(b.class_title,c.course_title)} as class_title,
            a.complete_date as completed,
            a.class_code as classnum,
            a.class_attend_status_code,
            nvl(to_char(complete_date,'mm/dd/yyyy'),'         ') completed_date
            from class_attend a, class b, course c
            where a.emp_ssn_num = '#q_employee.emp_ssn_num#'
            and ((a.class_attend_status_code in (select class_attend_status_code from class_attend_status where class_attend_pass_flag='Y'))
                 or
                 (a.class_attend_status_code = 'A'))
            <cfif NOT CompareNoCase(search_dates,'yes')>
              and (trunc(a.complete_date) between #start_dt# and #end_dt# )
            </CFIF>
            and b.class_code = a.class_code
            and c.course_code = b.course_code
            and c.instr_type_code = b.instr_type_code
            and c.course_rev_num = b.course_rev_num
            #sort_list1#
          </cfquery>
        <cfelseif NOT CompareNoCase(sourcetype,'sqlserver')>
          <cfquery name="q_get_courses" datasource="#data#">
            select b.course_code course,convert(varchar(12),retrain_date) due_date,
            b.instr_type_code setting,
            REQUAL_INTVL_MTH,
            b.course_rev_num rev,
            c.course_title title,
            IsNull(b.class_title,c.course_title) class_title,
            a.complete_date completed,
            <!--- a.due_date, BCC 04.28.2004 this column doesn't exist in train3--->
            a.class_code classnum,
            a.class_attend_status_code,
            case (IsNull(convert(varchar,a.complete_date,101),'xx')) when 'xx' then '          ' else convert(varchar,a.complete_date,101) end completed_date
            from class_attend a, class b, course c, class_attend_status g
            where a.emp_ssn_num = '#q_employee.emp_ssn_num#'
            and g.class_attend_status_code = a.class_attend_status_code
            and g.class_attend_pass_flag = 'Y'
            <cfif NOT CompareNoCase(search_dates,'yes')>
              and (a.complete_date between #start_dt# and #end_dt# )
            </CFIF>
            and b.class_code = a.class_code
            and c.course_code = b.course_code
            and c.instr_type_code = b.instr_type_code
            and c.course_rev_num = b.course_rev_num
            #sort_list1#
          </cfquery>
        </cfif>
        
        <cfscript>
          course_chg = "--------";
          if (q_get_courses.recordcount GT 0)
          {
            writeoutput('<br><table width="100%" border="0" cellpadding="0" cellspacing="0">');
            writeoutput('<tr><th colspan="6" align="left"><h3>Courses</h3></th></tr>');
            writeoutput('<tr> <th width="12%" valign="top" align="left">Course<br>&nbsp;<br><hr size="1" width="100%" noshade></th>' &
                        '<th width="7%" align="left">Type/<br>Rev<br><hr size="1" width="100%" noshade></th>' &
                        '<th width="45%" align="left">Course Title/<br>Class Title(s)<br><hr size="1" width="100%" noshade></th>' &
                        '<th width="14%" align="left">Completion<br>Date<br><hr size="1" width="100%" noshade></th>' &
                        '<th width="13%" align="left">Expire<br>Date<br><hr size="1" width="100%" noshade></th>' &
                        '<th width="9%" align="left">Class<br>Code<br><hr size="1" width="100%" noshade></th>' &
                        '</tr>');
            for (i=1; i LTE q_get_courses.RecordCount; i= i+1) {
              
              if (CompareNoCase(course_chg,q_get_courses.course[i] & q_get_courses.setting[i]))
              {
                writeoutput('<tr><th align="left">' & q_get_courses.course[i] & '</th><th align="left">' & q_get_courses.setting[i] & '</th><th colspan="3" align="left">' & q_get_courses.title[i] & '</th></tr>');
              }
              course_chg = q_get_courses.course[i] & q_get_courses.setting[i];
              if (NOT CompareNoCase(q_get_courses.class_attend_status_code[i],'A')){
                writeoutput('<tr><td>&nbsp;</td><td valign="top">' & q_get_courses.rev[i] & '</td><td valign="top">' & Left(q_get_courses.class_title[i],40) & '&nbsp;</td><td valign="top" colspan="2"><b>(Audited</b> on ' &  q_get_courses.completed_date[i] & ')</td><td align="right">' & q_get_courses.classNum[i] & '</td></tr>');
              } else {
                writeoutput('<tr><td>&nbsp;</td><td valign="top">' & q_get_courses.rev[i] & '</td><td valign="top">' & Left(q_get_courses.class_title[i],40) & '&nbsp;</td><td valign="top">' & q_get_courses.completed_date[i] & '</td><td valign="top">' & q_get_courses.due_date[i] & '&nbsp;</td><td align="right">' & q_get_courses.classNum[i] & '</td></tr>');
              }
            }
            writeoutput('<tr><td colspan="6"><hr size="1" width="100%" noshade></td></tr>');
          }
          WriteOutput('</table>');
        </cfscript>
      </cfif>
      <!------------------------------------------------------------------------------------------------------------------
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////  Quals  ////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ------------------------------------------------------------------------------------------------------------------->
      <cfif (histall IS true) or (histqual is true)>
        
        <cfscript>
          whichqual = "all";
          ssn_change = "123456789";
          expired = DateAdd('d',-1,Now());
        </cfscript>
        
        <cfif NOT CompareNoCase(sourcetype,'oracle')>
          <cfquery name="q_get_quals" datasource="#data#" >
            select qualification.qlf_code, eq.qlf_seq_num, qualification.qlf_title, eq.qlf_date, eq.expir_date, eq.emp_ssn_num,
            eq.addit_desc, eq.resp_size
            from emp_qlf eq, qualification
            where eq.emp_ssn_num = '#q_employee.emp_ssn_num#'
            <cfif NOT CompareNoCase(whichqual,'active')>
              and eq.qlf_seq_num in (select qlf_seq_num
                                     from prog_qlf
                                     where prog_code in (select prog_code
                                                         from job_prog
                                                         where job_code in (select job_code
                                                                            from emp_job
                                                                            where emp_ssn_num=eq.emp_ssn_num)))
            <cfelse>
              <cfif NOT CompareNocase(whichquals,'current')>
                and NVL(eq.expir_date,sysdate+1) >= sysdate
              </cfif>
            </cfif>
            and qualification.qlf_seq_num = eq.qlf_seq_num
            <cfif NOT CompareNocase(whichquals,'current')>
              and qlf_date =
              (select max(qlf_date)
               from emp_qlf b
               where b.emp_ssn_num = eq.emp_ssn_num
               <cfif NOT CompareNoCase(whichqual,'active')>
                 and b.qlf_seq_num in (select qlf_seq_num
                                       from prog_qlf
                                       where prog_code in (select prog_code
                                                           from job_prog
                                                           where job_code in (select job_code
                                                                              from emp_job
                                                                              where emp_ssn_num=b.emp_ssn_num)))
               <cfelse>
                 and NVL(b.expir_date,sysdate+1) >= sysdate
               </cfif>
               and b.qlf_seq_num = eq.qlf_seq_num)
              and eq.qlf_date =  (select max(qlf_date)
                                  from emp_qlf b
                                  where b.emp_ssn_num = eq.emp_ssn_num
                                  and b.qlf_seq_num = eq.qlf_seq_num
                                  and decode(expir_date,null,qlf_date,expir_date) =
                                  decode(eq.expir_date,null,eq.qlf_date,eq.expir_date))
              and nvl(eq.expir_date,sysdate+15000) = (select max(nvl(expir_date,sysdate+15000))
                                                      from emp_qlf b
                                                      where b.emp_ssn_num = eq.emp_ssn_num
                                                      and b.qlf_seq_num = eq.qlf_seq_num)
            </cfif>
            <cfif NOT CompareNoCase(search_dates,'yes')>
              and (eq.qlf_date between #start_dt# and #end_dt#)
            </cfif>
            order by qlf_code, expir_date
            
          </cfquery>
        <cfelseif NOT CompareNoCase(sourcetype,'sqlserver')>
          <cfquery name="q_get_quals" datasource="#data#">
            
            select qualification.qlf_code, qualification.qlf_title, eq.qlf_date, eq.expir_date, eq.emp_ssn_num,
            emp_last_name, emp_first_name, emp_mid_name,eq.qlf_seq_num,
            rtrim(emp_last_name) + ', ' + rtrim(emp_first_name) + ' ' + substring(IsNull(emp_mid_name,' '),1,1) ename,
            eq.addit_desc,org.old_org_code, eq.resp_size
            from emp_qlf eq, qualification, employee, org
            where employee.org_code = org.org_code
            <cfif NOT CompareNoCase(whichqual,'active')>
              and eq.qlf_seq_num in (select qlf_seq_num
                                     from prog_qlf
                                     where prog_code in (select prog_code
                                                         from job_prog
                                                         where job_code in (select job_code
                                                                            from emp_job
                                                                            where emp_ssn_num=eq.emp_ssn_num)))
            <cfelse>
              and IsNull(eq.expir_date,getdate()+1) >= getdate()
            </cfif>
            and employee.emp_active_stat_flag in ('A','L','T','U')
            and eq.emp_ssn_num = employee.emp_ssn_num
            and qualification.qlf_seq_num = eq.qlf_seq_num
            and eq.qlf_date =  (select max(qlf_date)
                                from emp_qlf b
                                where b.emp_ssn_num = eq.emp_ssn_num
                                <cfif NOT CompareNoCase(whichqual,'active')>
                                  and b.qlf_seq_num in (select qlf_seq_num
                                                        from prog_qlf
                                                        where prog_code in (select prog_code
                                                                            from job_prog
                                                                            where job_code in (select job_code
                                                                                               from emp_job
                                                                                               where emp_ssn_num=b.emp_ssn_num)))
                                <cfelse>
                                  and IsNull(b.expir_date,getdate()+1) >= getdate()
                                </cfif>
                                and b.qlf_seq_num = eq.qlf_seq_num)
            
            <cfif NOT CompareNoCase(search_emp,"yes")>
              AND employee.EMP_SSN_NUM = '#report_emp#'
            </cfif>
            
            <cfinclude template="trainrpt_search_inc_org.cfm">
            
            <cfif NOT CompareNoCase(search_dates,'yes')>
              and (eq.qlf_date between #start_dt# and #end_dt#)
            </cfif>
            
            #sort_list#
            
          </cfquery>
        </cfif>
        
        <cfif q_get_quals.recordcount GT 0>
          <cfscript>
            empssn_num = emp_ssn_num;
            ssn_change = emp_ssn_num;
            WriteOutput('<br><table width="100%" border="0" cellpadding="0" cellspacing="0">');
            
            WriteOutput('<tr><td colspan="6" valign="bottom"><h3>Quals</h3></td></tr>');
            WriteOutput('<tr>' &
                        '<th width="12%" align="left">Qual<br>Code<br><hr size="1" width="100%" noshade></th>' &
                        '<th width="40%" align="left">Qual<br>Title<br><hr size="1" width="100%" noshade></th>' &
                        '<th width="12%" align="left">Qual<br>Date<br><hr size="1" width="100%" noshade></th>' &
                        '<th width="12%" align="left">Expire<br>Date<br><hr size="1" width="100%" noshade></th>' &
                        '<th width="16%" align="left">Comment<br>&nbsp;<br><hr size="1" width="100%" noshade></th>' &
                        '<th width="8%" align="left">Resp<br>Size<br><hr size="1" width="100%" noshade></th>' &
                        '</tr>');
          </cfscript>
          <cfloop query="q_get_quals">
            
            <cfif NOT CompareNoCase(whichqual,'active')>
              <cfset p_emp_ssn_num="#q_employee.emp_ssn_num#">
              <cfset p_qlf_code="#q_get_quals.qlf_code#">
              <cfset p_qlf_rev = "">
              <cfset p_qlf_seq_num=#q_get_quals.qlf_seq_num#>
              
              <cfinclude template="jrr_create_qual_equiv_struct_inc.cfm">
              <cfset qual_equiv.date_template ="mm/dd/yyyy">
              <cfinclude template="jrr_ck_qual_equiv_inc.cfm">
            <cfelse>
              <cfquery name="qstatus" datasource="#data#">
                select count(*) statcount
                from emp_qlf eq
                where qlf_seq_num = #q_get_quals.qlf_seq_num#
                and emp_ssn_num = '#q_get_quals.emp_ssn_num#'
                and qlf_seq_num in (select qlf_seq_num
                                    from prog_qlf
                                    where prog_code in (select prog_code
                                                        from job_prog
                                                        where job_code in (select job_code
                                                                           from emp_job
                                                                           where emp_ssn_num=eq.emp_ssn_num)))
              </cfquery>
              
              <cfif qstatus.statcount GT 0>
                <cfset statflag = 'R'>
              <cfelse>
                <cfset statflag = 'N'>
              </cfif>
              
            </cfif>
            
            <cfscript>
              
              if (NOT CompareNoCase(whichqual,'active'))
              {
                if (qual_equiv.eq_is_valid)
                {
                  
                  WriteOutput('<tr><td><a href="http://trainweb.inel.gov/index.cfm?FuseAction=details&qlf_seq_num=#q_get_quals.qlf_seq_num#&startrow=1&maxrows=25&orderby=q.qlf_code&detlevel=less&web=mqgguest">#qlf_code#</a></td><td>#Left(qlf_title,30)#</td><td>#Left(DateFormat(qlf_date,"mm/dd/yyyy"),10)#&nbsp;</td>');
                  if ((expired gt expir_date) and (Len(expir_date) gt 0))
                  {
                    WriteOutput('<td>#Left(DateFormat(expir_date,"mm/dd/yyyy"),10)#&nbsp;</td><td colspan="2"><font color="blue">  **Equiv **</font></td></tr>');
                  } else {
                    WriteOutput('<td>#Left(DateFormat(expir_date,"mm/dd/yyyy"),10)#&nbsp;</td><td>#Mid(addit_desc & '       ',1,10)#&nbsp;</td><td>#resp_size#</td></tr>');
                  }
                  WriteOutput('<tr><td><font color="blue"> **Equiv ** #qual_equiv.eq_qual_code# #left(qual_equiv.eq_qual_title & RepeatString(" ",24),24)#</td><td>#qual_equiv.eq_qual_qdate#</td><td>#qual_equiv.eq_qual_xdate#</font>&nbsp;</td><td colspan="2">&nbsp;</td></tr>');
                } else {
                  WriteOutput('<tr><td><a href="http://trainweb.inel.gov/index.cfm?FuseAction=details&qlf_seq_num=#q_get_quals.qlf_seq_num#&startrow=1&maxrows=25&orderby=q.qlf_code&detlevel=less&web=mqgguest">#qlf_code#</a></td><td>#Left(qlf_title,30)#</td><td>#Left(DateFormat(qlf_date,"mm/dd/yyyy"),10)#&nbsp;</td>');
                  if ((expired gt expir_date) and (Len(expir_date) gt 0))
                  {
                    WriteOutput('<td>#Left(DateFormat(expir_date,"mm/dd/yyyy"),10)#&nbsp;</td><td colspan="2"><font color="red">Expired</font></td></tr>');
                  } else {
                    WriteOutput('<td>#Left(DateFormat(expir_date,"mm/dd/yyyy"),10)#&nbsp;</td><td>#Mid(addit_desc & '       ',1,10)#&nbsp;</td><td>#resp_size#&nbsp;</td></tr>');
                    
                  }
                }
                
              } else {
                WriteOutput('<tr><td><a href="http://trainweb.inel.gov/index.cfm?FuseAction=details&qlf_seq_num=#q_get_quals.qlf_seq_num#&startrow=1&maxrows=25&orderby=q.qlf_code&detlevel=less&web=mqgguest">#qlf_code#</a></td><td>#Left(qlf_title,30)#</td><td align="left">#Left(DateFormat(qlf_date,"mm/dd/yyyy"),10)#&nbsp;</td>');
                if ((expired gt expir_date) and (Len(expir_date) gt 0))
                {
                  WriteOutput('<td align="left">#Left(DateFormat(expir_date,"mm/dd/yyyy"),10)#&nbsp;</td><td><font color="red">Expired</font></td><td>#resp_size#&nbsp;</td></tr>');
                } else {
                  WriteOutput('<td align="left">#Left(DateFormat(expir_date,"mm/dd/yyyy"),10)#&nbsp;</td><td>#Mid(addit_desc & '       ',1,10)#&nbsp;</td><td>#resp_size#&nbsp;</td></tr>');
                }
              }
            </cfscript>
            
          </cfloop>
          
          <cfscript>
            writeoutput('<tr><td colspan="6"><hr size="1" width="100%" noshade></td></tr>');
            WriteOutput('</table><br>');
          </cfscript>
        </cfif>
        
        <!--- This code looks in the welder qualification database to see if the person has a current welder
        qualification. If they do, it puts in a link to the welder qual reports.
        <cfset errorflag="0">
        <cftry>
          <cfset empssntemp=right(q_get_quals.emp_ssn_num, 6)>
          <cfif left(empssntemp, 1) eq 0>
            <cfset empssn=right(empssntemp, 5)>
          <cfelse>
            <cfset empssn=empssntemp>
          </cfif>
          
          <cfquery name="empweldquals" datasource="weldcert">
            SELECT Distinct QUAL_ID, S_NUMBER, PROCNUM, TEST_DATE, EXP_DATE, ASME, AWS_D1_1, AWS_D1_2,
            AWS_D1_3, AWS_D9_1, AWS_D14_1, MIN_THICK, MAX_THICK, MIN_DIAM, FIRST_DATE, REMARKS, X_CODE,
            WELDCARD, PROCESS
            FROM QUALIFY
            WHERE (S_NUMBER = #empssn#) AND (EXP_DATE > GETDATE())
            ORDER BY PROCNUM
          </cfquery>
          
          <cfcatch type="Any">
            <cfset errorflag="1">
          </cfcatch>
        </cftry>
        
        <cfif errorflag is "0">
          <cfif empweldquals.recordcount gt 0>
            <cfoutput>
              This person holds current welder qualifications.
              Click here to go to the employee#chr(39)#s <a href="http://trainweb.inel.gov/index.cfm?web=weldcert&fuse=runreport&snumber=#empssn#&report=EWQ">welder qualification report</a>.
            </cfoutput>
          </cfif>
          </cfif--->>
        </cfif>
        <!------------------------------------------------------------------------------------------------------------------
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////  Reading  ///////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ------------------------------------------------------------------------------------------------------------------->
        <cfif (histall IS true) or (histread is true)>
          <cfif NOT CompareNoCase(sourcetype,'oracle')>
            <cfquery name="q_get_reading" datasource="#data#">
              select g.read_code as course, '       ' as setting, g.read_rev as rev,h.doc_ttl as title, h.doc_id as class_title,
              to_char(g.cmpl_date,'mm/dd/yyyy') as completed, 0 as classnum,doc_id
              from emp_reading g, reading h
              where g.emp_ssn_num = '#q_employee.emp_ssn_num#'
              <cfif NOT CompareNoCase(search_dates,'yes')>
                and (g.cmpl_date between #start_dt# and #end_dt# )
              </cfif>
              and h.read_code = g.read_code
              and h.read_rev = g.read_rev
              and h.contract_id = g.contract_id
              #sort_list1#
            </cfquery>
          <cfelseif NOT CompareNoCase(sourcetype,'sqlserver')>
            <cfquery name="q_get_reading" datasource="#data#">
              select g.read_code course, '       ' setting, g.read_rev rev,
              h.doc_ttl title, h.doc_id class_title,
              CONVERT(VARCHAR(12),g.cmpl_date,101) completed,
              0 classnum, doc_id
              from emp_reading g, reading h
              where g.emp_ssn_num = '#q_employee.emp_ssn_num#'
              <cfif NOT CompareNoCase(search_dates,'yes')>
                and (g.cmpl_date between #start_dt# and #end_dt# )
              </cfif>
              and h.read_code = g.read_code
              and h.contract_id = g.contract_id
              and h.read_rev = g.read_rev
              #sort_list1#
            </cfquery>
          </cfif>
          
          <cfset course_chg = "--------">
          <cfif (q_get_reading.RecordCount GT 0)>
            <cfoutput><br><table width="100%" border="0" cellpadding="0" cellspacing="0"></cfoutput>
              <cfoutput><tr><td colspan="4" valign="bottom"><h3>Required Reading</h3></td></tr></cfoutput>
              <cfoutput><tr><th width="15%" align="left">Read<br>Code<br><hr size="1" width="100%" noshade></th>
                  <th width="10%" align="left">Rev<br>&nbsp;<br><hr size="1" width="100%" noshade></th>
                  <th width="60%" align="left">Reading<br>Title<br><hr size="1" width="100%" noshade></th>
                  <th width="15%" align="left">Completion<br>Date<br><hr size="1" width="100%" noshade></th>
                </tr></cfoutput>
              <cfloop query="q_get_reading">
                <cfif NOT CompareNoCase(sourcetype,'oracle')>
                  <cfquery name="q_local_reading" datasource="#data#">
                    select read_code,read_link
                    FROM req_read_link
                    WHERE read_code = '#q_get_reading.course#'
                    and nvl(read_rev,'#rev#')  = '#rev#'
                  </cfquery>
                  
                <cfelseif NOT CompareNoCase(sourcetype,'sqlserver')>
                  <cfquery name="q_local_reading" datasource="#data#">
                    select read_code,read_link
                    FROM req_read_link
                    WHERE read_code = '#q_get_reading.course#'
                    and IsNUll(read_rev,'#rev#')  = '#rev#'
                  </cfquery>
                </cfif>
                <cfset rtitle = mid(q_get_reading.title,1,50)>
                
                <cfif (NOT CompareNoCase(q_get_reading.course,course_chg))>
                  <cfoutput><tr><td>&nbsp;</td><td>#q_get_reading.rev#</td><td>#rtitle#&nbsp;</td><td>#q_get_reading.completed#</td></tr></cfoutput>
                <cfelse>
                  <cfif q_local_reading.RecordCount GT 0>
                    <cfoutput><tr><td align="left"><a href="#q_local_reading.read_link#" target="_main">#q_get_reading.course#</a></td><td align="left">#q_get_reading.rev#</td><td align="left">#rtitle#&nbsp;</td><td align="left">#q_get_reading.completed#</td></tr></cfoutput>
                  <cfelse>
                    <cfif (len(q_get_reading.doc_id) GT 0)>
                      <cfoutput><tr><td align="left"><a href="http://edms.inel.gov/pls/docs/doc_3?f_doc=#q_get_reading.doc_id#" target="_main">#q_get_reading.course#</a></td><td align="left">#q_get_reading.rev#</td><td align="left">#rtitle#&nbsp;</td><td align="left">#q_get_reading.completed#</td></tr></cfoutput>
                    <cfelse>
                      <cfoutput><tr><td align="left">#q_get_reading.course#</td><td align="left">#q_get_reading.rev#</td><td align="left">#rtitle#&nbsp;</td><td align="left">#q_get_reading.completed#</td></tr></cfoutput>
                    </cfif>
                  </cfif>
                </cfif>
                <cfset course_chg = q_get_reading.course>
              </cfloop>
              <cfoutput><tr><td colspan="5"><hr size="1" width="100%" noshade></td></tr></cfoutput>
              <cfoutput></table></cfoutput>
          </cfif>
        </cfif>
        <!------------------------------------------------------------------------------------------------------------------
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////  Admin Forms  /////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ------------------------------------------------------------------------------------------------------------------->
        <cfif (histall IS true) or (histadmin is true)>
          <cfif NOT CompareNoCase(sourcetype,'oracle')>
            <cfquery name="q_get_admin_doc" datasource="#data#">
              select admin_doc.admin_doc_code admin_doc_code,
              admin_doc.admin_doc_title admin_doc_title,
              nvl(to_char(ead.complete_date,'mm/dd/yyyy'),'          ') complete_date,
              nvl(to_char(ead.expire_date,'mm/dd/yyyy'),'          ') expire_date,
              ead.emp_ssn_num,
              ead.resp_size resp_size,
              ead.comments
              from emp_admin_doc ead, admin_doc
              where ead.emp_ssn_num = '#q_employee.emp_ssn_num#'
              and admin_doc.admin_doc_code = ead.admin_doc_code
              <cfif NOT CompareNoCase(search_dates,'yes')>
                and (trunc(ead.complete_date) between #start_dt# and #end_dt# )
              </CFIF>
              order by ead.admin_doc_code
            </cfquery>
          <cfelseif NOT CompareNoCase(sourcetype,'sqlserver')>
            <cfquery name="q_get_admin_doc" datasource="#data#">
              select admin_doc.admin_doc_code admin_doc_code,
              admin_doc.admin_doc_title admin_doc_title,
              IsNull(convert(varchar(12),ead.complete_date,101),'          ') complete_date,
              IsNull(convert(varchar(12),ead.expire_date,101),'          ') expire_date,
              ead.emp_ssn_num,
              ead.resp_size resp_size,
              ead.comments
              from emp_admin_doc ead, admin_doc
              where ead.emp_ssn_num = '#q_employee.emp_ssn_num#'
              and admin_doc.admin_doc_code = ead.admin_doc_code
              <cfif NOT CompareNoCase(search_dates,'yes')>
                and (convert(datetime,ead.complete_date,101) between #start_dt# and #end_dt# )
              </CFIF>
              order by ead.admin_doc_code
            </cfquery>
          </cfif>
          
          <cfscript>
            if (q_get_admin_doc.RecordCount GT 0)
            {
              WriteOutput('<br><table width="100%" border="0" cellpadding="0" cellspacing="0">');
              WriteOutput('<tr><td colspan="5" valign="bottom"><h3>Admin Forms</h3></td></tr>');
              WriteOutput('<tr>' &
                          '<th width="12%" align="left">Admin<br>Code<br><hr size="1" width="100%" noshade></th>' &
                          '<th width="53%" align="left">Title<br>&nbsp;<br><hr size="1" width="100%" noshade></th>' &
                          '<th width="13%" align="left">Assign<br>Date<br><hr size="1" width="100%" noshade></th>' &
                          '<th width="13%" align="left">Expire<br>Date<br><hr size="1" width="100%" noshade></th>' &
                          '<th width="9%" align="left">Resp<br>Size<br><hr size="1" width="100%" noshade></th>' &
                          '</tr>');
              
              for (i = 1; i LTE q_get_admin_doc.RecordCount; i = i + 1)
              {
                WriteOutput('<tr><td>' &
                            q_get_admin_doc.admin_doc_code[i] & '</td><td>' &
                            q_get_admin_doc.admin_doc_title[i] & '</td><td>' &
                            q_get_admin_doc.complete_date[i] & '</td><td>' &
                            q_get_admin_doc.expire_date[i] & '&nbsp;</td><td>' &
                            q_get_admin_doc.resp_size[i] & '&nbsp;</td></tr>');
              }
              
              writeoutput('<tr><td colspan="5"><hr size="1" width="100%" noshade></td></tr>');
              WriteOutput('</table><br>');
            }
          </cfscript>
        </cfif>
        
        <cfscript>
          WriteOutput('</td></tr></table></div>');
        </cfscript>
        
      </cfloop>
      </div>
      <cfsetting EnableCFOutputOnly="no"></pre>
      
      <cfset def_firstopt = 'input' />
      <cfinclude template="trainrpt_new_search_inc.cfm">
      
    <cfelse>
      <cfoutput><cflocation url="#whoami#?option=input"></cfoutput>
    </cfif>
    </body>
  </html>
