#[
 # ###########################################################
 #  Copyright (c) 2025, University of Potsdam, Germany
 #  License       :                     
 #  Date          : $Date$              
 #  Author        : $Author$            
 #  Created By    : Detlef Groth         
 #  Created       : Thu Oct 9 22:33:40 2025        
 #  Last Modified : <251009.2249>       
 #                                      
 #  Description   :                     
 # ###########################################################                                     
]# 

echo "Gi"
type
  BinaryTree*[T] = ref object # BinaryTree is a generic type with
                              # generic param `T`
    le, ri: BinaryTree[T]     # left and right subtrees; may be nil
    data: T                   # the data stored in a node

proc newNode*[T](data: T): BinaryTree[T] =
  # constructor for a node
  new(result)
  result.data = data

proc add*[T](root: var BinaryTree[T], n: BinaryTree[T]) =
  # insert a node into the tree
  if root == nil:
    root = n
  else:
    var it = root
    while it != nil:
      # compare the data items; uses the generic `cmp` proc
      # that works for any type that has a `==` and `<` operator
      var c = cmp(it.data, n.data)
      if c < 0:
        if it.le == nil:
          it.le = n
          return
        it = it.le
      else:
        if it.ri == nil:
          it.ri = n
          return
        it = it.ri
