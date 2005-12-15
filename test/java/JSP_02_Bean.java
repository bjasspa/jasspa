/*******************************************************************************
*                                                                              *
*  File:        JSP_02_Bean.java                        Revision:  1.0         *
*                                                                              *
*  Purpose:     a "JavaBean" which displays the contents of several predefined *
*               JSP variables                                                  *
*                                                                              *
*  Creation:    09.09.2000                     Last Modification:  09.09.2000  *
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

import java.util.*;                  // Java utility classes and data structures
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.jsp.*;

public class JSP_02_Bean {

/*******************************************************************************
*                                                                              *
*                         Non-Public Instance Variables                        *
*                                                                              *
*******************************************************************************/

  StringBuffer Result      = new StringBuffer();
  boolean      oddTableRow = false;

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

  void print (String Text) {
    Result.append(Text);
  };

/*******************************************************************************
*                                                                              *
* println             appends the given text to "Result" and begins a new line *
*                                                                              *
*******************************************************************************/

  void println (String Text) {
    Result.append(Text);
    Result.append("\n");
  };

/*******************************************************************************
*                                                                              *
* showParameter                      displays a given parameter with its value *
*                                                                              *
*******************************************************************************/

  void showParameter (String Name, String Value) {

  /**** beware of values with multiple lines ****/

    if (Value != null) {
      if ((Value.indexOf('\r') >= 0) || (Value.indexOf('\n') >= 0)) {
        StringTokenizer Parser = new StringTokenizer(Value, "\r\n");
        switch (Parser.countTokens()) {
          case 0:  Value = "";
                   break;
          case 1:  Value = Parser.nextToken();
                   break;
          default: showParameter(Name, Parser.nextToken());
                   while (Parser.hasMoreTokens()) {
                     showParameter(null, Parser.nextToken());
                   };
                   return;
        };
      };
    };

  /**** now handle missing values or those without CR or LF ****/

    println("                <tr valign=top bgcolor=" + (oddTableRow ? "#CCFFCC" : "white") + ">");
    println("                  <td align=left><img src=\"PaperHole.gif\"></td>");
      if (Name == null) {
        println("                  <td></td>");
      } else {
        println("                  <td align=right><b><tt>" + Name + "</tt></b>:&nbsp;</td>");
      };
    print  ("                  <td align=left>");
      if (Value == null) {
        print("<i>(not available)</i>");
      } else {
        print("\"" + Value + "\"");
      };
    println("</td>");
    println("                  <td align=right><img src=\"PaperHole.gif\"></td>");
    println("                </tr>");

    oddTableRow = !oddTableRow;
  };

/*******************************************************************************
*                                                                              *
*                           Public Instance Methods                            *
*                                                                              *
*******************************************************************************/

  public String examineApplication (ServletContext Application) {
    Result.setLength(0);
      showParameter("DOCUMENT_ROOT",   Application.getRealPath("/"));
      showParameter("SERVER_SOFTWARE", Application.getServerInfo());
    return Result.toString();
  };

  public String examineConfig (ServletConfig Config) {
    Result.setLength(0);
    return Result.toString();
  };

  public String examineOut (JspWriter Out) {
    Result.setLength(0);
    return Result.toString();
  };

  public String examinePageContext (PageContext Context) {
    Result.setLength(0);
    return Result.toString();
  };

  public String examineRequest (HttpServletRequest Request) {
    Result.setLength(0);
      showParameter("AUTH_TYPE",       Request.getAuthType());
      showParameter("CONTENT_LENGTH",  String.valueOf(Request.getContentLength()));
      showParameter("CONTENT_TYPE",    Request.getContentType());
      showParameter("PATH_INFO",       Request.getPathInfo());
      showParameter("PATH_TRANSLATED", Request.getPathTranslated());
      showParameter("QUERY_STRING",    Request.getQueryString());
      showParameter("REMOTE_ADDR",     Request.getRemoteAddr());
      showParameter("REMOTE_HOST",     Request.getRemoteHost());
      showParameter("REMOTE_USER",     Request.getRemoteUser());
      showParameter("REQUEST_METHOD",  Request.getMethod());
      showParameter("SCRIPT_NAME",     Request.getServletPath());
      showParameter("SERVER_NAME",     Request.getServerName());
      showParameter("SERVER_PORT",     String.valueOf(Request.getServerPort()));
      showParameter("SERVER_PROTOCOL", Request.getProtocol());
    return Result.toString();
  };

  public String examineRequestParameters (HttpServletRequest Request) {
    Result.setLength(0);
      Enumeration NameList = Request.getParameterNames();
      if (NameList.hasMoreElements()) {
        while (NameList.hasMoreElements()) {
          String   ParameterName = (String) NameList.nextElement();
          String[] ValueList = Request.getParameterValues(ParameterName);

          if (ValueList.length == 0) {
            showParameter(ParameterName, null);
          } else {
            showParameter(ParameterName, ValueList[0]);
            for (int i = 1; i < ValueList.length; i++) {
              showParameter(null, ValueList[i]);
            };
          };
        };
      } else {
        showParameter(null, "<i>(none)</i>");
      };
    return Result.toString();
  };

  public String examineResponse (HttpServletResponse Response) {
    Result.setLength(0);
    return Result.toString();
  };

  public String examineSession (HttpSession Session) {
    Result.setLength(0);
    return Result.toString();
  };
};
