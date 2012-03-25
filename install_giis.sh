#install_giis.sh - Simple script the installs giis binary.
clear
#Installation - 0 for interactive - 1 use config file
if [ $# -eq 1 ]
then
echo -e $"\n\t\t\tInstalling giis binary - giis-4.6.1"
else
echo "Wrong Usage"
echo "Interactive installation : Usage: sh install_giis.sh 0"
echo "Config.file installation : Usage: sh install_giis.sh 1"
exit
fi


cp ./bin/giis /usr/bin

if [ $? -eq 0 ]
then
echo "copied to bin"
else
echo "Unable to move binary"
exit
fi

#wvar=`grep -c giis /etc/inittab`

#if [ $wvar -eq 0 ]
#then
#/bin/cat ./config/giis.config.in >> /etc/inittab ; 
#echo "config Done";
#else
#echo "config Found";
#fi
	
wvar=`grep -c giis /etc/crontab` ;
if [ $wvar -eq 0 ]
then
/bin/cat ./config/giis.daemon >> /etc/crontab ;
echo "cron Done"; 
else
echo "cron Found"; 
fi

if [ $1 -eq 0 ]
then
cp config/giis.conf /etc
giis -i
fi

if [ $1 -eq 1 ]
then
cp config/giis.conf /etc
giis -c
fi

cp config/hai /giis/hai
cp config/quotes /giis/quotes

echo "Done"

