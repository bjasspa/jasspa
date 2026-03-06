% hello world program
-module(helloworld).
% import function used to import the io module
-import(io,[fwrite/1]).
% export function used to ensure the start function can be accessed.
-export([start/0]).

% some more comments which might be written with more effort and more and
% more information about percentage signs 
start() ->
   fwrite("Hello, world!\n").
