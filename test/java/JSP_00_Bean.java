/*******************************************************************************
*                                                                              *
*  File:        JSP_00_Bean.java                        Revision:  1.0         *
*                                                                              *
*  Purpose:     a first "JavaBean" to be accessed from within a JSP document   *
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

public class JSP_00_Bean {

/*******************************************************************************
*                                                                              *
*                         Non-Public Instance Variables                        *
*                                                                              *
*******************************************************************************/

  String Message = "";

/*******************************************************************************
*                                                                              *
*                           Public Instance Methods                            *
*                                                                              *
*******************************************************************************/

  public String getMessage () {
    return Message;
  };

  public void setMessage (String Message) {
    this.Message = (Message == null ? "" : Message);
  };
};
