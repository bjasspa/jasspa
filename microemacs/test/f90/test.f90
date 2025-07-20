!-*- f90 -*- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!
! Copyright (C) 2025 MicroEmacs User.
!
! All rights reserved.
!
! Synopsis:
! Authors:     MicroEmacs User
!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
module test
contains
    elemental real function square(x)
        real, intent(in) :: x
        square = x*x
    end function
    integer function test1()
        print *, "Testing 1"
        test1 = 1
    end function
end module

program hello
    use test, only : test1
    integer :: y
    real :: x
    x = 3.0
    print *, "Hello World!"
    print *, "Hi"
    y = test1()
    !print *, "The squared value of", x, "is" , square(x), "!"
end program

