#! /usr/bin/perl

use strict;
$| = 1;
my $cnt = 0;
sub make($$$) {
	mkdir $_[0] or die $!;
	chdir $_[0] or die $!;

	for(my $i=0; $i<$_[2]; $i++) {
		open my $f, '>', "file_$i" or die $!;
		if(++$cnt >= 10) { $cnt=0; print "."; }
		for(my $j=0; $j<$i*10000; ++$j) {
			print $f "kjdfjkfdjkfd";
		}
		close $f;

		make("dir_$i", $_[1]-1, $_[2]) if $_[1] > 0;
	}

	chdir ".." or die $!;
}

make("testdir", 3, 5);

print "\n";
