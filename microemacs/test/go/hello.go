// -!- go -!- //////////////////////////////////////////////////////////////
// program should be runnable for instance via
// go run hello.go
// it contains a few issues to check linting and execting
package main

import "fmt"

// Press Esc-q within the paragraph below to see if formatting works

/* hello this is a very long comment which wraps two lines but has too many characters,
 * much longer than it should be for a single line. It can be used to check the formatting of long comment lines.
 */

func main() {
	// missing a n at the end of Printl
	fmt.Printl("Hello world!")
}
