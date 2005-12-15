<?php
 
/* $Id: worker.inc,v 1.1 2005-12-15 22:36:48 bill Exp $ */
 
require_once 'db.inc';

class WORKER
{

  var $worker;  // objects in our WORKER class

  function step1Page ()
  {
   global $qa_func;
   ?>
   <center>
   <table cellpadding=1 cellspacing=0 border=0 bgcolor=#40637A width=95% align=center>
   <tr>
    <td>
    <table cellpadding=3 cellspacing=0 border=0 bgcolor=#FFFFFF width=100%>
    <tr>
     <td>
     <table cellpadding=0 cellspacing=0 border=0 width=90% align=center>
     <tr>
      <td width=70% align=center><b>ProductView QA Workers</b></td>
     </tr>
     </table>
     &nbsp;&nbsp;&nbsp;&nbsp
     <table cellpadding=0 cellspacing=0 border=0 width=70% align=center>
     <tr>
     <form method=GET action=QA.php>
       <td width=100%>
         <b>CAD Object Adapter Version:</b>
         <input type="hidden" name="qa_func" value="<?php echo $qa_func;?>">
       </td>
      <td width=25%><?php print_formDropDown("cadoaver","select cadoaver from wrkconf_cadoaver"); ?></td>
      <td width=10% align=center>&nbsp;&nbsp;&nbsp;&nbsp<?php print_submit("small_submit.gif", "Display list of CADOA releases", "bottom"); ?></td>
     </form>
     </tr>
     </table>
     <p align=center><?php print_link("help.php", "| Help |");?></p>
     <p align=center><?php print_link("mailto:bmazeran@ptc.com?subject=ProductView QA Workers", "Contact for this page"); ?></p>
     </td>
    </tr>
    </table>
    </td>
   </tr>
   </table>
   </center>
   <?php
   return true;
  }
  
  function step2Page ($cadoaver)
  {
   global $qa_func;
   ?>
   <br><form method=GET action=QA.php>
   
   </form>
   <?php
  }
  
}

?>
