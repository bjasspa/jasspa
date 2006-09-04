<cfif NOT AllowReports>
   <cflocation url="no_reports.htm" />
</cfif>
<cfif REDIRECTREPORTS>
   <cflocation url="#reports_host#train_reports.cfm" />
</cfif>
<cfset whoami="trainrpt_ni_whoqm.cfm" />

<cfset title='Employees by Qual Marker' />
<cfset page_title='<h2>' & title & '</h2>' />

<cfparam name="option" default="setup" />
<cfparam name="scan" default="" />
<cfparam name="ckmatrix" default="no" />
<cfparam name="searchtype" default="simple" />
<cfset def_anyemp="yes" />
<cfparam name="isearch" default="" />

<html>
<head>
<cfif NOT CompareNoCase(option,"setup")>
<!--- define the variables to make the "setup" screen look like we want it to look --->
   <cfset fieldname = 'document.myform.isearch' />
   <cfset def_prompt="Select a Qual Marker" />
   <cfset def_validate=true />


   <SCRIPT LANGUAGE="javascript">
   <!--
   function verify (myForm)
   {
      if (myForm.isearch.value.length == 0)
      {
         alert('You must enter something in the search field.');
         return false;
      }
      return true;
   }
   function setfocus(myfield)
   {
      for (i=0;i<document.myform.elements.length;i++) {
         if (document.myform.elements[i].name == myfield.name) {
            document.myform.elements[i].focus();
            break;
         }
      }
   }

   //-->
   </SCRIPT>

   <cfquery name="q_marker_qual" datasource="#data#">
      select qlf_seq_num,qlf_code,qlf_title
        from qualification
       where qlf_seq_num in (select distinct marker_qlf_seq_num
                               from qual_marker_xref)
       order by qlf_code
   </cfquery>

   <cfset setfocus=true />
   <cfset simple_js=false />
   <cfinclude template="trainrpt_header_inc.cfm" />
   <center>
   <hr width="25%" noshade size="1" align="center">
   <form action="<cfoutput>#whoami#</cfoutput>" method="post" name="myform" <CFIF def_validate>onsubmit="return verify(this);"</cfif>>
   <table width="600" border="1" cellspacing="0"><tr><td align="center">
   <table width="600" border="1" cellspacing="0"><tr><td align="left">
   <br>
   <center>
   <b><cfoutput>#def_prompt#</cfoutput></b><br>
   <select name="isearch">
      <cfoutput query="q_marker_qual">
         <option value="#q_marker_qual.qlf_code#">#q_marker_qual.qlf_code#  -  #mid(q_marker_qual.qlf_title,1,50)#</option>
      </cfoutput>
   </select>
   <input type="hidden" name="exp_start" value="<cfoutput>#DateFormat(Now(),'mm/dd/yyyy')#</cfoutput>">
   <input type="hidden" name="exp_end" value="<cfoutput>#DateFormat(DateAdd("D",Now(),90),'mm/dd/yyyy')#"</cfoutput>>
   </td></tr></table>
   </td></tr></table>
   <br>
   <input type="hidden" name="option" value="input">
   <input type="hidden" name="contract_id" value="<cfoutput>#contract_id#</cfoutput>">
   <input type="submit" value="Search">
   <input type="reset">
   </form>

   <cfinclude template="trainrpt_footer_link_inc.cfm">
   </div>

<cfelseif NOT CompareNoCase(option,"input")>
   <cfinclude template="trainrpt_initial_queries_inc.cfm" />
   <cfset def_searchtype = "QualMarker" />
   <cfset def_snumber=false />
   <cfset def_arealist=true />
   <cfset def_anyemp=true />
   <cfset def_searchdate = false />
   <cfset def_Reporttype = false />

   <cfinclude template="trainrpt_input_select_inc.cfm" />
   <cfif getQualMarker.RecordCount is 0>
      <cfinclude template="trainrpt_header_inc.cfm" />
      <cfset def_error_message='<span class="bbb">NOT FOUND: The search string <span class="red">#isearch#</span> was not found in the course codes.</span>' />
      <cfset def_notify_trainadm=false />
      <cfset def_header=true />

      <cfinclude template="trainrpt_error_abort_inc.cfm" />
   </cfif>

   <cfset onloadparm='clickcount=0;' />
   <cfset setfocus=false />
   <cfset def_hiddenvars="contract_id" />
   <cfset def_hiddenvals=contract_id />
   <cfset fieldname='document.myform.report_emp' />

   <cfinclude template="trainrpt_input_inc.cfm" />


<cfelseif NOT CompareNoCase(option,"report")>

   <cfset style_context="report" />
   <cfinclude template="trainrpt_header_inc.cfm" />
   <cfinclude template = "trainrpt_defparm_inc.cfm" />

   <cfinclude template="trainrpt_prescreen_inc.cfm" />

   <cfset RPT_QCODE = UCase(RPT_QCODE)>
   <!----------------start------------------->
   <!--- simple / complex interface logic --->
   <cfset awhileago = DateAdd("y",-90,Now()) />

   <cfparam name="admin_code" default="" />
   <cfparam name="area_code" default="" />
   <cfparam name="orgqry" default="" />
   <cfparam name="search_admin" default="" />
   <cfparam name="search_area" default="" />
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
   <cfparam name="whichqual" default="active" />


   <cfinclude template="trainrpt_xlate_simple_inc.cfm" />

   <cfif NOT CompareNoCase(searchwhat,"emp")>
      <cfset search_emp = "yes" />
   </cfif>

   <cfif IsDefined("form.coursec")>
      <cfset coursec = Ucase(coursec) />
   </cfif>
   <cfif IsDefined("form.instrtype")>
      <cfset setting = ucase(instrtype) />

      <cfif NOT CompareNoCase(sourcetype,'oracle')>
         <cfquery name="getinstrtype" datasource="#data#">
            select instr_type_desc
              from instr_type
             where instr_type_code = '#setting#'
         </cfquery>
      <cfelseif NOT CompareNoCase(sourcetype,'sqlserver')>
         <cfquery name="getinstrtype" datasource="#data#">
            select instr_type_desc
              from instr_type
             where instr_type_code = '#setting#'
         </cfquery>
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
   <cfif Len(sort_date) GT 0>
      <cfset sort_list = ListAppend(sort_list,"x") />
      <cfset sort_title = ListAppend(sort_title,"x") />
   </cfif>
   <cfif Len(sort_course) GT 0>
      <cfset sort_list = ListAppend(sort_list,"x") />
      <cfset sort_title = ListAppend(sort_title,"x") />
   </cfif>
   <cfif Len(sort_org) GT 0>
      <cfset sort_list = ListSetAt(sort_list,#sort_org#,"org.old_org_code") />
      <cfset sort_title = ListSetAt(sort_title,#sort_org#,"Org Code") />
   </cfif>
   <cfif Len(sort_emp) GT 0>
      <cfset sort_list = ListSetAt(sort_list,#sort_emp#,"ej.emp_ssn_num") />
      <cfset sort_title = ListSetAt(sort_title,#sort_emp#,"Employee S-Number") />
   </cfif>
   <cfif Len(sort_lname) GT 0>
      <cfset sort_list = ListSetAt(sort_list,#sort_lname#,"emp_last_name") />
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

   <cfif Len(sort_title) EQ 0>
      <cfset sort_title = "Org Code" />
   </cfif>

   <cfif (Len(sort_course) GT 0) OR (Len(sort_date) GT 0)>
      <cfif Len(sort_course) EQ 0>
         <cfset sort_course=0 />
      <cfelse>
         <cfset sort_course = Val(sort_course)>
      </cfif>
      <cfif Len(sort_date) EQ 0>
         <cfset sort_date=0 />
      <cfelse>
         <cfset sort_date = Val(sort_date) />
      </cfif>
      <cfif sort_course GT sort_date>
          <cfset sort_list1 = 'order by type, course, setting, completed, classnum desc' />
      <cfelse>
          <cfset sort_list1 = 'order by type, completed, setting, course, classnum desc' />
      </cfif>
   </cfif>

   <cfset sort_list = "order by " & sort_list />
   <cfset sort_title = "Sorted By " & sort_title />

   <!--- simple / complex interface logic --->
   <!-----------------end-------------------->

   <center>
   <cfquery name="qlfcode" datasource="#data#">
      SELECT DISTINCT QLF_CODE, QLF_TITLE, QLF_SEQ_NUM, qlf_rev
         FROM QUALIFICATION
         WHERE QLF_CODE = '#rpt_qcode#'
   </cfquery>

   <cfoutput><h4>#qlfcode.qlf_code# - #qlfcode.qlf_title#</h4>

   <!--- <cfinclude template="trainrpt_rep_criteria_inc.cfm"> --->

   <cfinclude template="trainrpt_rep_criteria_inc.cfm">

   <cfset marker_qlf_seq_num = qlfcode.qlf_seq_num>

   </cfoutput>
   <hr width="25%" noshade size="1" align="center">
   </center>
   <br>
   <pre><cfsetting enablecfoutputonly="yes">
   <cfset code_change = "xxxxxxxx">
   <cfset hold_snum   = "xxxxxxxxx">
   <cfset cnta = 0>

  <cfif NOT CompareNoCase(sourcetype,'oracle')>
       <cfquery name="q_employee" datasource="#data#" >
          select distinct ej.emp_ssn_num,org.old_org_code,emp_last_name,
                 decode(function || position, '0000', 'Personal Job Code',function || position || location || jc_modifier) tendigitjobcode
            from emp_job ej,employee,org, job
           where employee.emp_ssn_num=ej.emp_ssn_num
             and ej.job_code = job.job_code
             and employee.org_code = org.org_code
             and ej.job_code in (select job_code
                                   from job_prog
                                  where prog_code in (select prog_code
                                                        from prog_qlf
                                                       where qlf_seq_num = #qlfcode.qlf_seq_num#))
             <cfif NOT CompareNoCase(searchwhat,'emp')>
                 <cfif NOT CompareNoCase(empqry,'snumber')>
                    AND employee.emp_ssn_num = '#report_emp#'
                 <cfelseif NOT CompareNoCase(empqry,'prime')>
                    and employee.co_code in (#inl_list#,#icp_list#)
                 <cfelseif NOT CompareNoCase(empqry,'icpemp')>
                    and employee.co_code in (#icp_list#)
                 <cfelseif NOT CompareNoCase(empqry,'inlemp')>
                    and employee.co_code in (#inl_list#)
                 <cfelseif NOT CompareNoCase(empqry,'amwtpemp')>
                    and employee.co_code in (#amwtp_list#)
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
          select distinct ej.emp_ssn_num,org.old_org_code,emp_last_name,
                 (case [function] + position
                    when '0000' then
                        'Personal Job Code'
                    else
                        [function] + position + location + jc_modifier
                  end) tendigitjobcode
            from emp_job ej,employee,org, job
           where employee.emp_ssn_num=ej.emp_ssn_num
             and ej.job_code = job.job_code
             and employee.org_code = org.org_code
             and ej.job_code in (select job_code
                                   from job_prog
                                  where prog_code in (select prog_code
                                                        from prog_qlf
                                                       where qlf_seq_num = #qlfcode.qlf_seq_num#))
             <cfif NOT CompareNoCase(searchwhat,'emp')>
                 <cfif NOT CompareNoCase(empqry,'snumber')>
                    AND employee.emp_ssn_num = '#report_emp#'
                 <cfelseif NOT CompareNoCase(empqry,'prime')>
                    and employee.co_code in (#inl_list#,#icp_list#)
                 <cfelseif NOT CompareNoCase(empqry,'icpemp')>
                    and employee.co_code in (#icp_list#)
                 <cfelseif NOT CompareNoCase(empqry,'inlemp')>
                    and employee.co_code in (#inl_list#)
                 <cfelseif NOT CompareNoCase(empqry,'amwtpemp')>
                    and employee.co_code in (#amwtp_list#)
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


   <cfoutput><div align="center"><table width="550" border="0"><tr><td><pre></cfoutput>
   <cfif NOT CompareNoCase(search_org,"yes")>
      <cfset lastorg = "xxxx">
   </cfif>

   <cfoutput><b><a href="http://trainweb.inel.gov/index.cfm?FuseAction=details&qlf_seq_num=#qlfcode.qlf_seq_num#&startrow=1&maxrows=25&orderby=qlfcode.qlf_code&detlevel=less&web=mqgguest">#qlfcode.qlf_code#</a> - #Left(qlfcode.QLF_TITLE & RepeatString(" ",30),30)#</b></cfoutput>
   <cfoutput><br><div align="center"></cfoutput>
   <cfoutput><table width="600" border="0" cellpadding="0" cellspacing="0"></cfoutput>
   <cfoutput><tr><th width="20%" align="left">S-Number/<br>&nbsp;&nbsp;Qual</th></cfoutput>
   <cfoutput><th width="40%" align="left">Employee Name/<br>&nbsp;&nbsp;Title</th></cfoutput>
   <cfoutput><th width="20%" align="left">Org/<br>&nbsp;&nbsp;Qual Date</th></cfoutput>
   <cfoutput><th width="20%" align="left">Required By/<br>&nbsp;&nbsp;Expire Date</th></tr></cfoutput>

   <cfloop query="q_employee">

      <cfset this_job_code = q_employee.tendigitjobcode>

      <cfif NOT CompareNoCase(sourcetype,'oracle')>
         <cfquery name="q_qual" datasource="#data#">
            SELECT rtrim(emp_last_name) || ', ' || rtrim(emp_first_name) || ' ' || substr(emp_mid_name,1,1) ename,
                     EMPLOYEE.EMP_SSN_NUM EMPSSN_NUM,employee.org_work_id,ORG.ORG_ID, ORG.ORG_CODE  ORGCODE,ORG.OLD_ORG_CODE
              FROM EMPLOYEE, ORG
             WHERE employee.emp_ssn_num='#q_employee.emp_ssn_num#'
               AND EMPLOYEE.EMP_ACTIVE_STAT_FLAG in ('A','L','U')
               AND ORG.ORG_CODE = EMPLOYEE.ORG_CODE
         </cfquery>
      <cfelseif NOT CompareNoCase(sourcetype,'sqlserver')>
         <cfquery name="q_qual" datasource="#data#">
            SELECT rtrim(emp_last_name) + ', ' + rtrim(emp_first_name) + ' ' + substring(IsNull(emp_mid_name,' '),1,1) ename,
                     EMPLOYEE.EMP_SSN_NUM EMPSSN_NUM,employee.org_work_id,ORG.ORG_ID, ORG.ORG_CODE  ORGCODE,ORG.OLD_ORG_CODE
              FROM EMPLOYEE, ORG
             WHERE employee.emp_ssn_num='#q_employee.emp_ssn_num#'
               AND EMPLOYEE.EMP_ACTIVE_STAT_FLAG in ('A','L','U')
               AND ORG.ORG_CODE = EMPLOYEE.ORG_CODE
         </cfquery>

      </cfif>

      <cfloop query="q_qual">

         <cfset this_org_code = q_qual.old_org_code>
         <cfset print_it = true>
         <cfset ckdate = DateAdd("d",-90, Now())>

         <cfset show_equiv = false>

         <cfif NOT CompareNoCase(whichqual,'active')>

            <cfset p_emp_ssn_num="#q_qual.empssn_num#">
            <cfset p_qlf_code="#RPT_QCODE#">
            <cfset p_qlf_rev = "">
            <cfset p_qlf_seq_num=#qlfcode.qlf_seq_num#>

            <cfinclude template="jrr_create_qual_equiv_struct_inc.cfm">
            <cfset qual_equiv.date_template ="mm/dd/yy">
            <cfinclude template="jrr_ck_qual_equiv_inc.cfm">


            <cfset print_it = print_it or qual_equiv.eq_is_valid>
            <cfset show_equiv = qual_equiv.eq_is_valid>

         </cfif>

         <cfset statflag = 'R'>

         <cfif NOT CompareNoCase(sort_org,"yes")>
            <cfif NOT CompareNoCase(lastorg,"xxxx")>
               <cfset lastorg = q_qual.old_org_code>
            </cfif>
         </cfif>
         <cfif NOT CompareNoCase(sort_org,"yes")>
            <cfif CompareNoCase(lastorg,q_qual.old_org_code)>
               <cfoutput>&nbsp;<br></cfoutput>
               <cfset lastorg = q_qual.old_org_code>
            </cfif>
         </cfif>
         <cfoutput><tr><td align="left"><cfif NOT CompareNoCase(left(empssn_num,3),'000')>#Right(EMPSSN_NUM,6)#      <cfelse>#Left(empssn_num,3)#-#Mid(empssn_num,4,2)#-#Right(empssn_num,4)# </cfif></td></cfoutput>
         <cfoutput><td align="left">#Left(ENAME & "                    ",20)# <a href="#plsql_host##job_requirements_report#.#jrr_option#?report_emp=#empssn_num#&expandjobs=yes&expandrnipp=N&myjob=&showprog=N&contract_id=#contract_id#"><img src="images/jrr.gif" border="0"></a></td></cfoutput>
         <cfoutput><td align="left">#this_ORG_CODE#</td></cfoutput>
         <cfoutput><td align="left">#this_job_code#</td></tr></cfoutput>
         <cfset cnta = cnta + 1>
         <cfset hold_snum = EMPSSN_NUM>
         <cfset empcnt = 0>
         <cfquery name="q_getmarkersquals" datasource="#data#">
            select qlf_seq_num
              from qual_marker_xref
             where marker_qlf_seq_num=#marker_qlf_seq_num#
         </cfquery>

         <cfloop query="q_getmarkersquals">
            <cfquery name="q_associated_quals" datasource="#data#">
                 select eq.qlf_seq_num,qlf_code,expir_date,qlf_date,qlf_title
                   from emp_qlf eq, qualification q
                  where eq.qlf_seq_num=q.qlf_seq_num
                    and emp_ssn_num='#q_qual.empssn_num#'
                    <cfif NOT COmpareNoCase(sourcetype,'oracle')>
                    and rownum=1
                    </cfif>
                    and eq.qlf_seq_num = #q_getmarkersquals.qlf_seq_num#
                    <cfif NOT COmpareNoCase(sourcetype,'oracle')>
                        and expir_date >= sysdate
                    <cfelse>
                        and expir_date >= getdate()
                    </cfif>
                    and expir_date = (select max(expir_date)
                                        from emp_qlf
                                       where qlf_seq_num = eq.qlf_seq_num and emp_ssn_num=eq.emp_ssn_num)
                  order by eq.qlf_seq_num
            </cfquery>

            <cfloop query="q_associated_quals">
               <cfset empcnt = empcnt + 1>
               <cfoutput><tr><td align="left">&nbsp;&nbsp;<a href="http://trainweb.inel.gov/index.cfm?FuseAction=details&emp_ssn_num=#q_qual.empssn_num#&qlf_seq_num=#q_associated_quals.qlf_seq_num#&startrow=1&maxrows=25&orderby=q.qlf_code&detlevel=less&web=mqgguest">#q_associated_quals.qlf_code#</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td></cfoutput>
               <cfoutput><td align="left">&nbsp;&nbsp;#q_associated_quals.qlf_title#</td></cfoutput>
               <cfoutput><td align="left">&nbsp;&nbsp;#DateFormat(q_associated_quals.qlf_date,'mm/dd/yyyy')#</td></cfoutput>
               <cfoutput><td align="left">&nbsp;&nbsp;#DateFormat(q_associated_quals.expir_date,'mm/dd/yyyy')#</td></tr></cfoutput>
            </cfloop>
         </cfloop>

         <cfif empcnt EQ 0>
            <cfoutput><tr><td align="left" colspan="4"><span class="red">&nbsp;&nbsp;No associated quals</span></td></tr></cfoutput>
         </cfif>
      </cfloop>

   </cfloop>

   <cfoutput></table></cfoutput>

   <cfoutput>Total: #cnta#</cfoutput>
   <cfoutput></pre></td></td></table></div></cfoutput>
   </pre><cfsetting enablecfoutputonly="no">

   <cfset firstopt='setup'>

   <cfinclude template="trainrpt_new_search_inc.cfm">


<cfelse>
   <cflocation url="#whoami#?option=setup">
</cfif>
</body>
</html>
