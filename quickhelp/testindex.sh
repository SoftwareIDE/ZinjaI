for A in $(cat index | cut -d " " -f2); do 
    if ! test -e $A; then 
	echo $A; 
    fi; 
done
