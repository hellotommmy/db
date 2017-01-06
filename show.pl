#!/usr/bin/perl -w

for (my $i=1; $i <= 10; $i++) {
  my $cmd = "./dbms  $i.sql -o ./out1/$i.out";
  system ("$cmd");
} # for i 


