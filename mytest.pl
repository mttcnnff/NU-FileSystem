#!/usr/bin/perl
use 5.16.0;
use warnings FATAL => 'all';

use Test::Simple tests => 28;
use IO::Handle;

sub mount {
    system("(make mount 2>&1) >> test.log &");
    sleep 1;
}

sub unmount {
    system("(make unmount 2>&1) >> test.log");
}

sub write_text {
    my ($name, $data) = @_;
    open my $fh, ">", "mnt/$name" or return;
    $fh->say($data);
    close $fh;
}

sub read_text {
    my ($name) = @_;
    open my $fh, "<", "mnt/$name" or return "";
    local $/ = undef;
    my $data = <$fh> || "";
    close $fh;
    $data =~ s/\s*$//;
    return $data;
}

sub read_text_slice {
    my ($name, $count, $offset) = @_;
    open my $fh, "<", "mnt/$name" or return "";
    my $data;
    seek $fh, $offset, 0;
    read $fh, $data, $count;
    close $fh;
    return $data;
}

system("rm -f data.nufs test.log");

say "#           == Basic Tests ==";
mount();

my $msg2 = "hello, two";
my $right = "ng is four";
system("mkdir mnt/foo");
ok(-d "mnt/foo", "Made a directory");

system("cp mnt/def.txt mnt/foo/abc.txt");
my $msg7 = read_text("foo/abc.txt");
say "# '$msg2' eq '$msg7'?";
ok($msg2 eq $msg7, "Read back data from copy in subdir.");

my $huge0 = "=This string is fourty characters long.=" x 1000;
write_text("40k.txt", $huge0);
my $huge1 = read_text("40k.txt");
ok($huge0 eq $huge1, "Read back 40k correctly.");

my $huge2 = read_text_slice("40k.txt", 10, 8050);
$right = "ng is four";
ok($huge2 eq $right, "Read with offset & length");

system("mkdir -p mnt/dir1/dir2/dir3/dir4/dir5");
my $hi0 = "hello there";
write_text("dir1/dir2/dir3/dir4/dir5/hello.txt", $hi0);
my $hi1 = read_text("dir1/dir2/dir3/dir4/dir5/hello.txt");
ok($hi0 eq $hi1, "nested directories");

system("mkdir mnt/numbers");
for my $ii (1..50) {
    write_text("numbers/$ii.num", "$ii");
}

my $nn = `ls mnt/numbers | wc -l`;
ok($nn == 50, "created 50 files");

for my $ii (1..5) {
    my $xx = $ii * 10;
    my $yy = read_text("numbers/$xx.num");
    ok($xx == $yy, "check value $xx");
}

for my $ii (1..4) {
    my $xx = $ii * 7;
    system("rm mnt/numbers/$xx.num");
}

unmount();

ok(!-d "mnt/numbers", "numbers dir doesn't exist after umount");

mount();

my $mm = `ls mnt/numbers | wc -l`;
ok($mm == 46, "deleted 4 files");

unmount();
