#!/usr/local/bin/perl
# 
# yup -- print multiple PS/text images on one page.
# Copyright (C) 1995  Malcolm Herbert
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# 
# Author: Malcolm Herbert (drol@rmit.edu.au)
# 
# cmd-perl.pl -- Malcolm Herbert (drol@rmit.edu.au), (c) 1995
#
# $Id: yup.pl,v 1.1 2005-12-15 22:36:43 bill Exp $

require "ctime.pl";

@arglist = (
'V','allavailable',0,1,'$allavail_f','','use all available page area',
'A','alternate',0,0,'$altern_f','','alternate page margins',
'b','boxlinewidth',1,1,'$box_ln','','spot box line width',
'B','boxspots',0,1,'$box_f','','draw a box around each spot',
'C','columnmajor',0,0,'$colmaj_f','','do column major output',
'D','duplex',0,0,'$duplex_f','','print on both sides of page',
'J','ejectpage',0,0,'$eject_f',jf ,'eject page at end of file',
'E','epsfile',0,0,'$epsf_f',ef ,'treat file as an EPS file',
'F','filedate',0,0,'$modify_f','','use file date in spot titles',
'O','flipallow',0,1,'$flip_f','','flip page sideways if necessary',
'K','forcegstate',0,0,'$gforce_f',gf ,'force graphics state',
'G','gutters',0,1,'$gutter_f','','use gutters',
'g','guttersize',1,5,'$gutter_sz','','gutter size',
'H','hardbounds',0,1,'$hardbdy_f',hf ,'make sure print is in bounds',
'h','help',0,-1,'$help_f','','print help messages',
'L','landscape',0,0,'$lands_f','','make each spot landscape',
'','lprboldgrey',1,0,'$gy_blpr',gc ,'grey value for line printer bold text',
'd','lprcharhigh',1,0,'$char_hi',ch ,'characters high for line printer',
'e','lprcharwide',1,0,'$char_wd',cw ,'characters wide for line printer',
'Z','lprcrlf',0,1,'$crlf_f',kf ,'treat LF as CRLF in line printer',
'','lprfontbold',1,'/Courier-Bold','$btext_fn',bx ,'line printer bold font',
'W','lprlinewrap',0,1,'$wrap_f',wf ,'wrap lines in line printer',
'','lprmargin',1,6,'$text_mg',xm ,'margin for line printer print',
'','lprpagegrey',1,1,'$gy_page',gp ,'grey value for line printer page',
'','lprtabsize',1,8,'$tab_sz',bz ,'spaces per line printer tab',
'','lprtextfont',1,'/Courier','$text_fn',xn ,'font for line printer text',
'','lprtextgrey',1,0,'$gy_lpr',gl ,'grey value for line printer normal text',
'z','lprtextsize',1,0,'$text_sz',xz ,'font size for line printer',
'','marginbottom',1,5,'$phmarg_b','','bottom page margin',
'','marginleft',1,5,'$phmarg_l','','left page margin',
'','marginright',1,5,'$phmarg_r','','right page margin',
'','margintop',1,5,'$phmarg_t','','top page margin',
'm','mediatype',1,'/letter','$media','','paper type to print on',
'','numberboxsize',1,16,'$number_box','','size of page number box',
'','numberfont',1,'/Helvetica','$number_fn','','font for page numbers',
'N','numberpages',0,1,'$number_f','','number each page',
'','numbersize',1,12,'$number_sz','','size for page number font',
'S','numberspots',0,1,'$spotnum_f','','number each spot',
'o','outfile',1,'','$out_file','','filename to dump output into',
'p','pagecounter',1,1,'$page_ctr',pc ,'page counter',
'n','pages',1,2,'$spots_min','','spots per page',
'P','program',0,0,'$prog_f',pf ,'treat files as PostScript',
'q','quiet',0,-1,'$quiet_f','','suppress message printing',
'R','reverse',0,0,'$reverse_f','','reverse spot order on page',
's','spotcounter',1,0,'$spot_ctr',sc ,'spot counter',
'v','spotshigh',1,1,'$spots_hi','','number of spots high in user tile',
'w','spotswide',1,2,'$spots_wd','','number of spots wide in user tile',
'','spotposition',1,0,'$spot_pos',sp ,'spot position',
'Q','strict',0,0,'$strict_f','','allow only n spots per page',
'k','strictlevel',1,5,'$strict_lvl','','tiling strictness level',
'T','text',0,0,'$text_f',xf ,'treat files as ASCII text',
'','titlebold',1,'/Helvetica-Bold','$btitle_fn','','font for bold titles',
'a','titleboldgrey',1,0,'$gy_btitle','','grey value for bold titles',
'f','titleboxgrey',1,.8,'$gy_box','','grey value of title box',
'','titleboxsize',1,20,'$title_box','','size of spot title box',
'c','titlecentre',1,'','$title_c',tc ,'centre title string',
'','titlefont',1,'/Helvetica','$title_fn','','font for normal titles',
't','titlegrey',1,0,'$gy_title','','grey value of title text',
'l','titleleft',1,'','$title_l',tl ,'left title string',
'r','titleright',1,'','$title_r',tq ,'right title string',
'I','titles',0,1,'$title_f','','title each spot',
'','titlesize',1,14,'$title_sz','','size for title font',
'X','translate',0,0,'$xlate_f',yf ,'translate origin',
'x','translatex',1,0,'$xlate_x',xx ,'translate factor x coordinate',
'y','translatey',1,0,'$xlate_y',xy ,'translate factor y coordinate',
'Y','tumble',0,0,'$tumble_f','','print for binding on top edge',
'M','usemedia',0,0,'$media_f','','use specified paper for print',
'j','userheight',1,792,'$spot_uy','','user defined spot height',
'','usersize',0,0,'$upage_f','','use user spot dimensions',
'U','usertile',0,0,'$utile_f','','use user tile specifications',
'i','userwidth',1,612,'$spot_ux','','user defined spot width'
);

@true_false = ('false', 'true');
@will_wont = ('will not', 'will');
$USAGE = "usage: $0 [ global/local options ] [ file [ [ local options ] file ... ] ]";

for ($i = $[; $i < $#arglist; $i += 7) {
  eval "$arglist[$i + 4] = \'$arglist[$i + 3]\'";
}

if (defined($ENV{'YUPARGS'})) {
  @ARGV = (split(/[ \t\n]+/, $ENV{'YUPARGS'}), @ARGV);
}

$endopt_f = 0; $file_idx = 0; $file_defs[0] = ''; $Title = '';
$file_name[0] = '-'; $banner_f = 0; $| = 1; @argv = @ARGV;
$0 =~ s/^.*[\/\\]//;

loop: while (@ARGV) {

  if (!@ARGV) { print STDERR "$0: ran out of arguments\n"; }
  $opt = ''; $flag = ''; $opt_num = -1; $arg = ''; $val = ''; $idx = 0;

  if ($ARGV[0] eq '--' && !$endopt_f) { 
    $endopt_f = 1; shift(@ARGV); next loop;
  }

  ($flag, $opt, $arg) = ($ARGV[0] =~ /(.)(.)(.*)/);

  if (($flag =~ /[+-]/) && $opt && !$endopt_f) {

    if ($flag eq $opt) { $flag .= $opt; $opt = $arg; $arg = ''; $idx = 1; }

    for ($i = $idx; $i < $#arglist; $i += 7) {
     if ($arglist[$i] eq $opt) { $opt_num = ($i - $idx) / 7; }
    }

    if ($opt_num == -1) {
      print STDERR "$0: unknown option $flag$opt\n$USAGE\n"; exit 1;
    }

    if ($arglist[$opt_num * 7 + 2]) {

      if (!$arg) {
	shift(@ARGV); $val = $ARGV[0];

	if (!defined($val)) {
	  print STDERR "$0: option $flag$opt requires an argument\n"; exit 2;
	}
      } else {
	$val = $arg;
      }
      shift(@ARGV);
    } else {

      if (!$arg) { shift(@ARGV); } else { $ARGV[0] = "$flag$arg"; }

      if ($flag =~ /[+]/) { $val = 1; } else { $val = 0; }
    }

    if ($file_idx == 0) {
      eval "$arglist[$opt_num * 7 + 4] = \'$val\'";
    } else {
      if ($arglist[$opt_num * 7 + 5] ne '') {
        $file_defs[$file_idx] .= "/$arglist[$opt_num * 7 + 5] ";
        if ($arglist[$opt_num * 7 + 2] == 1) {
          if ($arglist[$opt_num * 7 + 4] =~ /title_[crl]/) {
            $file_defs[$file_idx] .= "($val)";
          } else {
    	    $file_defs[$file_idx] .= "$val";
  	  }
        } else {
  	$file_defs[$file_idx] .= "$true_false[$val]";
        }
      } else {
        print STDERR "$0: warning:  global option $flag$opt ignored.\n";
      }
    }

    if ($help_f == 1) { &flying_help($opt_num, $val); }
  } else {

    if ((!-e $ARGV[0] || -d $ARGV[0]) && $ARGV[0] ne '-') {
     print STDERR "$0: warning: \`$ARGV[0]\' skipped - doesn\'t exist or is dir\n"; 
      shift(@ARGV); next loop;
    }

    $opt = ''; $file_name[$file_idx] = shift(@ARGV);
    if ($help_f == 1) { print "input file: $file_name[$file_idx]\n"; }
    $Title .= "$file_name[$file_idx] "; $file_name[++$file_idx] =''; 
    $file_defs[$file_idx] = '';
  }
}

if ($help_f != -1) { if ($help_f == 0) { &print_help; } exit 0; }

if (-t STDOUT) {

  if ($out_file eq '') { $out_file = 'yup.ps'; }

  for ($i = 0; $i < $file_idx; $i++) {
    if ($out_file eq $file_name[$i]) {
      print STDERR "$0: output file ($out_file) found in input file list\n";
      exit 3;
    }
  }

  open (OUTFILE, ">$out_file"); select (OUTFILE); $| = 1;
} else {
  $quiet_f = 0;
}

if ($quiet_f == -1) { 
  &print_banner(STDOUT); print STDOUT "yup --> $out_file\n\n";
}

for ($i = 4; $i < $#arglist; $i += 7) {
  if (($arglist[$i - 2] == 0) && ($arglist[$i - 1] >= 0)) {
    eval "$arglist[$i] = \$true_false[$arglist[$i]]";
  }
}

$Title =~ s/ - / stdin /g; if ($Title eq '') { $Title = 'stdin'; }
$DocFonts = "Helvetica Helvetica-Bold Courier Courier-Bold"; $CommandLn = '';
$Date = &ctime(time); chop($Date); 

if ($title_l eq '' && $modify_f eq false) { 
  $title_l = $Date; 
}

$User = getpwuid($<);

for ($i = 0; $i <= $#argv; $i++) {
  if ($argv[$i] !~ /[ 	]/) { $CommandLn .= "$argv[$i] "; }
  else { $CommandLn .= "\'$argv[$i]\' "; }
}

print <<YUP_EOF;
%!PS-Adobe-1.0
%%Creator: yup, Malcolm Herbert (drol\@rmit.edu.au), (c) 1995
%%Revision: \$Id: yup.pl,v 1.1 2005-12-15 22:36:43 bill Exp ${dummy}\$
%%DocumentFonts: $DocFonts
%%Title: $Title
%%CreationDate: $Date
%%YupCommandLine: $CommandLn
%%For: $User
%%
%%  yup comes with ABSOLUTELY NO WARRANTY.  This is free software, 
%%  and you are welcome to redistribute it under certain conditions; 
%%  see the file COPYING for details on warranty and these conditions.
%%
%%EndComments
%%YupArgsBegin
$gy_page $gy_blpr $gy_lpr $gy_btitle $gy_title $gy_box $xlate_x $xlate_y
$xlate_f $tab_sz $wrap_f $crlf_f $btext_fn $text_fn $text_mg $text_sz $char_wd
$char_hi $eject_f $epsf_f $gforce_f $text_f $hardbdy_f $page_ctr($title_c)
($title_l)($title_r)$altern_f $prog_f $box_f $btitle_fn $title_sz $title_fn
$number_sz $number_fn $colmaj_f $spotnum_f $spot_ctr $spot_pos $allavail_f
$spots_min $strict_lvl $strict_f $spots_wd $spots_hi $utile_f $flip_f
$gutter_sz $gutter_f $title_box $title_f $spot_uy $spot_ux $upage_f $phmarg_b
$phmarg_r $phmarg_t $phmarg_l $number_box $number_f $reverse_f $box_ln $lands_f
$media $media_f $duplex_f $tumble_f
%%YupArgsEnd
/Yup where{quit}if/IV save def/Yup 150 dict def Yup begin[/!/not/&/and/+/add/-
/sub/B/clippath/C/ceiling/D/closepath/E/exit/F/for/G/forall/H/setgray/I/index
/J/counttomark/K/lineto/L/loop/M/mod/N/neg/O/matrix/P/putinterval/Q/floor/R
/rotate/S/setmatrix/T/stopped/U/statusdict/V/type/W/load/Y/known/^/pop/a
/currentmatrix/b/pathbbox/c/copy/d/def/e/div/f/false/i/if/j/ifelse/k/findfont
/l/translate/m/mul/n/stringwidth/o/newpath/p/setlinewidth/r/roll/t/true/u
/scalefont/v/moveto/w/rlineto/x/exch/y/string/z/setfont/~/dup{{load def}loop}
stopped pop pop[/X{x d}/A{{x d}G}/PE{pc 2 M 0 eq}/MR{af PE lf ! & &{pl}{pr}j}
/ML{af PE lf ! & &{pr}{pl}j}/MT{af PE lf & &{pb}{pt}j}/MB{af PE lf & &{pt}{pb}
j}/NP{pc 1 + log C cvi y pc x cvs &I nn z ~ n ^ af lf ! PE & &{^ ML}{ux MR - x
-}j af lf PE ! & &{MB nb +}{uy MT - nz - nb +}j v show &H}/OX{ML}/OY{MB af nf
PE ! lf & & &{nz +}i}/SO{sp ~ sm M x sm idiv cf !{x}i sx gz + m OX + x sh x - 1
- sy gz + tz + m OY +}/SC{O a aload ^[/s5/s4/s3/s2/s1/s0]A}/RC{[s0 s1 s2 s3 s4
s5]}/SI{&G yc S SO l &C ss ss scale ox N oy N yf{xy + x xx + x}i l SC IG}/SB{
SO l o 0 0 v 0 sy w sx 0 w 0 sy N w D}/SX{&I SB bl p stroke &H}/ST{&I SO sy + l
o 0 0 v 0 tz w sx 0 w 0 tz N w D &I gb H fill &H bl p stroke gi H tn z 0 tb l
tb 0 v tl show tq ~()eq sf &{^ sc 2 + log C cvi ~ 5 + y ~ 0(Page )P ~ 5 sc 1 +
5 4 r y cvs P}i ~ sx x n ^ - tb - 0 v show tc ~()eq{^ fn}i gd H bt z ~ sx x n ^
- 2 e 0 v show &H}/EP{/df t d &I &C yc S SO l 0 0 v 0 sy w sx 0 w 0 sy N w D 1
H fill &H}/IG{IM o IC 1 p 0 setlinecap 0 setlinejoin[]0 setdash 0 H 10
setmiterlimit}/IM{RC S}/DM{^ RC}/IC{&C O a yc S hf{SB clip o}i S}/CP{{}^}/JP{
&I &G yc S nf{NP}i &S/pc pc 1 + d/sp sb d &H/df f d}/SP{&G yc S bf{SX}i tf{ST}
i/sc sc 1 + d/sp sp si + d sp se eq{JP}i SI}/FS{O a O a RC O invertmatrix O
concatmatrix S &I S}/save_path{[{/v W J 3 r}{/K W J 3 r}{/curveto W J 7 r}{/D W
J 1 r}{pathforall}T{0 0/v W}i}/rstr_path{{~ V/marktype eq{^ E}i exec}L}/FR{
save_path B save_path &H O a RC O concatmatrix S o &C rstr_path clip rstr_path
}/SG{gf{FS}i &V}/RG{[[/df/s0/s1/s2/s3/s4/s5/pc/sc/sp]{~ W}G J 2 + -1 r &R{{~ V
/marktype ne{d}{E}j}L}T ^ ^ gf{FR}i}/GS{gf{FS}i &I}/GR{&H gf{FR}i}/GA{&J SI}
/GC{currentfile read !{quit}i}/ps_text{Yup/YV Y{ef{SP}i jf{JP}i YV RG}i/sc 0
{{end}L}T ^ Yup begin{{~ V/marktype ne{d}{E}j}L}T clear/YV SG d SI 0{~ GC ys 3
2 r get eq{1 +}{^ 0}j ~ ys length eq{E}i}L ^[{GC ~ 32 eq{^}{E}j}L{GC ~ 10 eq{^
E}i}L]~ length y 0 3 2 r{3 c put ^ 1 +}G ^/fn X[pf xf or !{GC GC 2 c ~ 33 eq x
37 eq or x 37 eq &{clear{GC 10 eq{E}i}L/pf t d}i}i pf !{LI LP{GC 10 eq{E}i}L}i
}/find_scale{2 c 1 - gz m nz + zy x - x 1 - gz m zx x - sx 4 I m e x sy tz + 3
I m e 2 c ge{x}i ^ ~ ss gt{4 c f[/qf/ss/sh/sw/sg]A}i ^ ff{2 c 1 - gz m nz + zx
x - x 1 - gz m zy x - sx 4 I m e x sy tz + 3 I m e 2 c ge{x}i ^ ~ ss gt{4 c t[
/qf/ss/sh/sw/sg]A}i ^}i}/LI{J 0 eq{GC GC}i 0 0[/cy/cx/c2/c1]A ^ gf/gf t d &I B
gp H fill &H 0 ch 0 gt{/ch ch Q d 4 +}i cw 0 gt{/cw cw Q d 2 +}i xz 0 gt{1 +}i
~ 0 eq{/ch 66 d ^ 4}i ~ 1 eq{^ 3}i ~ 2 eq{xn k z/xz sx ss e xm 2 m - cw e(M)n ^
e d ^ 3}i ~ 3 eq{/ch sy ss e xm 2 m - xz e Q d cw 0 eq{^ 5}i}i ~ 4 eq{/xz sy ss
e xm 2 m - ch e d ^ 5}i ~ 5 eq{xn k xz u z/cw sx ss e xm 2 m -(M)n ^ e Q d}i ~
6 eq{sy ss e xm 2 m - ch e xn k z sx ss e xm 2 m - cw e(M)n ^ e 2 c gt{x}i ^
/xz X/ch sy ss e xm 2 m - xz e Q d xn k xz u z/cw sx ss e xm 2 m -(M)n ^ e Q d
}i 7 eq{}i/xn xn k xz u d/bx bx k xz u d xn z gl H/em(M)n ^ d ox xm + oy xm + o
0 0 v(gjpqy)f charpath b o 4 3 r ^ ^ ^ - l}/OP{SP &I B gp H fill &H c0 c1 c2 LV
RG[/c2/c1/c0]A/LV SG d}/LP{/LV SG d{/c0 c1 d/c1 c2 d/c2 GC d c0 32 lt{c0 10 eq
{c1 c2 eq c1 37 eq &{E}{/cy cy 1 + d kf{/cx 0 d}i}j}i c0 13 eq{c1 c2 eq c1 37
eq &{E}{/cx 0 d}j}i c0 8 eq{/cx cx 1 - ~ 0 lt{^ 0}i d}i c0 9 eq{/cx cx bz e Q 1
+ bz m d}i c0 12 eq{OP}i}{c0 c2 eq c1 8 eq &{&I bx z gc H cx em m ch cy - 1 -
xz m v( )~ 0 c0 put show/df t d/cx cx 1 + d &H{/c1 GC d/c2 GC d c0 c2 eq c1 8
eq & !{E}i}L}{cx em m ch cy - 1 - xz m v( )~ 0 c0 put show/df t d/cx cx 1 + d}
j}j wf cx cw ge &{/cx 0 d/cy cy 1 + d}i cy ch ge{OP}i}L LV RG df{SP}i/gf X}{{
bind d}L}T ^ ^ U/settumble Y{U begin settumble end}{^}j U/setduplexmode Y{U
begin setduplexmode end}{^}j{~ where{^ cvx exec}{^}j}{^}j userdict begin[
/letter/legal/note/a3/a4/a5/a6/b4/flsa/flse/halfletter/11x17/ledger]{{}d}G[/EP
/&E/erasepage/IG/&G/initgraphics/IM/&M/initmatrix/DM/&D/defaultmatrix/IC/&C
/initclip/GR/&H/grestore/GA/&J/grestoreall/GS/&I/gsave/CP/&P/copypage/SP/&S
/showpage/RG/&R/restore/SG/&V/save/ps_text/ps_text null{~ V/marktype eq{^ E}i ~
null ne{~ 3 1 r W d}{^}j x[/Yup/begin 4 3 r/end 4{cvx 4 1 r}repeat]cvx d}L[/&F
/fill/&O/eofill/&U/stroke/&N/image/&L/imagemask/&T/show/&B/ashow/&W/widthshow
/&A/awidthshow/&K/kshow{~ V/marktype eq{^ E}i 2 c W d[/Yup/begin/t/d/end 8 7 r
6{cvx 6 1 r}repeat/df 5 1 r]cvx d}L/showpage{Yup begin ef !{SP}i end}d end &G B
b ^ ^ 2 c[/oy/ox]A l B b 4 2 r ^ ^ 3 2 r{x ~ 0 l 90 R}i 2 c gt[/lf/py/px]A ~
/bl X 2 e ~ py x -/uy X ~ px x -/ux X ~[/lx/ly]A 2 c 5 2 r ! & !{^ 0}i[/nz/rf
/nf/pl/pt/pr/pb]A/zx ux pl - pr - d/zy uy pt - pb - d !{^ ^ py px}i[/sx/sy]A ~
/tf X !{^ 0}i/tz X !{^ 0}i[/gz/ff]A/ss 0 d{2 c m 3 1 r find_scale 6{^}repeat}{
^ ^{^ 0}i 1 I + 1 x{1 1 2 I{2 c M 0 eq{2 c idiv find_scale ^}i ^}F ^}F}j qf ff
&{pb pr pt pl lf{0 py l -90 R 4 1 r f}{px 0 l 90 R 4 3 r t}j lx ly ux uy zx zy
px py[/px/py/zx/zy/ux/uy/lx/ly/lf/pl/pt/pr/pb]A}i/yc O a d/sx sx ss m d/sy sy
ss m d zx sx sw m - gz sw 1 - m - zy sy tz ss m + sh m - gz sh 1 - m nz + - 3 2
r{/sy x sh e sy + d/sx x sw e sx + d}{2 e ~/pt x pt + d/pb x pb + d 2 e ~/pr x
pr + d/pl x pl + d}j sg M ~ 0 lt{sg +}i[/sp/sc]A rf{^ f sg 1 - -1 -1}{0 1 sg}j
[/se/si/sb/sf]A ~/cf X{sh}{sw}j/sm X nf{/nn x k 3 2 r u d &I nn z/nb nz o 0 0 v
(M)f charpath b 4 1 r ^ ^ ^ - 2 e d &H}{^ ^}j tf{/tn x k 3 2 r ss m ~ 5 1 r u d
/bt x k 3 2 r u d/tz tz ss m d &I tn z/tb tz o 0 0 v(M)f charpath b 4 1 r ^ ^ ^
- 2 e d &H ^ t}{^ ^ ^}j x Yup/LI Y !{^ t}i f(YupFile:)[/ys/df/pf/bf/af/tq/tl
/tc/pc/hf/xf/gf/ef/jf/ch/cw/xz/xm/xn/bx/kf/wf/bz/yf/xy/xx/gb/gi/gd/gl/gc/gp]A
SI end
%%YupBegin

YUP_EOF

if ($file_idx == 0) { $file_idx = 1; }
for ($i = 0; $i < $file_idx; $i++) {
  print "mark ";

  if ($file_defs[$i] =~ /\/tq *\(.*\)/) { $modify_f = false; }

  if ($modify_f eq true && $title_l eq '') {
    @mtime = stat($file_name[$i]);
    $mtime = &ctime($mtime[9]); chop($mtime);
    print "/" . tl  . " ($mtime) ";
  }

  print "$file_defs[$i] ps_text\n%%YupFile: ";

  if ($file_name[$i] eq '-') { print "stdin\n"; } 
  else { print "$file_name[$i]\n"; }

  open (TMP, "$file_name[$i]");

  if ($quiet_f == -1) { 
    if ($file_name[$i] eq '-') { print STDOUT "(stdin"; }
    else { print STDOUT "($file_name[$i]"; }
  }

  while (<TMP>) { $_ =~ s/^%%/% %/; print $_; }

  if ($_ !~ "\n") { print "\n"; }

  print "%%YupEOF\n";

  if ($quiet_f == -1) { print STDOUT ") "; }
}

print <<YUP_EOF;
%%YupEnd
%%Trailer
%%Revision: \$Id: yup.pl,v 1.1 2005-12-15 22:36:43 bill Exp ${dummy}\$
%%Creator: yup, Malcolm Herbert (drol\@rmit.edu.au), (c) 1995
%%
%%  yup comes with ABSOLUTELY NO WARRANTY.  This is free software, 
%%  and you are welcome to redistribute it under certain conditions; 
%%  see the file COPYING for details on warranty and these conditions.
%%
Yup begin ef{SP}{EP/df f d}j &G yc S df sp sb ne or{JP}i end IV &R
%%EOF

YUP_EOF

if ($quiet_f == -1) { print STDOUT " Done.\n"; }

sub print_help_item {
  local($i) = @_;
  local($opt, $longopt, $default) = ('', '', '');

  $opt = $arglist[$i];
  $longopt = $arglist[$i + 1];

  if ($arglist[$i + 2]) {
    $default = $arglist[$i + 3];
    $longopt = "--$longopt arg";
    if ($opt ne '') { $opt = "-$opt arg"; }
  } else {
    $default = $true_false[$arglist[$i + 3]];
    $longopt = "(++|--)$longopt";
    if ($opt ne '') { $opt = "(+|-)$opt"; }
  }

  print " $opt\t  $longopt\t";
  if (length($longopt) < 14) { print "\t"; }
  print "$arglist[$i + 6] ($default)\n";
}

sub print_banner {
  local(*FILEHANDLE) = @_;

  if ($banner_f == 0) {
    $banner_f = 1;

print FILEHANDLE <<BANNER_EOF;
$0, by Malcolm Herbert (drol\@rmit.edu.au), (c) 1995 \$Revision: 1.1 ${dummy}\$
$0 comes with ABSOLUTELY NO WARRANTY; see the file COPYING for details.
This is free software, and you are welcome to redistribute it under 
certain conditions; again, see the file COPYING for details.

BANNER_EOF

  }
}

sub print_help {
  &print_banner(STDOUT);

print <<HELP_EOF;
$USAGE

Boolean options use + (or ++) to set them true.  - (or --) sets them false.
+h gives verbose output of options as they are processed (should be first
option)  -h gives this help.  +q is equivalent to -q.

Global options (and defaults):

HELP_EOF

  for ($i = 0; $i < $#arglist; $i += 7) {
    if ($arglist[$i + 5] eq '') { &print_help_item($i); }
  }

  print "\nLocal options (and defaults):\n\n";

  for ($i = 0; $i < $#arglist; $i += 7) {
    if ($arglist[$i + 5] ne '') { &print_help_item($i); }
  }
}

sub flying_help {
  local($opt_num, $val) = @_;

  &print_banner(STDOUT);

  if (($arglist[$opt_num * 7 + 5] eq '') && ($file_idx > 0)) { return; }

  if ($arglist[$opt_num * 7 + 2] == 0) {
    print "$will_wont[$val] $arglist[$opt_num*7+6]\n";
  } else {
    print "$arglist[$opt_num*7+6] is now set to \'$val\'\n";
  }
}

