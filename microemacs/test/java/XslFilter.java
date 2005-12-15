package cuckoo;
import java.io.*;
import javax.servlet.*;
import javax.servlet.http.*;
/**
 * Cuckoo Resin filter.
 * <p>Copyright (c) 2001-2002</p>
 * <pre>
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * </pre>
 * @author  Alexis Grandemange
 * @version 0, 0, 1
 */
public class XslFilter implements Filter {
  /** Keep the Filter config. */
  private FilterConfig config;
  /** Set the Filter config. */
  public void init(FilterConfig config) {
    this.config = config;
  }
  /** Preprocesses the request. */
  public void doFilter(ServletRequest request, ServletResponse response,
    FilterChain next) throws IOException, ServletException {
    HttpServletRequest req = (HttpServletRequest)request;
    String style = req.getParameter("style");
    req.setAttribute("caucho.xsl.stylesheet", style);
    next.doFilter(request, response);
  }
  public void destroy() {
  }
} 