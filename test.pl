#!/usr/bin/perl
use 5.16.0;
use warnings FATAL => 'all';

our $test_count = 29;

use Test::Simple tests => 29;
use IO::Handle;

my @bins = qw(test1 test2 test3);

system("(cd tests && make -f make) > /dev/null");

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

sub exec_tests {
    my ($cmd) = @_;
    unless (-e "tests/$cmd") {
        ok(0, "missing $cmd");
    }

    open my $hh, "-|", "tests/$cmd" or die;
    my $plan = <$hh>;
    $plan =~ /^1\.\.(\d+)$/ or die;
    my $nn = $1;
    my $oks = 0;
    while (<$hh>) {
        if (/^ok.*?-\s+(.*?)$/) {
            ++$oks;
            ok(1, "($cmd) $1");
        }
        if (/^not.*?-\s+(.*?)$/) {
            ok(0, "($cmd) $1");
        }
    }
    while ($oks++ < $nn) {
        ok(0, "($cmd) missed test");
    }
    close $hh;
}

system("rm -f data.nufs test.log");

say "#           == Basic Tests ==";

mount();

my $msg0 = "hello, one";
write_text("one.txt", $msg0);
ok(-e "mnt/one.txt", "File1 exists.");
ok(-f "mnt/one.txt", "File1 is regular file.");
my $msg1 = read_text("one.txt");
say "# '$msg0' eq '$msg1'?";
ok($msg0 eq $msg1, "Read back data1 correctly.");

my $long0 = "=This string is fourty characters long.=" x 50;
write_text("2k.txt", $long0);
my $long1 = read_text("2k.txt");
ok($long0 eq $long1, "Read back long correctly.");

my $long2 = read_text_slice("2k.txt", 10, 50);
my $right = "ng is four";
ok($long2 eq $right, "Read with offset & length");

my $files = `ls mnt`;
ok($files =~ /one\.txt/, "one.txt is in the directory");
ok($files =~ /2k\.txt/, "2k.txt is in the directory");

unmount();

if (-e "mnt/one.txt") {
    for (7..$test_count) {
        ok(0, "unmount failed");
    }
    exit(0);
}

mount();

$files = `ls mnt`;
ok($files =~ /one\.txt/, "one.txt is in the directory still");

$msg1 = read_text("one.txt");
say "# '$msg0' eq '$msg1'?";
ok($msg0 eq $msg1, "Read back data1 correctly again.");

system("mv mnt/one.txt mnt/abc.txt");
$files = `ls mnt`;
ok($files !~ /one\.txt/, "moved one.txt");
ok($files =~ /abc\.txt/, "have abc.txt");

my $msg4 = read_text("abc.txt");
say "# '$msg1' eq '$msg4'?";
ok($msg1 eq $msg4, "Read back data after rename.");

system("rm -f mnt/abc.txt");
$files = `ls mnt`;
ok($files !~ /abc\.txt/, "deleted abc.txt");

say "#           == Less Basic Tests ==";

my $huge0 = "=This string is fourty characters long.=" x 1000;
write_text("40k.txt", $huge0);
my $huge1 = read_text("40k.txt");
ok($huge0 eq $huge1, "Read back 40k correctly.");

my $huge2 = read_text_slice("40k.txt", 10, 8050);
$right = "ng is four";
ok($huge2 eq $right, "Read with offset & length");

unmount();

mount();

for my $bin (@bins) {
    exec_tests($bin);
}

unmount();
