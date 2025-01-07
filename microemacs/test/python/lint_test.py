def my_function():
    """ An example for a Pylint demonstration."""
    my_sum1 = 3 + 4;
    print("This code contains Umlauts äöüÄÖÜ and the sz: ß!")
    print("The sum is %d." % my_sum1)
    raise Exception("This shouldn't happen.")
    return True

my_function()
