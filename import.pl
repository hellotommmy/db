#!/usr/bin/perl -w

@columns = (1,2,3,4,5);
@tables  = ("tab1", "tab2");

foreach (@columns) {
  my $col = $_;
  foreach (@tables) {
    my $tab = $_;
   # my $cmd ="psql -U dbms -f col$col$tab"; 
   my $cmd ="./dbms col$col$tab"; 
    print ("$cmd\n");
    system($cmd);
  } # foreach tables 
} # foreach columns 
