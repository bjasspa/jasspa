/*******************************************************************************
*                                                                              *
*  File:        JSP_03_Bean.java                        Revision:  1.0         *
*                                                                              *
*  Purpose:     a "JavaBean"  which loads a given (plain text) file, processes *
*               its contents in order to make it HTML-compliant  and creates a *
*               "preformatted paragraph" for the result                        *
*                                                                              *
*  Creation:    10.09.2000                     Last Modification:  10.09.2000  *
*                                                                              *
*  Platform:    IBM-compatible PC running Windows 98SE                         *
*                                                                              *
*  Environment: Java 1.3                                                       *
*                                                                              *
*  Author:      Andreas Rozek           Phone:  ++49 (711) 6770682             *
*               Kirschblütenweg 15      Fax:    -                              *
*             D-70569 Stuttgart         EMail:  Andreas.Rozek@T-Online.De      *
*               Germany                                                        *
*                                                                              *
*  URL:         http://Home.T-Online.De/home/Andreas.Rozek                     *
*                                                                              *
*  Copyright:   this software is published under the "GNU General Public Li-   *
*               cense" (see "http://www.gnu.org/copyleft/gpl.html" for addi-   *
*               tional information)                                            *
*                                                                              *
*  Comments:    (none)                                                         *
*                                                                              *
*******************************************************************************/

package JSPAcquainting;

import java.io.*;                            // Java file handling and stream IO
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.jsp.*;

public class JSP_03_Bean {

/*******************************************************************************
*                                                                              *
*                         Non-Public Instance Variables                        *
*                                                                              *
*******************************************************************************/

  String       BasePath = "";
  StringBuffer Result   = new StringBuffer();

/*******************************************************************************
*                                                                              *
*                         Non-Public Instance Methods                          *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*                                                                              *
* print                                     appends the given text to "Result" *
*                                                                              *
*******************************************************************************/

  void print (char Character) {
    Result.append(Character);
  };

  void print (String Text) {
    Result.append(Text);
  };

/*******************************************************************************
*                                                                              *
* println             appends the given text to "Result" and begins a new line *
*                                                                              *
*******************************************************************************/

  void println () {
    Result.append("<br>\n");
  };

  void println (String Text) {
    print(Text);
    println();
  };

/*******************************************************************************
*                                                                              *
*                           Public Instance Methods                            *
*                                                                              *
*******************************************************************************/

  public String insertText (String FileName) {
    Result.setLength(0);
      try {
        BufferedReader InStream = new BufferedReader(new FileReader(BasePath+FileName));
          print("<pre>");
            String InLine = InStream.readLine();
            while (InLine != null) {
              if (InLine.length() > 80) {
                InLine = InLine.substring(0,73) + "...";
              };

              print("<tt>");
                for (int i = 0; i < InLine.length(); i++) {  // poor approach...
                  char Character = InLine.charAt(i);
                  if ((Character < 32) || ((Character >= 127) && (Character < 160))) {
                    print("^" + (int) Character);
                  } else switch (Character) {
                    case '&': print("&amp;");  break;
                    case '"': print("&quot;"); break;
                    case '<': print("&lt;");   break;
                    case '>': print("&gt;");   break;
                    default:  print(Character);
                  };
                };
              println("</tt>");
            };
          println("</pre>");
        InStream.close();
      } catch (Exception Signal) {
        if (Result.length() > 0) {
          println();
        };

        print  ("<h3><font color=red>IOException</font></h3>");
        println("An error occurred while reading file \"" + BasePath + "JSP_03.txt\"");

        String Message = Signal.getMessage();
        if (Message == null) {
          println("(Reason: unknown)");
        } else {
          println("(Reason: \"" + Signal.getMessage() + "\")");
        };
      };
    return Result.toString();
  };


  public String setBasePath (String RootPath, String ServletPath) {
    String fullServletPath = RootPath + ServletPath.replace('/',File.separatorChar);
    int    SeparatorPos    = fullServletPath.lastIndexOf(File.separatorChar);
      if (SeparatorPos > 0) {
        BasePath = fullServletPath.substring(0,SeparatorPos+1);
      } else {
        BasePath = RootPath;
      };
    return "";
  };
};
