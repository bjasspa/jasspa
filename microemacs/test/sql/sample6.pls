

/* available online in file 'sample6' */
#include <stdio.h>
#include <string.h>

typedef char asciz;

EXEC SQL BEGIN DECLARE SECTION;
   /* Define type for null-terminated strings. */
   EXEC SQL TYPE asciz IS STRING(20);
   asciz  username[20];
   asciz  password[20];
   int    dept_no;    /* which department to query */
   char   emp_name[10][21];
   char   job[10][21];
   EXEC SQL VAR emp_name is STRING (21);
   EXEC SQL VAR job is STRING (21);
   float  salary[10];
   int    done_flag;
   int    array_size;
   int    num_ret;    /* number of rows returned */
   int    SQLCODE;
EXEC SQL END DECLARE SECTION;

EXEC SQL INCLUDE sqlca;

int print_rows();       /* produces program output      */
int sqlerror();         /* handles unrecoverable errors */

main()
{
   int i;

   /* Connect to Oracle. */
   strcpy(username, "SCOTT");
   strcpy(password, "TIGER");

   EXEC SQL WHENEVER SQLERROR DO sqlerror();

   EXEC SQL CONNECT :username IDENTIFIED BY :password;
   printf("\nConnected to Oracle as user: %s\n\n", username);

   printf("Enter department number: ");
   scanf("%d", &dept_no);
   fflush(stdin);

   /* Print column headers. */
   printf("\n\n");
   printf("%-10.10s%-10.10s%s\n", "Employee", "Job", "Salary");
   printf("%-10.10s%-10.10s%s\n", "--------", "---", "------");

   /* Set the array size. */
   array_size = 10;
   done_flag = 0;
   num_ret = 0;

   /* Array fetch loop - ends when NOT FOUND becomes true. */
   for (;;)
   {
      EXEC SQL EXECUTE
         BEGIN personnel.get_employees
            (:dept_no, :array_size, :num_ret, :done_flag,
            :emp_name, :job, :salary);
         END;
      END-EXEC;

      print_rows(num_ret);

      if (done_flag)
         break;
   }

   /* Disconnect from Oracle. */
   EXEC SQL COMMIT WORK RELEASE;
   exit(0);
}

print_rows(n)
int n;
{
   int i;

   if (n == 0)
   {
      printf("No rows retrieved.\n");
      return;
   }

   for (i = 0; i < n; i++)
      printf("%10.10s%10.10s%6.2f\n",
         emp_name[i], job[i], salary[i]);
}
sqlerror()
{
   EXEC SQL WHENEVER SQLERROR CONTINUE;
   printf("\nOracle error detected:");
   printf("\n% .70s \n", sqlca.sqlerrm.sqlerrmc);
   EXEC SQL ROLLBACK WORK RELEASE;
   exit(1);
}
