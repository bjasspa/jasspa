
 -- SETS.E

 --
 -- An include file to facilitate the declaration and manipulation
 -- of mathematical sets constructed from Euphoria sequences.
 --
 -- Copyright November 2001 by Rod Jackson. All rights reserved.
 --
 -- This file is made freely available for all public, private,
 -- and commercial uses. Please do not publicly distribute
 -- modifications without the author's permission.
 --
 -- For additional information, please contact the author at:
 -- rodjackson@bigfoot.com
 --
 --------------------------------------------------------------------
 --
 -- AVAILABLE CONSTANTS:
 --
 --    EMPTY_SET ==> an empty mathematical set; i.e., {}
 --
 -- AVAILABLE TYPES:
 --
 --    set ==> ensures that the variable is a sequence wherein
 --            each of its members only occurs once in the sequence
 --
 -- AVAILABLE FUNCTIONS:
 --
 --    add_member ==>    s = add_member (s, x)
 --       adds a new member to a set, if it's not already a member
 --
 --    remove_member ==> s = remove_member (s, x)
 --       removes the specified member from the set
 --
 --    union ==>         s = union (s, s)
 --       returns the union of two given sets
 --
 --    intersection ==>  s = intersection (s, s)
 --       returns the intersection of two given sets
 --
 --    diff ==>          s = diff (s, s)
 --       returns the difference of the two given sets; i.e.,
 --       the inverse of the union (with the intersection removed)
 --
 --    is_subset ==>     i = is_subset (s, s)
 --       returns T (1) if the first set is a subset of the second
 --
 --    is_superset ==>   i = is_superset (s, s)
 --       returns T (1) if the first set is a superset of the 2nd
 --
 -------------------------------------------------------------------

 ---------------------
 -- local constants --
 ---------------------

constant F = 0,
         T = 1

global constant EMPTY_SET = {}

 ---------------------
 -- local variables --
 ---------------------

integer p, L3, is_set
sequence s3, intersect, d
object o

 ------------------
 -- global types --
 ------------------

global type set (object s)

   if (atom (s)) then
      is_set = F
   else

      is_set = T
      for i = 2 to length (s) do
         p = find (s[i], s[1..i-1])
         if (p > 0) then
            is_set = F
            exit
         end if
      end for

   end if

   return is_set

end type -- set()

 ----------------------
 -- global functions --
 ----------------------

global function add_member (object x, set s)

   p = find (x, s)

   if (p < 1) then
      s = append (s, x)
   end if

   return s

end function -- add_member()

 ----------

global function remove_member (object x, set s)

   p = find (x, s)

   if (p > 0) then
      s = s[1..p-1] & s[p+1..length(s)]
   end if

   return s

end function -- remove_member()

 ----------

global function union (set s1, set s2)

   L3 = length (s1)
   s3 = s1 & s2

   for i = 1 to length (s2) do
      o = s2[i]
      p = find (o, s1)
      if (p < 1) then
         L3 = L3 + 1
         s3[L3] = o
      end if
   end for

   s3 = s3[1..L3]

   return s3

end function -- union()

 ----------

global function intersection (set s1, set s2)

   L3 = 0
   s3 = repeat (0, length (s1))

   for i = 1 to length (s2) do
      o = s2[i]
      p = find (o, s1)
      if (p > 0) then
         L3 = L3 + 1
         s3[L3] = o
      end if
   end for

   s3 = s3[1..L3]

   return s3

end function -- intersection()

 ----------

global function diff (set s1, set s2)

   s3 = EMPTY_SET

   for i = 1 to length (s1) do
      o = s1[i]
      p = find (o, s2)
      if (p < 1) then
         s3 = append (s3, o)
      end if
   end for

   for i = 1 to length (s2) do
      o = s2[i]
      p = find (o, s1)
      if (p < 1) then
         s3 = append (s3, o)
      end if
   end for

   return s3

end function -- diff()

 ----------

global function is_subset (set sub, set super)

   intersect = intersection (sub, super)
   d = diff (intersect, sub)

   if (length (d) > 0) then
      return F
   else
      return T
   end if

end function -- is_subset()

 ----------

global function is_superset (set super, set sub)

   return is_subset (sub, super)

end function -- is_superset()

