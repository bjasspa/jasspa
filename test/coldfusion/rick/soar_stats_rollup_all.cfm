<!---
   ||
   || Generate matrix of employees versus completion for the following criteria
   ||
   || SOAR Principles and Concepts  = 0INL1175 + SMCS0037
   || SOAR Observations             = 0INL1174 + SMCS0038
   || Human Performance Awareness   = BEAHUAWR (7492)
   || SOAR Observer Qual            = QLSOAR01 (7194)
   ||
---->
<cfinclude template="soar_courses.cfm" />
<html>
<head><title>SOAR Completion Statistics</title>
<style type="text/css">
<!--
TD {font-family: Arial; font-size: 10pt}
B  {font-family: Arial; font-size: 10t}
TH {font-family: Arial, sans-serif; font-size: 9pt}
P  {font-family: Arial, sans-serif; font-size: 10pt}
H1  {font-family: Arial, sans-serif}
H2  {font-family: Arial, sans-serif;font-weight: bold; font-size: 15pt; color: black}
H3  {font-family: Arial, sans-serif}
H4  {font-family: Arial, sans-serif}

.hover a:hover { font-weight : bold; color: green }

a:link    { color : #009; background : #fff none; }
a:visited { color : #609; background : #fff none; }
a:hover   { color : #f66; background : #fff none; }
a:active  { color : #f00; background : #fff none; }
.title {font-family: Arial, sans-serif; font-weight: bold; font-size: 15pt; color: black}

.super {vertical-align: super}
.bold {font-weight: bold}
.ssbold {font-family: Arial, sans-serif;font-weight: bold; font-size: 9pt; color:black}
.red {color: red}
.blue {color: blue}
.green {color: green}
.brown {color: brown}
.navy {color: navy}
.gray {color: #cccccc}
.white {color: #ffffff}
.bwhite {font-weight: bold;color: #ffffff}
.bbarial {font-family: Arial; font-weight: bold; font-size: 9pt}
.rarial {font-family: Arial; color: red; font-size: 9pt}
.bb {font-weight: bold; font-size: 9pt}
.bbb {font-weight: bold; font-size: 11pt; color: blue}
.bbn {font-family: Arial, sans-serif; font-weight: bold; font-size: 11pt; color: navy}
.bbg {font-weight: bold; font-size: 9pt; color: green}
.bbr {font-weight: bold; font-size: 9pt; color: red}

.bb {font-weight: bold; font-size: 9pt; color: blue}
.bn {font-family: Arial, sans-serif; font-weight: bold; font-size: 9pt; color: navy}
.bg {font-weight: bold; font-size: 9pt; color: green}
.br {font-weight: bold; font-size: 9pt; color: red}

.h3r {font-family: Arial, sans-serif;font-weight: bold; font-size: 14pt; color: red}
.h3g {font-family: Arial, sans-serif;font-weight: bold; font-size: 14pt; color: green}
.h3b {font-family: Arial, sans-serif;font-weight: bold; font-size: 14pt; color: blue}
.h3n {font-family: Arial, sans-serif;font-weight: bold; font-size: 14pt; color: navy}
.vvt {font-family: Arial, sans-serif; font-size: 6pt}
.vt  {font-family: Arial, sans-serif; font-size: 8pt}
.t  {font-family: Arial, sans-serif; font-size: 11pt}
.big {font-family: Arial, sans-serif; font-size: 10pt}
.vbig {font-family: Arial, sans-serif; font-size: 12pt}
.vvbig {font-family: Arial, sans-serif; font-size: 16pt}
.mono {font-family: monospace; font-size: 8pt}
.bmono {font-family: monospace; font-weight: bold; font-size: 8pt}

img.bcblack {
   border-style: solid;
   border-color: ##000000
}

-->
</style>
</head>
<body bgcolor="white">

   <cfparam name="option"            default = "bydept" />
   <cfparam name="soar_princon_ttl"  default = "BBS Principles and Concepts (Percent)" />
   <cfparam name="soar_observr_ttl"  default = "SOAR Observations (Percent)" />
   <cfparam name="soar_huaawar_ttl"  default = "Human Performance Awareness (Percent)" />
   <cfparam name="soar_obsvrql_ttl"  default = "SOAR Observer Qual (Percent)" />
   <cfparam name="soar_huaawar_qual" default = "7492" />
   <cfparam name="soar_observr_qual" default = "7194" />
   <cfparam name="soar_companies"    default = "2392" />


   <cfset title = 'SOAR Employee Training Complete by Directorate' />
   <cfoutput><div align="center">
   <h2>#title#</h2>
   <table width="800" border="1" cellpadding="0" cellspacing="0"></cfoutput>
   <cfoutput><tr><th valign="bottom" width="08%">Division</th>
                 <th valign="bottom" width="40%">Division Title</th>
                 <th valign="bottom" width="13%" colspan="2">#soar_huaawar_ttl#</th>
                 <th valign="bottom" width="13%" colspan="2">#soar_princon_ttl#</th>
                 <th valign="bottom" width="13%" colspan="2">#soar_observr_ttl#</th>
                 <th valign="bottom" width="13%" colspan="2">#soar_obsvrql_ttl#</th>
             </tr></cfoutput>


   <cfset princon_pct = 0.0 />
   <cfset observr_pct = 0.0 />
   <cfset huaawar_pct = 0.0 />
   <cfset obsvrql_pct = 0.0 />

   <cfset princon_ttl = 0.0 />
   <cfset observr_ttl = 0.0 />
   <cfset huaawar_ttl = 0.0 />
   <cfset obsvrql_ttl = 0.0 />
   <cfset emp_total   = 0/>

   <cfquery name="q_get_dept" datasource="#data#">
      select rpad(org_id,4,'0') as print_org_id, org_title, org_id
         from org
         where (length(org_id) = 3) and org_id like 'IL%';
   </cfquery>

   <cfloop query="q_get_dept">
      <cfquery name="q_get_emp_dept_count" datasource="#data#">
         select emp_ssn_num
            from employee e inner join org o on e.org_code = o.org_code
            where co_code in (#soar_companies#)
            and o.org_id like '#q_get_dept.org_id#' ||'%'
      </cfquery>

      <cfset emp_count = q_get_emp_dept_count.RecordCount />
      <cfset emp_total = emp_total + emp_count />

      <cfquery name="q_get_princon" datasource="#data#" >
         select distinct emp_ssn_num
            from employee e inner join org o on e.org_code = o.org_code
            where emp_ssn_num in (select emp_ssn_num
                                    from class_attend
                                    where class_code in (select class_code
                                                         from class
                                                       where course_code in (#PreserveSingleQuotes(principles_and_concepts_courses)#))
                                       and class_attend_status_code in ('C','G','T','O'))
            and co_code in (#soar_companies#)
            and emp_active_stat_flag = 'A'
            and o.org_id like '#q_get_dept.org_id#' ||'%'
            order by emp_ssn_num
      </cfquery>

      <cfset princon_count = q_get_princon.RecordCount />

      <cfquery name="q_get_observr" datasource="#data#" >
         select distinct emp_ssn_num
            from employee e inner join org o on e.org_code = o.org_code
            where emp_ssn_num in (select emp_ssn_num
                                    from class_attend
                                 where class_code in (select class_code
                                                        from class
                                                       where course_code in (#PreserveSingleQuotes(observer_courses)#))
                                    and class_attend_status_code in ('C','G','T','O'))
            and co_code in (#soar_companies#)
            and emp_active_stat_flag = 'A'
            and o.org_id like '#q_get_dept.org_id#' ||'%'
            order by emp_ssn_num
      </cfquery>

      <cfset observr_count = q_get_observr.RecordCount />

      <cfquery name="q_get_huaawar" datasource="#data#" >
         select distinct emp_ssn_num
            from employee e inner join org o on e.org_code = o.org_code
                           inner join emp_qlf eq on e.emp_ssn_num = eq.emp_ssn_num
            where qlf_seq_num in (#soar_huaawar_qual#)
            and co_code in (#soar_companies#)
            and emp_active_stat_flag = 'A'
            and o.org_id like '#q_get_dept.org_id#' ||'%'
            order by emp_ssn_num;
      </cfquery>

      <cfset huaawar_count = q_get_huaawar.RecordCount />

      <cfquery name="q_get_obsvrql" datasource="#data#" >
         select distinct emp_ssn_num
            from employee e inner join org o on e.org_code = o.org_code
                           inner join emp_qlf eq on e.emp_ssn_num = eq.emp_ssn_num
            where qlf_seq_num in (#soar_observr_qual#)
            and co_code in (#soar_companies#)
            and emp_active_stat_flag = 'A'
            and o.org_id like '#q_get_dept.org_id#' ||'%'
            order by emp_ssn_num;
      </cfquery>

      <cfset obsvrql_count = q_get_obsvrql.RecordCount />

      <cftry>
         <cfset princon_pct = (princon_count/emp_count) * 100.0 />
         <cfset observr_pct = (observr_count/emp_count) * 100.0 />
         <cfset huaawar_pct = (huaawar_count/emp_count) * 100.0 />
         <cfset obsvrql_pct = (obsvrql_count/emp_count) * 100.0 />
         <cfcatch type="any">
            <cfset princon_pct = 0.0 />
            <cfset observr_pct = 0.0 />
            <cfset huaawar_pct = 0.0 />
            <cfset obsvrql_pct = 0.0 />
         </cfcatch>
      </cftry>

      <cfif emp_count GT 0>
         <cfset princon_ttl = princon_ttl + princon_count />
         <cfset observr_ttl = observr_ttl + observr_count />
         <cfset huaawar_ttl = huaawar_ttl + huaawar_count />
         <cfset obsvrql_ttl = obsvrql_ttl + obsvrql_count />

         <cfscript>
         if (princon_count is 0) {
            princon_pcts = '';
         } else {
            princon_pcts = NumberFormat(princon_pct,'999') &  ' ';
         }
         if (observr_count is 0) {
            observr_pcts = '';
         } else {
            observr_pcts = NumberFormat(observr_pct,'999') &  ' ';
         }
         if (huaawar_count is 0) {
            huaawar_pcts = '';
         } else {
            huaawar_pcts = NumberFormat(huaawar_pct,'999') &  ' ';
         }
         if (obsvrql_count is 0) {
            obsvrql_pcts = '';
         } else {
            obsvrql_pcts = NumberFormat(obsvrql_pct,'999') &  ' ';
         }
         </cfscript>


         <cfoutput>
                  <tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>
                  <tr><td align="right">#mid(q_get_dept.org_id,3,2)#000&nbsp;&nbsp;&nbsp;</td>
                       <td align="left">#q_get_dept.org_title#</td>
                       <td align="right">#huaawar_pcts#&nbsp;</td></td><td valign="top" align="right">(#huaawar_count#/#emp_count#)</td>
                       <td align="right">#princon_pcts#&nbsp;</td></td><td valign="top" align="right">(#princon_count#/#emp_count#)</td>
                       <td align="right">#observr_pcts#&nbsp;</td></td><td valign="top" align="right">(#obsvrql_count#/#emp_count#)</td>
                       <td align="right">#obsvrql_pcts#&nbsp;</td></td><td valign="top" align="right">(#observr_count#/#emp_count#)</td>
                   </tr>
                   </cfoutput>
      </cfif>

   </cfloop>
   <cftry>
      <cfset princon_pct = (princon_ttl/emp_total) * 100.0 />
      <cfset observr_pct = (observr_ttl/emp_total) * 100.0 />
      <cfset huaawar_pct = (huaawar_ttl/emp_total) * 100.0 />
      <cfset obsvrql_pct = (obsvrql_ttl/emp_total) * 100.0 />
      <cfcatch type="any">
         <cfset princon_pct = 0.0 />
         <cfset observr_pct = 0.0 />
         <cfset huaawar_pct = 0.0 />
         <cfset obsvrql_pct = 0.0 />
      </cfcatch>
   </cftry>
   <cfset princon_pcts = NumberFormat(princon_pct,'999') &  ' ' />
   <cfset observr_pcts = NumberFormat(observr_pct,'999') &  ' ' />
   <cfset huaawar_pcts = NumberFormat(huaawar_pct,'999') &  ' ' />
   <cfset obsvrql_pcts = NumberFormat(obsvrql_pct,'999') &  ' ' />
   <cfoutput>
            <tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>
            <tr><td align="right"><b>Totals</b>&nbsp;&nbsp;&nbsp;</td>
                <td align="left">&nbsp;</td>
                <td align="right">#huaawar_pcts#&nbsp;</td><td valign="top" align="right">(#huaawar_ttl#/#emp_total#)</td>
                <td align="right">#princon_pcts#&nbsp;</td><td valign="top" align="right">(#princon_ttl#/#emp_total#)</td>
                <td align="right">#observr_pcts#&nbsp;</td><td valign="top" align="right">(#obsvrql_ttl#/#emp_total#)</td>
                <td align="right">#obsvrql_pcts#&nbsp;</td><td valign="top" align="right">(#observr_ttl#/#emp_total#)</td>
            </tr></cfoutput>
   <cfoutput></table>
   <form name="myform" action="#cgi.script_name#"><input type="hidden" name="option" value="byarea">
   <input type="submit" value="Show by Work Location"></form>
   </div></cfoutput>
</body>
</html>

