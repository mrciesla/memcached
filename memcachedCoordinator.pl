#!/usr/bin/perl

use strict;
use IO::Socket::INET;
sub createSocket{
    # flush after every write
    $| = 1;
    
    my ($socket,$client_socket);
    my ($peeraddress,$peerport);
    
    # creating object interface of IO::Socket::INET modules which internally does 
    # socket creation, binding and listening at the specified port address.
    $socket = new IO::Socket::INET (
        LocalHost => '141.212.106.235',
        LocalPort => '5000',
        Proto => 'tcp',
        Listen => 5,
        Reuse => 1
    ) or die "ERROR in Socket Creation : $!\n";
    return $socket;
 
}

sub pinMemcached{
    my $cmd = "ps -ef |grep \"memc[a]ched \" | awk '{system(\"sudo taskset -cap 0 \"\$2);}'";
    print $cmd;
    `$cmd`;
    if($? !=0){
        return "failure";
    }
    return "success";
}

sub resetMemcached{
    #Kill memcached and the profiler 
    print "Reseting memcached\n";
    print `sudo killall memcached`;
    print `sudo killall memcachedProfiler`;
    #Start memcached
    print system('/home/mrciesla/git/memcached/memcached -u mrciesla -l 141.212.106.235 -m 1024 -d');
    if ($? == -1) {
         print "failed to execute: $!\n";
         return "failure";
    }
    sleep 1;
    return "success";
    
}

sub isRunning(){
    my $run = `ps -ef |grep \"memc[a]ched \"`;
    if($run){
        return "success";
    }
    return "failure";
}

sub processCommand{
    my $command = shift;
    chomp($command);
    print "Command is #$command#\n";
    if($command eq "reset"){
        return resetMemcached();
    }elsif($command eq "pinMemcached"){
        return pinMemcached();
    }elsif($command eq "running"){
        return isRunning();
    }elsif($command eq "profile"){
        return "failuire";
    }elsif($command eq "profileHash"){
        return "failuire";
    }elsif($command eq "pinProfile"){
        return "failuire";
    }elsif($command eq "start"){
        return `cat /proc/stat | grep cpu  | awk '{print \$1,\$2,\$3,\$4,\$5}'`;
    }elsif($command eq "end"){
        return `cat /proc/stat | grep cpu  | awk '{print $1,$2,$3,$4,$5}'`;
    }else{
        return "failuire";
    }
}
sub coordinate{
    my $client_socket;
    my ($peeraddress,$peerport);
    
    my $keepListening = 1;
    do{
        my $socket = createSocket(); 
        # waiting for new client connection.
        my $client_socket = $socket->accept();
        
        # get the host and port number of newly connected client.
        my $peer_address = $client_socket->peerhost();
        my $peer_port = $client_socket->peerport();
        
        print "Accepted New Client Connection From : $peeraddress, $peerport\n";
        
        # write operation on the newly accepted client.
        my $data;
        
        # read operation on the newly accepted client
        $data = <$client_socket>;
        my $success = processCommand($data);
        $client_socket->send($success);
        $socket->close();
    }while($keepListening);
}

coordinate();
