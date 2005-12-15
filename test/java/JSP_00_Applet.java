/*******************************************************************************
*                                                                              *
*  File:        JSP_00_Applet.java                      Revision:  1.0         *
*                                                                              *
*  Purpose:     an applet to be inserted in a JSP document using <jsp:plugin>  *
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

import java.applet.*;                                    // basic applet classes
import java.awt.*;                           // basic AWT classes (& interfaces)

public class JSP_00_Applet extends Applet {

/**** information about author, version and copyright of this applet ****/

  final static String AppletInfo =
    "JSP_00_Applet - an applet for the Java Server Page \"JSP_00.htm\"\n" +
    "\n" +
    "Andreas Rozek\n" +
    "Kirschblütenweg 15\n" +
    "D-70569 Stuttgart\n" +
    "Germany\n" +
    "\n" +
    "Phone: ++49 (711) 6770682\n" +
    "Fax: -\n" +
    "EMail: Andreas.Rozek@T-Online.De";

/**** information about foreseen applet parameters ****/

  final static String[][] ParameterInfo = null;

/*******************************************************************************
*                                                                              *
*                           Public Instance Methods                            *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*                                                                              *
* destroy                         cleans up before the applet will be unloaded *
*                                                                              *
*******************************************************************************/

  public void destroy() {
  };

/*******************************************************************************
*                                                                              *
* getAppletInfo       provides information about author, version and copyright *
*                                                                              *
*******************************************************************************/

  public String getAppletInfo() {
    return AppletInfo;               // see above for definition of "AppletInfo"
  };

/*******************************************************************************
*                                                                              *
* getParameterInfo       provides information about foreseen applet parameters *
*                                                                              *
*******************************************************************************/

  public String[][] getParameterInfo() {
    return ParameterInfo;         // see above for definition of "ParameterInfo"
  };

/*******************************************************************************
*                                                                              *
* init                 initializes an applet each time it's loaded or reloaded *
*                                                                              *
*******************************************************************************/

  public void init() {
  };

/*******************************************************************************
*                                                                              *
* paint                                              (re)draws applet contents *
*                                                                              *
*******************************************************************************/

  public void paint (Graphics Graf) {
    int Width  = getWidth();
    int Height = getHeight();

    Graf.setColor(Color.black);
    Graf.fillRect(0,0, Width,Height);

    Graf.setColor(Color.black);
    Graf.drawRect(5,5, 590,390);
    Graf.drawLine(0,0, 600,400);
    Graf.drawLine(0,400, 600,0);
  };

/*******************************************************************************
*                                                                              *
* start                     starts an applet when its document comes into view *
*                                                                              *
*******************************************************************************/

  public void start() {                          // may be called several times!
  };

/*******************************************************************************
*                                                                              *
* stop                      stops an applet when its document gets out of view *
*                                                                              *
*******************************************************************************/

  public void stop() {                           // may be called several times!
  };
};
