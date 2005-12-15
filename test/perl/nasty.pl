#!C:/unix/usr/Perl/bin/perl -w

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
$cursor->fetchrow; #returns @fields
}

#########################
#Execute statement
sub main'dbDo {
my ($sql)=@_;
my $rows=$dbh->do($sql) || die $dbh->errstr;
$rows;
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

