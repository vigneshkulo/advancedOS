remoteuser=vxk111430
remotecomputer1=net01.utdallas.edu
remotecomputer2=net02.utdallas.edu
remotecomputer3=net03.utdallas.edu
remotecomputer4=net04.utdallas.edu
remotecomputer5=net05.utdallas.edu
remotecomputer6=net06.utdallas.edu
remotecomputer7=net07.utdallas.edu
remotecomputer8=net08.utdallas.edu
remotecomputer9=net09.utdallas.edu
remotecomputer10=net10.utdallas.edu
remotecomputer11=net11.utdallas.edu
remotecomputer12=net12.utdallas.edu
remotecomputer13=net13.utdallas.edu
remotecomputer14=net14.utdallas.edu
remotecomputer15=net15.utdallas.edu
remotecomputer16=net16.utdallas.edu
remotecomputer17=net17.utdallas.edu
remotecomputer18=net18.utdallas.edu
remotecomputer19=net19.utdallas.edu
remotecomputer20=net20.utdallas.edu

ssh -l "$remoteuser" "$remotecomputer1" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer2" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer3" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer4" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer5" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer6" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer7" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer8" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer9" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer10" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer11" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer12" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer13" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer14" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer15" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer16" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer17" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer18" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer19" "cd $HOME/advancedOS/Project1;./phase2" &
ssh -l "$remoteuser" "$remotecomputer20" "cd $HOME/advancedOS/Project1;./phase2" &


#Run this script on CS machine.
#Prerequisite - Passwordless login should be enabled using Public keys and you should have logged on to the net machines atleast once after creating a public key.
#example
#-bash-4.1$ ssh net23.utdallas.edu
#The authenticity of host 'net23.utdallas.edu (10.176.67.86)' can't be established.
#RSA key fingerprint is 66:af:c1:ce:29:b8:5b:7b:8e:25:33:92:bb:96:0e:46.
#Are you sure you want to continue connecting (yes/no)? yes

#Your code should be in directory $HOME/advancedOS/Project1
#Your main program should be named Project1.java or Project1.cpp or Project1.c
