echo "compiling..."
g++ wxAutocompClassExtractor.cpp -O2 -o wxAutocompClassExtractor.bin
g++ wxAutocompFunctionsExtractor.cpp -O2 -o wxAutocompFunctionsExtractor.bin
export OPWD=$(pwd)
cd $1
echo "getting filenames..."
sh $OPWD/extraer_clases.sh > $OPWD/class_list
sh $OPWD/extraer_funciones.sh > $OPWD/func_list
echo generating index...
cp $OPWD/ids $OPWD/wxWidgets
for A in $(cat $OPWD/class_list); do
    echo C $A
    $OPWD/wxAutocompClassExtractor.bin $A >> $OPWD/wxWidgets
done
for A in $(cat $OPWD/func_list); do
    echo F $A
    $OPWD/wxAutocompFunctionsExtractor.bin $A >> $OPWD/wxWidgets
done
cat $OPWD/stc >> $OPWD/wxWidgets
echo "all done"