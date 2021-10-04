#!/usr/bin/expect -f

set timeout 20

if {[llength $argv] < 2} {
	puts "Usage $argv0 \[expect option ...\] password cmd \[args ...\]"
	exit 1
}

set password [lindex $argv 0]
set cmd [lrange $argv 1 end]

# Run the given command
eval spawn $cmd

expect_before {
	timeout { puts "Unable to access the host"; exit 1 }
}

# Disable output before interaction
log_user 0

expect {
	eof { puts "Connection rejected by the host"; exit 2 }
	"(yes/no)?" { send "yes\r" }
	"assword:" { send "$password\r"; interact; exit 0}
}

foreach {pid spawnid os_error_flag value} [wait] break

exit $value
