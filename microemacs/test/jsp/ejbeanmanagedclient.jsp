<!-- Copyright (c) 1999-2000 by BEA Systems, Inc. All Rights Reserved.-->

<!-- Here we use a Java string ("PAGETITLE") to set 
     the same phrase as the title and as a head.-->
<html>
<head>
<title>
<%= pagetitle %>
</title>
</head>

<body bgcolor=#FFFFFF>

<font face="Helvetica">

<h2>
<font color=#DB1260>
<%= pagetitle %>
</font>
</h2>

<!-- Using the "import" feature -->

<%@ page import="
    javax.naming.*,
    javax.ejb.*,
    java.rmi.RemoteException,
    java.rmi.Remote,
    java.util.*,
    examples.ejb.basic.beanManaged.* 
"%>

<!-- Here, we declare a class method -->

<%!
  String pagetitle = "JSP example using EJBean-managed persistence";
  String url       = "t3://localhost:7001";
  String accountId = "10020";

  // Declaring a Java class
  public Context getInitialContext() throws Exception {
    Properties p = new Properties();
    p.put(Context.INITIAL_CONTEXT_FACTORY,
          "weblogic.jndi.WLInitialContextFactory");
    p.put(Context.PROVIDER_URL, url);
    return new InitialContext(p);
  }
    
  String getStackTraceAsString(Exception e)
  {
    // Dump the stack trace to a buffered stream, then send it's contents
    // to the JSPWriter. 
    ByteArrayOutputStream ostr = new ByteArrayOutputStream();
    e.printStackTrace(new PrintWriter(ostr));
    return(ostr.toString());
  }
%>

<!-- Note that the following Java code will be inserted into the body
of the servlet -->

<%
  double amount  = 100;
  double balance = 3000;

  AccountPK accountKey = new AccountPK();
  accountKey.accountId = accountId;
  try {
    // Contact the AccountBean container (the "AccountHome") through JNDI.

    Context ctx = getInitialContext();
    AccountHome home = (AccountHome) ctx.lookup("beanManaged.AccountHome");

%>

<p>
<b>Looking up account <%= accountId %> ...</b>

<%
    Account ac = null;
    try {
      ac = (Account) home.findByPrimaryKey(accountKey);
    } 
    catch (Exception ee) {
        out.println("<br>Did not find "+ accountId);
    }

    if (ac == null) {
      out.print("<br>Account " + accountId + 
                " being created; opening balance is $" + balance);
      ac = home.create(accountId, balance);      
    } 
    else {
      out.print("<br>Account " + accountId +
                " found; balance is $" + ac.balance());
    }
%>

<h2>Part A:</h2> 
Deposit and attempt to withdraw more than the current
account balance. An application-specific exception should be thrown:-
<p>

Current Balance: $ <%= ac.balance() %> <br>
Depositing: $ <%= amount %>
<p>

<%
    // Deposit the amount into the account
    balance = ac.deposit(amount);
%>

New balance: $ <%= balance %>

<p>
<h2>PartB:</h2>
Withdraw an amount greater than 
current balance. Expecting an exception...
<p>

<%
    amount = balance + 10;
    try {
      balance = ac.withdraw(amount);
      out.print("Error: An exception should have been thrown.");
    } 
    catch (ProcessingErrorException pe) {
      out.print("Received expected Processing Error:" + pe);
      out.print("<pre>" + getStackTraceAsString(pe) + "</pre>");
    }
%>      

<h2>Part C</h2> 
Create some new accounts, with different initial balances.
Find all the accounts with a balance greater than a specific value.
When finished, the new accounts are removed.
<p>

<%
    int numAccounts = 5;
    long now = System.currentTimeMillis();
    Vector v = new Vector();
    
    for (int i = 0; i < numAccounts; i++) {
      String id = "" + now + i;     // create unique account  id
      balance = i*100;              // initial balance
      v.addElement(home.create(id, balance)); 
%>

<br>Created account: <%= id %> with  balance: $ <%= balance %>

<%
    } // end of creating accounts for loop

    if (v.size() == numAccounts) {
      out.print("<p>Success: " + numAccounts + " accounts successfully created");
    }
    else {
      out.print("<p>Error: Only " + v.size() + 
                " accounts were created successfully");
    }
    
%>

<%
    double balanceGreaterThan = 700;
%>

<p>Querying for accounts with a balance greater than <%= balanceGreaterThan %> 
<table><tr><th Account ID><th Balance></tr>

<%
    Enumeration e = home.findBigAccounts(balanceGreaterThan);
    if (e != null) {
      while (e.hasMoreElements()) {
        Account bigAccount= (Account) e.nextElement();
%>
<tr>
    <td><%= bigAccount.getPrimaryKey() %></td>
    <td><%= bigAccount.balance() %></td>
</tr>
<%
        // Thread.sleep(1000);
      }
    }
%>
</table>

<p>Now Removing the accounts we just created...

<%
    for (int i = 0; i < numAccounts; i++) {
      String id = String.valueOf(now) + String.valueOf(i); 
      ((Account)(v.elementAt(i))).remove();
      out.print("<br>Removed account: " +id);
    }

  // Catch exceptions
  } 
  catch (ProcessingErrorException pe) {
    out.print("<p>Unexpected Processing Error: " + pe +
              "<pre>"+getStackTraceAsString(pe)+"</pre>");
  }
  catch (Exception e) {
    out.print("<p>:::::::::::::: Unexpected Error :::::::::::::::::");
    out.print("<pre>"+getStackTraceAsString(e)+"</pre>");
  }
  finally {
%>

<p>Completed EJB operations at <%= new Date() %>
<p><hr width=80%>

<%
  }
%>

<p>
<font size=-1>
Copyright &copy; 1999-2000 by BEA Systems, Inc. All Rights Reserved.
</font>

</font>
</body>
</html>


