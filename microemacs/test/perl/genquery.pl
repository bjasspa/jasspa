#!C:/unix/usr/Perl/bin/perl -w
# genquery.pl v1.01 
# "Generic" SQL query generator and html form and table generator.  
# Author:Marc Beneteau marc@odbsoft.com  Date:Oct/98

use CGI::Carp qw/fatalsToBrowser/;
package genquery;

#########################
#Open database connection
sub main'dbOpen {
if($remote_db) { 
    #connect through dbi:mysql
    $dbh = DBI->connect("dbi:mysql:$dbname", $dbname, $dbpass)
    || die "Cannot connect to db server $DBI::errstr,\n";
}else{
    #connect through dbi:ODBC
    $dbh = DBI->connect("dbi:ODBC:$dbname", '', '') 
    || die "Cannot connect to db server $DBI::errstr,\n";
}
}

#########################
#Open cursor
sub main'dbExecute {
my ($sql)=@_;
$cursor =$dbh->prepare($sql) 
|| die "Cannot prepare statement: $DBI::errstr\n";
$cursor->execute;
}

#########################
#Fetch
sub main'dbFetch {
$cursor->fetchrow;	 #returns @fields
}

#########################
#Execute statement
sub main'dbDo {
my ($sql)=@_;
my $rows=$dbh->do($sql) || die $dbh->errstr;
$rows;
}

#########################
#Close cursor
sub main'dbCloseCursor {
$cursor->finish;
}

#########################
#Close connection
sub main'dbClose {
$dbh->disconnect;
}

#########################
#Automatically generate an SQL INSERT statement from the names/values of form parameters 
sub main'GenInsert {
my ($q,$table)=@_;
my @fparam=$q->param; #names of all parameters
my $first_where=1;
my $first_col=1;
$sql='insert into '.$table.' (';
$values=' values (';
for($i=0;defined(my $fname=$fparam[$i]);$i++) {
    $fn=lc($fname);
    my $fvalue=$q->param($fname);
    if(length($fvalue)==0) {next;}
    if($fvalue eq '(any)') {$fvalue=''}
    $rw=$reserved_words{$fn};
    if (defined $rw) {next;}
    if($fn eq 'state' || $fn eq 'country') {$fvalue=uc($fvalue)}
    if($first_col) {
        $first_col=0;
    }else{
        $sql.=',';
        $values.=',';
    }
    $sql.=$fname;
    $fvalue=~s/\cM//g;
    $fvalue=~s/\n/<br>/g;
    $values.=$dbh->quote($fvalue);
}
$sql.=')';
$values.=')';
$sql.$values; 
}


##################
#Generate the WHERE clause from form parameters
#Pass in the initial sql statement ('Select abc from xyx')
sub main'GenWhere {
my ($q,$sql)=@_;
my @fparam=$q->param; #names of all parameters
my $first_col=1;
for($i=0;defined(my $fname=$fparam[$i]);$i++) {
    $fn=lc($fname);
    my $fvalue=$q->param($fname);
    if($fvalue eq '(any)') {$fvalue=''}
    if(length($fvalue)==0 || length($fvalue)>80) {next;}
    $rw=$reserved_words{$fn};
    if (defined $rw) {next;}
    if($first_col) {
        $first_col=0; 
        $sql.=" WHERE ";
    }else{
        $sql.=" AND ";
    }
    $sql.=$fname."=";
    #$numeric=$numeric_columns{$fn};
    if(lc($fname) eq 'id' || defined $numeric) { 
        $sql.=$fvalue; #insert numeric value
    } else {
        $sql.=$dbh->quote($fvalue);  #add quotes and escape special chars
    }
}
$sql; 
}

####################
#Generate an SQL UPDATE statement from the names/values of form parameters 
sub main'GenUpdate {
my ($q,$table,$id)=@_;
my @fparam=$q->param; 
my $first_col=1;
$sql='update '.$table.' set ';
for($i=0;defined(my $fname=$fparam[$i]);$i++) {
    $fn=lc($fname);
    $rw=$reserved_words{$fn};
    if (defined $rw) {next;}
    my $fvalue=$q->param($fname);
    if($fn eq 'state' || $fn eq 'country') {$fvalue=uc($fvalue)}
    if($first_col) {
        $first_col=0;
    }else{
        $sql.=',';
    }
    $sql.=$fname."=";
    $fvalue=~s/\cM//g;
    $fvalue=~s/\n/<br>/g;
    $sql.=$dbh->quote($fvalue);
}
$sql.=" where id=".$id;
$sql;
}

# original GenRegexp
sub main'GenRegexp {
my ($q,$sql)=@_;
my ($debug)=0;
my @fparam=$q->param; #names of all parameters
my $first_col=1;
if($sql=~/(^.+\s+)(\w+)(conditions)/) {
    my $sp = $2;
    #require 'dumpvar.pl';
    #main::dumpValue(\@fparam);
    if($sp eq 'client'){
        $s = "c"}
    if($sp eq 'gs'){
        $s = "gs"}
    if($sp eq 'oa'){
        $s = "oa"}
    ###$select1 = "SELECT " . $sp . "conditions." . $s . "cname , " . $sp . "themes." . $s . "tname , " . $sp . "subthemes." . $s . "stname , "; 
    $select1 = "SELECT " . $sp . "conditions.ID, " . $sp . "conditions." . $s . "cname , " . $sp . "themes." . $s . "tname , " . $sp . "subthemes." . $s . "stname , "; 
    $select2 = $sp . "zones." . $s . "zdescription , " . $sp . "conditions." . $s . "cdescription , " . $sp . "conditions." . $s . "ccaseno";
    $from = " FROM " . $sp . "themes , " . $sp . "conditions , " . $sp . "subthemes , " . $sp . "zones ";
    $where1 = "WHERE " . $sp . "conditions.themeid = " . $sp . "themes.ID AND " . $sp . "conditions.subthemeid = " . $sp . "subthemes.ID AND ";
    $where2 = $sp . "conditions.zoneid = " . $sp . "zones.ID";
    $sqlconcat = $select1 . $select2 . $from . $where1 . $where2;
    
    for($i=0;defined(my $fname=$fparam[$i]);$i++) {
        $fn=lc($fname);
        my $fvalue=$q->param($fname);
        my($foundRegexpField) = 1;
        
        if($fvalue eq '(any)') {$fvalue=''}
        ### Warning: if the value queried is more than 80 characters, we skip it
        if(length($fvalue)==0 || length($fvalue)>255) {next;}
        $rw=$reserved_words{$fn};
        if (defined $rw) {
            next;
        } else {
            if($fname=~/cname/){
                $last = $sp . "conditions." . $s . "cname";
                $sqlconcat .= " AND " . $last;
            }
            elsif($fname=~/themeid/){
                $last = $sp . "themes." . $s . "tname";
                $sqlconcat .= " AND " . $last;
            }
            elsif($fname=~/subthemeid/){
                if($debug){ print "<br> in subthemes query loop <br>" }
                $last = $sp . "subthemes." . $s . "stname";
                $sqlconcat .= " AND " . $last;
            }
            elsif($fname=~/zoneid/){
                $last = $sp . "zones." . $s . "zdescription";
                $sqlconcat .= " AND " . $last;
            }
            elsif($fname=~/caseno/)
            {
                $last = $sp . "conditions." . $s . "ccaseno";
                $sqlconcat .= " AND " . $last;
            }
            elsif($fname=~/cdescription/){
                $last = $sp . "conditions." . $s . "cdescription";
                $sqlconcat .= " AND " . $last;
            }
            elsif($fname=~/^id/)
            {
                $last = $sp . "conditions.ID";
                $sqlconcat .= " AND " . $last;
            }
            else{
                $foundRegexpField = 0;
                print "query everything<BR>";
            }
        }
        if ($foundRegexpField) {
            $sqlconcat .= " REGEXP(";
            $sqlconcat .= $dbh->quote($fvalue); 
            $sqlconcat .= ")";
        }
    }
}
elsif($sql=~/(^.+\s+)(\w+)(subthemes)/) {
    my $sp = $2;
    #require 'dumpvar.pl';
    #main::dumpValue(\@fparam);
    if($sp eq 'client'){
        $s = "c"}
    if($sp eq 'gs'){
        $s = "gs"}
    if($sp eq 'oa'){
        $s = "oa"}
    $sqlconcat = "SELECT " . $sp . "subthemes.ID , " . $sp . "subthemes." . $s . "stname , " . $sp . "subthemes.";
    $sqlconcat .= $s . "stdescription FROM " . $sp . "subthemes WHERE clientsubthemes.cstname REGEXP('.*') ";
    
    for($i=0;defined(my $fname=$fparam[$i]);$i++) {
        $fn=lc($fname);
        my $fvalue=$q->param($fname);
        if($fvalue eq '(any)') {$fvalue=''}
        ### Warning: if the value queried is more than 80 characters, we skip it
        if(length($fvalue)==0 || length($fvalue)>255) {next;}
        $rw=$reserved_words{$fn};
        if (defined $rw) {
            next;
        }
        else {
            if($fname=~/stname/){
                $last = $sp . "subthemes." . $s . "stname";
                $sqlconcat .= " AND " . $last; 
            }
            elsif($fname=~/stdescription/){
                $last = $sp . "subthemes." . $s . "stdescription";
                $sqlconcat .= " AND " . $last;
            }
            else{
                print "query everything";
            }
        }
        $sqlconcat .= " REGEXP(";
        $sqlconcat .= $dbh->quote($fvalue); 
        $sqlconcat .= ")";
    }
}
elsif($sql=~/(^.+\s+)(\w+)(themes)/) {
    my $sp = $2;
    #require 'dumpvar.pl';
    #main::dumpValue(\@fparam);
    if($sp eq 'client'){
        $s = "c"}
    if($sp eq 'gs'){
        $s = "gs"}
    if($sp eq 'oa'){
        $s = "oa"}
    $sqlconcat = "SELECT " . $sp . "themes.ID , " . $sp . "themes." . $s . "tname , " . $sp . "themes." . $s . "tdescription FROM " . $sp . "themes ";
    $sqlconcat .= " WHERE clientthemes.ctname REGEXP('.*') ";
    
    for($i=0;defined(my $fname=$fparam[$i]);$i++) {
        $fn=lc($fname);
        my $fvalue=$q->param($fname);
        if($fvalue eq '(any)') {$fvalue=''}
        ### Warning: if the value queried is more than 80 characters, we skip it
        if(length($fvalue)==0 || length($fvalue)>255) {next;}
        $rw=$reserved_words{$fn};
        if (defined $rw) {
            next;
        }
        else {
            if($fname=~/tname/){
                $last = $sp . "themes." . $s . "tname";
                $sqlconcat .= " AND " . $last;
            }
            elsif($fname=~/tdescription/){
                $last = $sp . "themes." . $s . "tdescription";
                $sqlconcat .= " AND " . $last;
            }
            else{
                print "query everything";
            }
        }
        $sqlconcat .= " REGEXP(";
        $sqlconcat .= $dbh->quote($fvalue); 
        $sqlconcat .= ")";
    }
}
elsif($sql=~/(^.+\s+)(\w+)(zones)/) {
    my $sp = $2;
    #require 'dumpvar.pl';
    #main::dumpValue(\@fparam);
    if($sp eq 'client'){
        $s = "c"}
    if($sp eq 'gs'){
        $s = "gs"}
    if($sp eq 'oa'){
        $s = "oa"}
    $sqlconcat = "SELECT " . $sp . "zones.ID , " . $sp . "zones." . $s . "zname , " . $sp . "zones." .$s . "zdescription FROM " . $sp . "zones ";
    $sqlconcat .= " WHERE clientzones.czname REGEXP('.*')" ;
    
    for($i=0;defined(my $fname=$fparam[$i]);$i++) {
        $fn=lc($fname);
        my $fvalue=$q->param($fname);
        if($fvalue eq '(any)') {$fvalue=''}
        ### Warning: if the value queried is more than 80 characters, we skip it
        if(length($fvalue)==0 || length($fvalue)>255) {next;}
        $rw=$reserved_words{$fn};
        if (defined $rw) {
            next;
        }
        else {
            if($fname=~/zname/){
                $last = $sp . "zones." . $s . "zname";
                $sqlconcat .= " AND " . $last;
            }
            elsif($fname=~/zdescription/){
                $last = $sp . "zones." . $s . "zdescription";
                $sqlconcat .= " AND " . $last;
            }
            else{
                print "query everything";
            }
        }
        $sqlconcat .= " REGEXP(";
        $sqlconcat .= $dbh->quote($fvalue); 
        $sqlconcat .= ")";
    }
}
elsif($sql=~/users/) {
    #require 'dumpvar.pl';
    #main::dumpValue(\@fparam);
    $sqlconcat = "SELECT users.ID , users.departmentname , users.firstname , users.lastname , users.email , users.city FROM users ";
    $sqlconcat .= " WHERE users.city REGEXP('.*') ";
    
    for($i=0;defined(my $fname=$fparam[$i]);$i++) {
        $fn=lc($fname);
        my $fvalue=$q->param($fname);
        if($fvalue eq '(any)') {$fvalue=''}
        ### Warning: if the value queried is more than 80 characters, we skip it
        if(length($fvalue)==0 || length($fvalue)>255) {next;}
        $rw=$reserved_words{$fn};
        if (defined $rw) {
            next;
        }
        else {
            if($fname=~/departmentname/){
                $last = " users.departmentname ";
                $sqlconcat .= " AND " . $last ;
            }
            elsif($fname=~/firstname/){
                $last = " users.firstname ";
                $sqlconcat .= " AND " . $last ;
            }
            elsif($fname=~/lastname/){
                $last = " users.lastname ";
                $sqlconcat .= " AND " . $last ;
            }
            elsif($fname=~/email/){
                $last = " users.email ";
                $sqlconcat .= " AND " . $last ;
            }
            elsif($fname=~/city/){
                $last = " users.city ";
                $sqlconcat .= " AND " . $last ;
            }
            else{
                print "query everything";
            }
        }
        $sqlconcat .= " REGEXP(";
        $sqlconcat .= $dbh->quote($fvalue); 
        $sqlconcat .= ")";
    }
}
else {
    print "Your query is not valid";
    #$sql = &main'GenMatch($q,$sql);
    #$sql = &main'GenWhere($q,$sql);
}
return $sqlconcat; 
}



####################
#Generates an html table from a cursor
#Parameter is the name of the cgi to execute for the detail view (one record per page)
sub main'GenTable {
my ($cgipgm,$embed)=@_;
my $cols=$cursor->{NUM_OF_FIELDS};
$n='';
print "<table border=1",$n,">";
my $i=0;
print "\n<tr>";
for($i=0;$i<$cols;$i++) {
    my $fname=$cursor->{NAME}->[$i];
    $datatype=$cursor->{TYPE}->[$i]; #ODBC 1==char 11==date 4==int -1==text
    if($datatype == -1 || $datatype==252) {next} #don't show longvarchars
    $fn=lc($fname);
    if($fn eq 'userkey' || $fn eq 'password' || $fn eq 'date_created' || $fn eq 'date_updated') {next};
    $fname=~s/_/ /g;
    print "<td bgcolor=\"#FFFFCC\"><b>",$fname,"</b></td>";
}
print("</tr>");

my $anyrecs=0;
while(@fields = $cursor->fetchrow) {
    $anyrecs=1;
    print "\n<TR>";
    my $cols=$cursor->{NUM_OF_FIELDS};
    for($i=0;$i<$cols;$i++) {
        $datatype=$cursor->{TYPE}->[$i]; #ODBC 1==char 11==date 4==int -1==text
        if($datatype == -1 || $datatype==252) {next}
        my $fvalue=$fields[$i];
        my $fname=$cursor->{NAME}->[$i];
        #print $fname,"datatype=",$datatype;
        print "<td nowrap>";
        $fn=lc($fname);
        $fn=lc($fname);
        if($fn eq 'userkey' || $fn eq 'password'|| $fn eq 'date_created' || $fn eq 'date_updated') {next};
        if($fn eq 'id') { #generate a submit href
            print "<a href=\"",$cgipgm,"&id=",$fvalue,"\">&nbsp; ",$fields[$i],"&nbsp;</a>";
        } elsif ($fn eq 'link') { #generate a link href
            print '<a href="',$fvalue,'">';
            if(length($fvalue)>0) {print 'click'}
            print '</a>';
        } else {
            print $fvalue;
            #print $datatype;
        }
        print "</td>";
    }
    print("</TR>");
}
print "</table><p>\n";
if(!$anyrecs && !(defined $embed)) {print "<p>No records matched your query - try removing some of your search criteria";}

}

#################
#Generate a full record printout 
sub main'GenFullRecord {
my ($table,$id,$debug)=@_;
$sql = "select * from " . $table . " where id=" . $id;
if(defined $debug) {print "sql=",$sql,"\n";}
$cursor =$dbh->prepare($sql) 
|| die "Cannot prepare statement: $DBI::errstr\n";
$cursor->execute;
@fields = $cursor->fetchrow;
main'printCenter($table.":View record");
print "<table>";
my $cols=$cursor->{NUM_OF_FIELDS};
$prec=0;
for ($i=0;$i<$cols;$i++) {
    $fvalue=$fields[$i];
    if(!defined $fvalue) {$fvalue='';}
    my $fname=$cursor->{NAME}->[$i];
    if($fname eq 'userkey' || $fname eq 'password') {next}
    $fname=~s/_/ /g;
    print "<tr><td align=right valign=top>",$fname,":<td><b>",$fvalue,"</b>";
}
print "</table>";
}

#Support functions
sub printRow{
    my($txt,$val)=@_;
    print "<tr><td align=right>",$txt;
    print "<td>",$val;
}
sub main'printCenter {
my ($txt)=@_;
print "<center><h2>",$txt,"</h2></center>";
}
sub printSelect {
    my ($name,$init,$values)=@_;
    print "<td><select name=",$name,">";
    print "<option value=\"$init\">$init";
    foreach $val(split(/,/,$values)) {
        print "<option value=\"$val\">$val";
    }
    print "</select>\n";
}

####################
#Generate a data-entry form for @fields columns (default '*') from table name
#Optionally fill in form with record_id given
sub main'GenForm {
my ($debug,$table,$id,$fields)=@_;
$do_update=0;
$do_query=0;
if(!(defined $id)) {$id=0}
if($id>0) {$do_update=1}
if($id<0) {$do_query=1}
if(!(defined $fields) || $fields eq '*') {$fields='*'}
$sql = "select " . $fields . " from " . $table;
if($do_update) {$sql.=" where id=" . $id}
main'dbExecute($sql);
if($do_update) {@fields = $cursor->fetchrow}
print "<table><tr valign=top><td colspan=2>";

for($i=0;defined (my $fname=$cursor->{NAME}->[$i]);$i++) {
    #print "$i=",$i,"\n";
    $fn=$lfname=lc($fname);
    $fname=~s/_/ /;
    if($remote_db) {
        $prec=$cursor->{LENGTH}->[$i];
    }else{
        #$prec=$cursor->{PRECISION}->[$i]; doesn't work
        $prec=60;
        if($fn eq 'vegetarian') {$prec=1} #for testing
    }
    $datatype=$cursor->{TYPE}->[$i]; #1==char 11==date 4==int -1==text
    #print "datatype=",$datatype," prec=",$prec,"\n";
    $value='';
    if($do_update) {
        $value=$fields[$i];
    }
    
    ######Special processing of individual fields goes here####
    #if($fn eq 'date_created' || $fn eq 'date_updated' || $fn eq 'name' || $fn eq 'id') {next} 
    #if($fn eq 'date_created' || $fn eq 'date_updated' || $fn eq 'id') {next} 
    if($fn eq 'date_created' || $fn eq 'date_updated') {next} 
    if($fn eq 'id') {
        if($do_query) {
            if($id == -1) {
                printRow('Record ID','<input type=text name=id size=5 maxlength=5>');
            }
            next;
        }
        if(!$do_update) {next;} #don't show id field on add new record
        #not all browsers support readonly text variables, put in hidden field for safety
        print "<tr valign=top><td align=right>Record ID<td><b>",$id,"</b>";
        print '<input type=hidden name=id value="',$id,'">';
    } elsif($fname =~ /(\w+)id$/i) {
        if($do_query) {
            $addon = $1 . ("");
            print "<tr valign=top><td align=right>",$addon;
            print '<td><input type=text name=',$lfname,' value="',$value,
            '" size=80',' maxlength=',$prec,'> ';
            #printRow($1,'<input type=text name=',$lfname,' value="',$value,'" size=80',' maxlength=',$prec,'>';
            next;
        }
        if(!$do_update) {next;} #don't show id field on add new record
        #simple textfield
        print "<tr valign=top><td align=right>",$fname;
        print '<td><input type=text name=',$lfname,' value="',$value,
        '" size=',$prec,' maxlength=',$prec,'> ';
    } elsif(   ($fname =~ /(\w+)(name$)/i) 
            || ($fname =~ /(\w+)(description$)/i)
            || ($fname =~ /(\w+)(caseno$)/i) 
            )
    {
        $fname_sub = $2;
        if($do_query) {
            if(($fname =~ /first/i) || ($fname =~ /last/i))
            {
                print "<tr valign=top><td align=right>",$fname;
                print '<td><input type=text name=',$lfname,' value="',$value,
                '" size=80',' maxlength=',$prec,'> ';
                next;
            }
            elsif($fname =~ /department/i ){ 
                my $dropval=$dropdown{$fn}; #from genquery_site.pl
                if(defined $dropval) {
                    #drop-down list
                    print "<tr valign=top><td align=right>","Department";
                    if($value eq '') {$value='(any)'};
                    printSelect($fn,$value,$dropval);
                }
            }
            elsif($fname_sub eq 'Name') {
                print "<tr valign=top><td align=right>","Number";
                print '<td><input type=text name=',$lfname,' value="',$value,
                '" size=80',' maxlength=',$prec,'> ';
                next;
            }
            else {
                print "<tr valign=top><td align=right>", $fname_sub;
                print '<td><input type=text name=',$lfname,' value="',$value,
                '" size=80',' maxlength=',$prec,'> ';
                next;
            }
        }
        if(!$do_update) {next;} #don't show id field on add new record
        ### simple textfield
        if($prec > 80){ 
            print "<tr valign=top><td align=right>",$fname;
            print '<td><input type=text name=',$lfname,' value="',$value,
            '" size=80 maxlength=',$prec,'> ';
            next;
        } else {
            print "<tr valign=top><td align=right>",$fname;
            print '<td><input type=text name=',$lfname,' value="',$value,
            '" size=',$prec,' maxlength=',$prec,'> ';
        }
    } elsif( ($fn eq 'userkey') && ($do_update) ) {
        if(defined $remote_user){
            #put in hidden field
            print "<tr valign=top><td><input type=hidden name=userkey value=$remote_user>";
        }else{
            #user must enter
            print "<tr valign=top><td align=right>",$fname;
            print '<td><input type=text name=userkey value="" size=',$prec,
            ' maxlength=',$prec,'> ';
        }
    } elsif( ($fn eq 'userkey') && ($do_query) ) {
        next ;
    } else {
        #standard field processing
        my $dropval=$dropdown{$fn}; #from genquery_site.pl
        if(defined $dropval) {
            #drop-down list
            print "<tr valign=top><td align=right>",$fname;
            if($value eq '') {$value='(any)'};
            printSelect($fn,$value,$dropval);
        }elsif($prec==1) {
            #check box
            print "<tr valign=top><td align=right>",$fname;
            print '<td><input type=checkbox name=',$lfname,' value="Y"';
            if($value) {print ' checked'};
            print '> ';
            ###}elsif ($prec<256 && $datatype!=-1) { 
        }elsif ($datatype == -1 || $datatype != -1) { 
            #simple textfield
            #print "<tr valign=top><td align=right>",$fname;
            #print '<td><input type=text name=',$lfname,' value="',$value,
            #'" size=',$prec,' maxlength=',$prec,'> ';
            print "<tr valign=top><td align=right>",$fname;
            print '<td><input type=text name=',$lfname,' value="',$value,
            '" size=80',' maxlength=',$prec,'> ';
        }else{ 
            #textarea
            if($do_query) {next}
            print "<tr valign=top><td align=right valign=top>",$fname;
            $value=~s/<br>/\n/g;
            print '<td><textarea name=',$lfname,' wrap=physical rows=18 cols=80>',$value,
            '</textarea> ';
        }
    }
    #print help text
    $helptext=$formhelp{$fn}; #from genquery_site.pl
    if(defined $helptext) {
        print "&nbsp;&nbsp;",$helptext;
    }
}	
print "</table>";
main'dbCloseCursor();
}
1;
