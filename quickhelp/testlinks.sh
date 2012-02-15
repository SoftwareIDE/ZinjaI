for A in $(grep quickfile *.html | cut -d \" -f2 | cut -d \: -f 2); do 
    if ! test -e $A; then 
	echo $A; 
    fi; 
done
