/* available online in file 'sample5' */
#include <stdio.h>
   char    buf[20];
EXEC SQL BEGIN DECLARE SECTION;
   int     acct;
   double  debit;
   double  new_bal;
   VARCHAR status[65];
   VARCHAR uid[20];
   VARCHAR pwd[20];
EXEC SQL END DECLARE SECTION;

EXEC SQL INCLUDE SQLCA;

main()
{
   extern double atof();

   strcpy (uid.arr,"scott");
   uid.len=strlen(uid.arr);
   strcpy (pwd.arr,"tiger");
   pwd.len=strlen(pwd.arr);

   printf("\n\n\tEmbedded PL/SQL Debit Transaction Demo\n\n");
   printf("Trying to connect...");
   EXEC SQL WHENEVER SQLERROR GOTO errprint;
   EXEC SQL CONNECT :uid IDENTIFIED BY :pwd;
   printf(" connected.\n");
for (;;)       /*  Loop infinitely */
{
   printf("\n** Debit which account number? (-1 to end) ");
   gets(buf);
   acct = atoi(buf);
   if (acct == -1)  /* Need to disconnect from Oracle */
   {                /* and exit loop if account is -1 */
       EXEC SQL COMMIT RELEASE;
       exit(0);
   }

   printf("   What is the debit amount? ");
   gets(buf);
   debit = atof(buf);

   /* ---------------------------------- */
   /* ----- Begin the PL/SQL block ----- */
   /* ---------------------------------- */
   EXEC SQL EXECUTE

   DECLARE
      insufficient_funds EXCEPTION;
      old_bal            NUMBER;
      min_bal            CONSTANT NUMBER := 500;
   BEGIN
      SELECT bal INTO old_bal FROM accounts
         WHERE account_id = :acct;
         -- If the account doesn't exist, the NO_DATA_FOUND
         -- exception will be automatically raised.
      :new_bal := old_bal - :debit;
      IF :new_bal >= min_bal THEN
         UPDATE accounts SET bal = :new_bal
            WHERE account_id = :acct;
         INSERT INTO journal
            VALUES (:acct, 'Debit', :debit, SYSDATE);
         :status := 'Transaction completed.';
      ELSE
         RAISE insufficient_funds;
      END IF;
      COMMIT;
   EXCEPTION
      WHEN NO_DATA_FOUND THEN
         :status := 'Account not found.';
         :new_bal := -1;
      WHEN insufficient_funds THEN
         :status := 'Insufficient funds.';
         :new_bal := old_bal;
      WHEN OTHERS THEN
         ROLLBACK;
         :status := 'Error: ' || SQLERRM(SQLCODE);
         :new_bal := -1;
   END;
  
   END-EXEC;
   /* -------------------------------- */
   /* ----- End the PL/SQL block ----- */
   /* -------------------------------- */

   status.arr[status.len] = '\0';  /* null-terminate */
                                   /* the string     */
   printf("\n\n   Status:  %s\n", status.arr);
   if (new_bal >= 0)
      printf("   Balance is now:  $%.2f\n", new_bal);
}  /* End of loop */

errprint:
   EXEC SQL WHENEVER SQLERROR CONTINUE;
   printf("\n\n>>>>> Error during execution:\n");
   printf("%s\n",sqlca.sqlerrm.sqlerrmc);
   EXEC SQL ROLLBACK RELEASE;
   exit(1);
}
