// -!- go -!- 
// Copyright (c) 2025 MicroEmacs User.
//
// All rights reserved.
//
// Synopsis:    test file
// Authors:     MicroEmacs User


package main

import (
    "fmt"
    "os"
)
type Student struct {
    name string
    marikel int
}

func (s Student) Name() {
    fmt.Println(s.name)
}

func main () {
    argv := os.Args
    fmt.Printf("Usage: %s\n",argv[0])
    fmt.Println("Hello World!")
    df := Student{"Delf",1007}
    df.Name()
}   
