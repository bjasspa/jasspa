"""<tt000.py

 Subject: "Thinking in Tkinter"
 Author : Stephen Ferg (steve@ferg.org)

ABOUT " THINKING IN TKINTER "

I've been trying to teach myself Tkinter out of various books, and I'm finding
it more difficult than I think it should be.

The problem is that the authors of the books want to rush into telling me about
all of the widgets in the Tkinter toolbox, but never really pause to explain
basic concepts.  They don't explain how to "think in Tkinter".

Here are a few short programs that begin to explain how to think in Tkinter. In
them, I don't attempt to catalog all of the types of widgets, attributes, and
methods that are available in Tkinter.  and I certainly don't try to provide a
comprehensive introduction to Tkinter.  I just try to get you started down the
road of understanding some basic Tkinter concepts.

Note that the discussion is devoted exclusively to the Tkinter pack (or
"packer") geometry manager.  There is no discussion of the grid or place
geometry managers.

THE FOUR BASIC GUI-PROGRAMMING TASKS

When you develop a user interface (UI) there is a standard set of tasks that you
must accomplish.

1) You must specify how you want the UI to *look*.  That is, you must write code
that determines what the user will see on the computer screen.

2) You must decide what you want the UI to *do*.  That is, you must write
routines that accomplish the tasks of the program.

3) You must associate the "looking" with the "doing".  That is, you must write
code that associates the things that the user sees on the screen with the
routines that you have written to perform the program's tasks.

4) finally, you must write code that sits and waits for input from the user.


SOME GUI-PROGRAMMING JARGON

GUI (graphical user interface) programming has some special jargon associated
with these basic tasks.

1) We specify how we want a GUI to look by describing the "widgets" that we want
it to display, and their spatial relationships (i.e. whether one widget is above
or below, or to the right or left, of other widgets).  The word "widget" is a
nonsense word that has become the common term for "graphical user interface
component".  Widgets include things such as windows, buttons, menus and menu
items, icons, drop-down lists, scroll bars, and so on.

2) The routines that actually do the work of the GUI are called "callback
handlers" or "event handlers".  "Events" are input events such as mouse clicks
or presses of a key on the keyboard.  These routines are called "handlers"
because they "handle" (that is, respond to) such events.

3) Associating an event handler with a widget is called "binding".  Roughly, the
process of binding involves associating three different things: 

	(a) a type of event (e.g. a click of the left mouse button, 
	    or a press of the ENTER key on the keyboard), 
	(b) a widget (e.g. a button), and 
	(c) an event-handler routine.  

For example, we might bind (a) a single-click of the left mouse button on (b) 
the "CLOSE" button/widget on the screen to (c) the "closeProgram" routine, which 
closes the window and shuts down the program.

4) The code that sits and waits for input is called the "event loop".

ABOUT THE EVENT LOOP

If you believe the movies, every small town has a little old lady who spends all 
of her time at her front window, just WATCHING.  She sees everything that goes 
on in the neighborhood.  A lot of what she sees is uninteresting of course -- 
just people going to and fro in the street.  But some of it is interesting -- 
like a big fight between the newly-wed couple in the house across the street. 
When interesting events happen, the watchdog lady immediately is on the phone with
the news to the police or to her neighbors.

The event loop is a lot like this watchdog lady.  The event loop spends all of 
its time watching events go by, and it sees all of them.  Most of the events are 
uninteresting, and when it sees them, it does nothing.  But when it sees 
something interesting -- an event that it knows is interesting, because an event 
handler has been bound to the event -- then it immediately calls up the event 
handler and lets it know that the event has happened.


PROGRAM BEHAVIOR

This program eases you into user-interace programming by showing how these basic
concepts are implemented in a very simple program.  This program doesn't use
Tkinter or any form of GUI programming.  It just puts up a menu on the console,
and gets simple keyboard input.  Even so, as you can see, it does the four basic
tasks of user-interface programming.

[revised: 2003-02-23]
>"""

#----- task 2:  define the event handler routines ---------------------
def handle_A():
	print "Wrong! Try again!"

def handle_B():
	print "Absolutely right!  Trillium is a kind of flower!"
	 
def handle_C():
	print "Wrong! Try again!"

# ------------ task 1: define the appearance of the screen ------------
print "\n"*100   # clear the screen
print "            VERY CHALLENGING GUESSING GAME"
print "========================================================"
print "Press the letter of your answer, then the ENTER key."
print
print "    A.  Animal"
print "    B.  Vegetable"
print "    C.  Mineral"
print
print "    X.  Exit from this program"
print
print "========================================================"
print "What kind of thing is 'Trillium'?"
print

# ---- task 4: the event loop.  We loop forever, observing events. ---
while 1:

	# We observe the next event
	answer = raw_input().upper()

	# -------------------------------------------------------
	# Task 3: Associate interesting keyboard events with their
	# event handlers.  A simple form of binding.
	# -------------------------------------------------------
	if answer == "A": handle_A()
	if answer == "B": handle_B()
	if answer == "C": handle_C()
	if answer == "X": 
		# clear the screen and exit the event loop
		print "\n"*100
		break

	# Note that any other events are uninteresting, and are ignored