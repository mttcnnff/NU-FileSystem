use 5.16.0;
use warnings FATAL => 'all';

use Test::Simple tests => 2;
use IO::Handle;

sub write_text {
    my ($name, $data) = @_;
    open my $fh, ">", "mnt/$name" or return;
    $fh->say($data);
    close $fh;
}

system("mkdir mnt/numbers");
ok(-d "mnt/numbers", "made numbers!");
for my $ii (1..50) {
    write_text("numbers/$ii.num", "$ii");
}

my $nn = `ls mnt/numbers | wc -l`;
ok($nn == 50, "created 50 files");